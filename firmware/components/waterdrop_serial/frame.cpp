#include <cxxflags.inc>
#include "frame.h"

#include "esphome/components/uart/uart.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <numeric>

namespace esphome::waterdrop_serial::frame {

using namespace _details;

static const char *const TAG = "waterdrop_serial.frame";

union Header {
  struct {
    uint8_t preamble[2];
    uint8_t length;
    Command command;
  } __attribute__((packed)) s;

  uint8_t raw[sizeof(s)];

  constexpr Header() : s{.preamble = {0xAA, 0x55}} {}

  static_assert(sizeof(s) == 4);
};

enum class PayloadChecksum : uint8_t {
  NONE,
  SHORT,
  LONG,
};

Source opposite(Source source) {
  switch (source) {
    case Source::RO:
      return Source::FAUCET;
    case Source::FAUCET:
      return Source::RO;
  }
  assert(false);
}

static bool is_valid(Source source, Command command, size_t &expected_payload_length) {
  switch (source) {
    case Source::RO:
      switch (command) {
        case Command::COMMAND_C2:
          expected_payload_length = 3;
          return true;
        case Command::COMMAND_C5:
          expected_payload_length = 8;
          return true;
        case Command::COMMAND_22:
          expected_payload_length = 9;
          return true;
      }
      return false;
    case Source::FAUCET:
      switch (command) {
        case Command::COMMAND_22:
          expected_payload_length = 8;
          return true;
        case Command::COMMAND_C2:
        case Command::COMMAND_C5:
          return false;
      }
      return false;
  }
  assert(false);
}

static PayloadChecksum payload_checksum_for(Source source, Command command) {
  switch (command) {
    case Command::COMMAND_C2:
      return PayloadChecksum::LONG;
    case Command::COMMAND_22:
      switch (source) {
        case Source::RO:
          return PayloadChecksum::SHORT;
        case Source::FAUCET:
          return PayloadChecksum::LONG;
      }
      assert(false);
    case Command::COMMAND_C5:
      return PayloadChecksum::NONE;
  }
  assert(false);
}

static uint8_t frame_overhead_for(Source source, Command command) {
  return payload_checksum_for(source, command) == PayloadChecksum::NONE ?
      FRAME_OVERHEAD_A : FRAME_OVERHEAD_B;
}

static std::string to_string(Command command) {
  return str_sprintf("%02X", static_cast<uint8_t>(command));
}

std::string Frame::to_string() const {
  return str_sprintf("%s: %s", frame::to_string(command).c_str(),
      format_hex_pretty(payload.data(), payload_length, ' ').c_str());
}

uint8_t Frame::get_checksum_base() const {
  return std::accumulate(payload.begin(), payload.begin() + payload_length, uint8_t{0});
}

std::optional<uint8_t> Frame::get_payload_checksum(uint8_t checksum_base) const {
  switch (payload_checksum_for(source, command)) {
    case PayloadChecksum::NONE:
      return std::nullopt;
    case PayloadChecksum::SHORT:
      return checksum_base - payload[0];
    case PayloadChecksum::LONG:
      return checksum_base;
  }
  assert(false);
}

uint8_t Frame::get_frame_checksum(uint8_t checksum_base) const {
  return 0xAA + 0x55 + frame_overhead_for(source, command) + payload_length
      + static_cast<uint8_t>(command) + checksum_base;
}

void Frame::write(uart::UARTDevice &uart) const {
  size_t expected_payload_length;
  assert(is_valid(source, command, expected_payload_length));
  assert(payload_length == expected_payload_length);
  assert(expected_payload_length <= payload.size());

  static Header header;
  header.s.length = payload_length + frame_overhead_for(source, command);
  header.s.command = command;

  uart.write_array(header.raw, sizeof(header));
  uart.write_array(payload.data(), payload_length);

  const auto checksum_base = get_checksum_base();
  const auto payload_checksum = get_payload_checksum(checksum_base);
  if (payload_checksum.has_value()) {
    uart.write_byte(*payload_checksum);
  };
  uart.write_byte(get_frame_checksum(checksum_base));
}

Parser::Parser(Source source) : source_(source) {}

const Counters &Parser::counters() const {
  return counters_;
}

bool Parser::feed(uint8_t byte, Frame &frame) {
  counters_.rx_bytes++;

  switch (state_) {
    case State::WAIT_AA:
      if (byte == 0xAA) {
        state_ = State::WAIT_55;
      } else {
        counters_.rx_junk_bytes++;
      }
      return false;

    case State::WAIT_55:
      if (byte == 0x55) {
        state_ = State::WAIT_LENGTH;
      } else {
        state_ = byte == 0xAA ? State::WAIT_55 : State::WAIT_AA;
        counters_.rx_junk_bytes++;
      }
      return false;

    case State::WAIT_LENGTH:
      if (FRAME_OVERHEAD_A <= byte && byte <= FRAME_MAX_LENGTH) {
        length_expected_ = byte - FRAME_PREAMBLE_LENGTH;
        length_current_ = 0;
        state_ = State::READ_BODY;
      } else {
        counters_.invalid_lengths++;
        ESP_LOGW(TAG, "Invalid frame length: %u", byte);
        state_ = byte == 0xAA ? State::WAIT_55 : State::WAIT_AA;
      }
      return false;

    case State::READ_BODY:
      body_[length_current_++] = byte;
      if (length_current_ >= length_expected_) {
        state_ = State::WAIT_AA;
        return finish_(frame);
      }
      return false;
  }
  assert(false);
}

void Parser::log_invalid_frame_(const char *error_message) const {
  const uint8_t raw_length = length_expected_ + FRAME_PREAMBLE_LENGTH;
  ESP_LOGW(TAG, "Invalid frame (%s): AA 55 %02X %s", error_message, raw_length,
           format_hex_pretty(body_.data(), length_current_, ' ', false).c_str());
}

bool Parser::finish_(Frame &frame) {
  frame.source = source_;
  assert(length_current_ >= FRAME_OVERHEAD_A - FRAME_PREAMBLE_LENGTH);
  frame.command = static_cast<Command>(body_[0]);
  size_t expected_payload_length;
  if (!is_valid(source_, frame.command, expected_payload_length)) {
    counters_.invalid_command++;
    counters_.last_invalid_command = static_cast<uint8_t>(frame.command);
    log_invalid_frame_("invalid command");
    return false;
  }

  const uint8_t net_overhead = frame_overhead_for(source_, frame.command) - FRAME_PREAMBLE_LENGTH;
  frame.payload_length = length_current_ - net_overhead;
  if (net_overhead > length_current_ || frame.payload_length > frame.payload.size()
      || frame.payload_length != expected_payload_length) {
    counters_.invalid_lengths++;
    log_invalid_frame_("invalid length");
    return false;
  }

  std::copy_n(body_.begin() + 1, frame.payload_length, frame.payload.begin());

  const auto checksum_base = frame.get_checksum_base();
  if (frame.get_frame_checksum(checksum_base) != body_[length_current_ - 1]) {
    counters_.invalid_checksum_frame++;
    log_invalid_frame_("invalid frame checksum");
    return false;
  }
  const auto payload_checksum = frame.get_payload_checksum(checksum_base);
  if (payload_checksum.has_value() && *payload_checksum != body_[length_current_ - 2]) {
    counters_.invalid_checksum_payload++;
    log_invalid_frame_("invalid payload checksum");
    return false;
  }

  counters_.frames_ok++;
  return true;
}

}  // namespace esphome::waterdrop_serial::frame
