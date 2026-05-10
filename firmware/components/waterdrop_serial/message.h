#pragma once

#include "frame.h"

#include <cstdint>

namespace esphome::waterdrop_serial::message {

struct MessageC2 {
  static constexpr auto COMMAND = frame::Command::COMMAND_C2;

  /**
   * State of the RO unit, but also having effect on faucet screen state.
   *
   * Boot: FF
   * Screen on: F1, F7
   * Flush: F3, F5, F6
   * Screen off: all others
   */
  uint8_t state;

  /* No effect known.
   *
   * 0x03 - regular
   * 0x01 - present during flush and boot
   */
  uint8_t unknown = 0x03;

  /**
   * Error state.
   *
   * 0b00000100: E02
   * 0b00000001: E03
   * 0b00000010: E04
   * 0b10000000: E01
   *
   * Display priority: E02 > E03 > E04 > E01
   */
  uint8_t error = 0;
};
static_assert(sizeof(MessageC2) == 3);

struct Message22Request {
  static constexpr auto COMMAND = frame::Command::COMMAND_22;

  enum class FaucetState : uint8_t {
    OPEN = 0x02,
    CLOSED = 0x03,
  };

  /**
   * The slot to request information for.
   *
   * Original faucet requests slots in the following order:
   * 0x0D, 0x01, 0x0E, 0x0F, 0x03, 0x02
   *
   * TODO: Investigate if there's any other slots on RO unit.
   */
  uint8_t slot;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x09;

  FaucetState faucetState;

  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
};
static_assert(sizeof(Message22Request) == 8);

}  // namespace esphome::waterdrop_serial::message
