#pragma once

#include "endian.h"
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

/* Please note that time based values are not perfectly accurate:
 *  - The RO unit counts a full day after about 24:05 hours.
 *  - Timer-based values are stored to EEPROM every 24 hours, so they reset a given day on each
 *   power cycle.
 */

struct MessageC2 {
  static constexpr auto COMMAND = frame::Command::COMMAND_C2;

  /**
   * State of the RO unit, but also having effect on faucet screen state.
   *
   * 0xF1 - "needs water"? 5s present before pump is running OR continuously when no inlet water
   *        (but before E03 is detected)
   * 0xF7 - briefly before pump running for every-5-min-flush
   * 0xFF - idle
   * 0xF3, 0xF5, 0xF6 - flushing after boot?
   * 0xF0 - E03
   */
  uint8_t state;

  /* Inlet valve state?
   *
   * 0x03 - pump running OR there's no inlet water
   * 0x01 - pump idle
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
  endian::big_uint16_t magicCounter1c = 0;
};

struct MessageC5Slot02 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_02;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;
  uint8_t magicSensor1 = 0x35;
  uint8_t unknown5 = 0x08;
  uint8_t unknown6 = 0x34;
  uint8_t unknown7 = 0x00;  // 0x01 when E03
};

struct MessageC5Slot03 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_03;

  uint8_t unknown1 = 0x00;
  uint8_t air_temperature = 20;

  // When was the last service (e.g. filter replacement) in runtime hours.
  endian::big_uint16_t hours_since_service = 48;

  endian::big_uint16_t magicCounter1d = 0;
  uint8_t unknown7 = 0x00;
};

struct MessageC5Slot04 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_04;

  // Same as MessageC2::state, but may lose brief spikes.
  uint8_t unknown1 = 0xFF;

  // Corelated with MessageC2::state
  // - F1: when idle
  // - F3: active during closed inlet testing (inlet closed + restarting RO every 20 minutes)
  // - F5, F7: unknown spikes
  // - Not reacting to E03
  uint8_t unknown2 = 0xF1;

  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0E;  // 00, 0E
  uint8_t unknown7 = 0xFF;  // 00, FF
};

struct MessageC5Slot05 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_05;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t air_temperature_1 = 20;
  uint8_t air_temperature_2 = 20;
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
  uint8_t unknown7 = 0x00;  // 0 when idle, 1 roughly on E03 (turned 1->0 27s after E03 cleared).
};

using MessageC5 = message_variant<frame::Command::COMMAND_C5,
                                MessageC5Slot01, MessageC5Slot02, MessageC5Slot03,
                                MessageC5Slot04, MessageC5Slot05>;
static_assert(sizeof(MessageC5) == 8);

enum class Message22Slot : uint8_t {
  SLOT_01 = 0x01,
  SLOT_02 = 0x02,
  SLOT_03 = 0x03,
  SLOT_05 = 0x05,
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
   */
  Message22Slot slot;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x09;

  FaucetState faucetState = FaucetState::CLOSED;

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

struct Message22Slot0F {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0F;

  endian::big_uint16_t filter_total_life_cf = 0xFFFF;
  endian::big_uint16_t filter_total_life_ro = 0xFFFF;
  endian::big_uint16_t filter_total_life_cb = 0xFFFF;
  uint8_t unknown1 = 0xFF;  // 44, FF
  uint8_t unknown2 = 0x33;
};

struct Message22Slot03 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_03;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x00;
  endian::big_uint16_t tds = 0;
  uint8_t unknown5 = 0x00;
  uint8_t air_temperature = 20;
};

struct Message22Slot05 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_05;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;

  // values in 0-16 range, ~normal-distributed around 8, dropping when running water.
  uint8_t unknown4 = 0x00;

  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x00;
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

// CF, RO, CB: filter used life. Turns orange at 360 hours (15 days) before turning red
struct Message22Slot02 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_02;

  endian::big_uint16_t filter_used_life_cf = 0xFE96;
  endian::big_uint16_t filter_used_life_ro = 0xFE97;
  endian::big_uint16_t filter_used_life_cb = 0xFFCC;
  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
};

struct Message22Slot0D {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0D;

  endian::big_uint16_t magicCounter1aCF = 0;
  endian::big_uint16_t magicCounter1aRO = 0;
  endian::big_uint16_t magicCounter1aCB = 0;
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};


struct Message22Slot01 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_01;

  endian::big_uint16_t magicCounter1bCF = 0;
  endian::big_uint16_t magicCounter1bRO = 0;
  endian::big_uint16_t magicCounter1bCB = 0;
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

using Message22Response = message_variant<frame::Command::COMMAND_22,
                                        Message22Slot01, Message22Slot02,
                                        Message22Slot03, Message22Slot05,
                                        Message22Slot0D, Message22Slot0E,
                                        Message22Slot0F>;
static_assert(sizeof(Message22Response) == 9);

}  // namespace esphome::waterdrop_serial::message
