#pragma once

#include "frame.h"
#include "packed_variant.h"

#include <cstdint>

namespace esphome::waterdrop_serial::message {

template<frame::Command CommandValue, typename... Alternatives>
struct message_variant : packed_variant<Alternatives...> {
  static constexpr auto COMMAND = CommandValue;

  using packed_variant<Alternatives...>::packed_variant;
  using packed_variant<Alternatives...>::operator=;
};

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

enum class MessageC5Slot : uint8_t {
  SLOT_01 = 0x01,
  SLOT_02 = 0x02,
  SLOT_03 = 0x03,
  SLOT_04 = 0x04,
  SLOT_05 = 0x05,
};

struct MessageC5Slot01 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_01;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x40;
  uint8_t unknown3 = 0x22;
  uint8_t unknown4 = 0x00;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
  uint8_t unknown7 = 0x0A;  // 00 (rare) or 0A
};

struct MessageC5Slot02 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_02;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x35;  // 00, 34, 35
  uint8_t unknown5 = 0x08;
  uint8_t unknown6 = 0x34;
  uint8_t unknown7 = 0x00;
};

struct MessageC5Slot03 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_03;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x0E;  // 0C, 0E, 0D (rare), 19 (rare)
  uint8_t unknown3 = 0x01;  // 01, 03
  uint8_t unknown4 = 0xB0;  // 90, B0
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0A;
  uint8_t unknown7 = 0x00;
};

struct MessageC5Slot04 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_04;

  uint8_t unknown1 = 0xF1;  // F1, F3, FF
  uint8_t unknown2 = 0xF1;  // F1, F3
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0E;
  uint8_t unknown7 = 0xFF;
};

struct MessageC5Slot05 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_05;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x0E;
  uint8_t unknown4 = 0x0E;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
  uint8_t unknown7 = 0x00;
};

using MessageC5 = message_variant<frame::Command::COMMAND_C5,
                                MessageC5Slot01, MessageC5Slot02, MessageC5Slot03,
                                MessageC5Slot04, MessageC5Slot05>;
static_assert(sizeof(MessageC5) == 8);

enum class Message22Slot : uint8_t {
  SLOT_01 = 0x01,
  SLOT_02 = 0x02,
  SLOT_03 = 0x03,
  SLOT_0D = 0x0D,
  SLOT_0E = 0x0E,
  SLOT_0F = 0x0F,
};

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
  Message22Slot slot;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x09;

  FaucetState faucetState;

  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
};
static_assert(sizeof(Message22Request) == 8);

struct Message22Slot0E {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0E;

  uint8_t unknown1 = 0xFF;
  uint8_t unknown2 = 0xFF;
  uint8_t unknown3 = 0xFF;
  uint8_t unknown4 = 0xFF;
  uint8_t unknown5 = 0xFF;
  uint8_t unknown6 = 0xFF;
  uint8_t unknown7 = 0xFF;
  uint8_t unknown8 = 0x11;
};

// CF, RO, CB (H, L): filter total life (when it turns red).
struct Message22Slot0F {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0F;

  uint8_t filter_total_life_cf_high = 0xFF;
  uint8_t filter_total_life_cf_low = 0xFF;
  uint8_t filter_total_life_ro_high = 0xFF;
  uint8_t filter_total_life_ro_low = 0xFF;
  uint8_t filter_total_life_cb_high = 0xFE;
  uint8_t filter_total_life_cb_low = 0x99;
  uint8_t unknown1 = 0xFF;
  uint8_t unknown2 = 0x33;
};

struct Message22Slot03 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_03;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  uint8_t tds_high = 0x00;
  uint8_t tds_low = 0x01;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0C;  // 0C or 0E
};

// CF, RO, CB (H, L): filter used life. Turns orange at 360 units before red
struct Message22Slot02 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_02;

  uint8_t filter_used_life_cf_high = 0xFE;
  uint8_t filter_used_life_cf_low = 0x96;
  uint8_t filter_used_life_ro_high = 0xFE;
  uint8_t filter_used_life_ro_low = 0x97;
  uint8_t filter_used_life_cb_high = 0xFE;
  uint8_t filter_used_life_cb_low = 0x98;
  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
};

// unknown2/unknown4: 0A or 00 (rare) - always the same and same in 01 and 0D slots
struct Message22Slot0D {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0D;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x0A;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x0A;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0A;
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

// Seems to be the same as 0D slot
struct Message22Slot01 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_01;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x0A;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x0A;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0A;
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

using Message22Response = message_variant<frame::Command::COMMAND_22,
                                        Message22Slot01, Message22Slot02,
                                        Message22Slot03, Message22Slot0D,
                                        Message22Slot0E, Message22Slot0F>;
static_assert(sizeof(Message22Response) == 9);

}  // namespace esphome::waterdrop_serial::message
