#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace esphome::waterdrop_serial_ro::frame {

enum class Command : uint8_t {
  COMMAND_22 = 0x22,
  COMMAND_C2 = 0xC2,
};

struct Frame {
  static constexpr size_t MAX_PAYLOAD_LENGTH = 34;

  Command command;
  uint8_t payload_length;
  std::array<uint8_t, MAX_PAYLOAD_LENGTH> payload;

  std::string toString() const;
};

struct Counters {
  size_t rx_bytes;
  size_t frames_ok;
  size_t rx_junk_bytes;
  size_t invalid_lengths;
  size_t invalid_checksum_payload;
  size_t invalid_checksum_frame;
};

class Parser {
 public:
  const Counters &counters() const;
  bool feed(uint8_t byte, Frame &frame);

 private:
  enum class State : uint8_t {
    WAIT_AA = 0,
    WAIT_55,
    WAIT_LENGTH,
    READ_BODY,
  };

  static constexpr size_t FRAME_PREAMBLE_LENGTH = 3; // AA 55 LENGTH

  // + command + frame checksum
  static constexpr size_t FRAME_MIN_LENGTH = FRAME_PREAMBLE_LENGTH + 2;

  // + command + frame checksum + optional payload checksum
  static constexpr size_t FRAME_MAX_LENGTH = Frame::MAX_PAYLOAD_LENGTH + FRAME_MIN_LENGTH + 1;

  uint8_t raw_length_() const;
  bool finish_(Frame &frame);
  void log_invalid_frame_(const char *error_message) const;

  Counters counters_{};
  State state_ = State::WAIT_AA;
  uint8_t length_expected_;
  uint8_t length_current_;
  std::array<uint8_t, FRAME_MAX_LENGTH - FRAME_PREAMBLE_LENGTH> body_;
};

}  // namespace esphome::waterdrop_serial_ro::frame
