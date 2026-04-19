#include "frame.h"

#include <algorithm>
#include <numeric>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::waterdrop_serial_ro::frame {

static const char *const TAG = "waterdrop_serial_ro.frame";

enum class PayloadChecksum : uint8_t {
  NONE,
  SHORT,
  LONG,
};

static PayloadChecksum payload_checksum_for(Command command) {
  switch (command) {
    case Command::COMMAND_C2:
      return PayloadChecksum::LONG;
    case Command::COMMAND_22:
      return PayloadChecksum::SHORT;
  }

  return PayloadChecksum::NONE;
}

static std::string toString(Command command) {
  return str_sprintf("%02X", static_cast<uint8_t>(command));
}

std::string Frame::toString() const {
  return str_sprintf("%s: %s", frame::toString(command).c_str(),
      format_hex_pretty(payload.data(), payload_length, ' ').c_str());
}

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
      if (FRAME_MIN_LENGTH <= byte && byte <= FRAME_MAX_LENGTH) {
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
  return false;
}

uint8_t Parser::raw_length_() const {
  return length_expected_ + FRAME_PREAMBLE_LENGTH;
}

void Parser::log_invalid_frame_(const char *error_message) const {
  ESP_LOGW(TAG, "Invalid frame (%s): AA 55 %02X %s", error_message, raw_length_(),
           format_hex_pretty(body_.data(), length_current_, ' ', false).c_str());
}

bool Parser::finish_(Frame &frame) {
  size_t header_overhead = 2; // command + frame checksum
  assert(length_current_ >= header_overhead);
  const uint8_t command_byte = body_[0];
  frame.command = static_cast<Command>(command_byte);

  const auto pchk = payload_checksum_for(frame.command);
  if (pchk != PayloadChecksum::NONE) header_overhead++;

  frame.payload_length = length_current_ - header_overhead;
  if (length_current_ < header_overhead
      || frame.payload_length > frame.payload.size()
      || (pchk == PayloadChecksum::SHORT && length_current_ == header_overhead)) {
    counters_.invalid_lengths++;
    log_invalid_frame_("invalid length");
    return false;
  }

  std::copy_n(body_.begin() + 1, frame.payload_length, frame.payload.begin());

  uint8_t payload_checksum =
      std::accumulate(frame.payload.begin(), frame.payload.begin() + frame.payload_length, 0);
  const uint8_t frame_checksum = 0xAA + 0x55 + raw_length_() + command_byte + payload_checksum;
  if (frame_checksum != body_[length_current_ - 1]) {
    counters_.invalid_checksum_frame++;
    log_invalid_frame_("invalid frame checksum");
    return false;
  }

  if (pchk != PayloadChecksum::NONE) {
    if (pchk == PayloadChecksum::SHORT) {
      payload_checksum -= frame.payload[0];
    }

    if (payload_checksum != body_[length_current_ - 2]) {
      counters_.invalid_checksum_payload++;
      log_invalid_frame_("invalid payload checksum");
      return false;
    }
  }

  counters_.frames_ok++;
  return true;
}

}  // namespace esphome::waterdrop_serial_ro::frame
