#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace esphome::uart {
class UARTDevice;
}

namespace esphome::waterdrop_serial::frame {

namespace _details {

static constexpr size_t PAYLOAD_MAX_LENGTH = 9;
static constexpr size_t FRAME_PREAMBLE_LENGTH = 3; // AA 55 LENGTH
static constexpr size_t FRAME_OVERHEAD_A = 5; // AA 55 LENGTH command <payload> frame_checksum
static constexpr size_t FRAME_OVERHEAD_B = 6; // Frame overhead with optional payload checksum
static constexpr size_t FRAME_MAX_LENGTH = PAYLOAD_MAX_LENGTH + FRAME_OVERHEAD_B;

}  // namespace _details

enum class Source : uint8_t {
  RO = 1,
  FAUCET,
};

enum class Command : uint8_t {
  COMMAND_22 = 0x22,
  COMMAND_C2 = 0xC2,
  COMMAND_C5 = 0xC5,
};

struct Frame {
  Source source;
  Command command;
  uint8_t payload_length;
  std::array<uint8_t, _details::PAYLOAD_MAX_LENGTH> payload;

  std::string to_string() const;
  void write(uart::UARTDevice &uart) const;
  uint8_t get_checksum_base() const;
  std::optional<uint8_t> get_payload_checksum(uint8_t checksum_base) const;
  uint8_t get_frame_checksum(uint8_t checksum_base) const;
};

struct Counters {
  size_t rx_bytes;
  size_t frames_ok;
  size_t rx_junk_bytes;
  size_t invalid_lengths;
  size_t invalid_checksum_payload;
  size_t invalid_checksum_frame;
  size_t invalid_command;
  uint8_t last_invalid_command;
};

class Parser {
 public:
  Parser(Source source);

  const Counters &counters() const;
  bool feed(uint8_t byte, Frame &frame);

 private:
  enum class State : uint8_t {
    WAIT_AA = 0,
    WAIT_55,
    WAIT_LENGTH,
    READ_BODY,
  };

  bool finish_(Frame &frame);
  void log_invalid_frame_(const char *error_message) const;

  Source source_;
  Counters counters_{};
  State state_ = State::WAIT_AA;
  uint8_t length_expected_;
  uint8_t length_current_;
  std::array<uint8_t, _details::FRAME_MAX_LENGTH - _details::FRAME_PREAMBLE_LENGTH> body_;
};

Source opposite(Source source);

}  // namespace esphome::waterdrop_serial::frame
