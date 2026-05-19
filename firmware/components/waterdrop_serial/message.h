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

/* Water events are spread between 3 fields:
 *  - MessageC2::state
 *  - MessageC5Slot04::unknown1
 *  - MessageC5Slot04::unknown2
 *
 * Idle
 *  - C2=FF, C5u1=FF, C5u2=F1
 *
 * Filling glass with water (without pressure tank):
 *  - C5u2 doesn't change at all
 *  - First 3 seconds: C2 oscillates between F1 and FF with 650ms period
 *  - Then C5u2 joins, oscillating between F1 and FF with 2.5s period
 *
 * Filling pitcher or glass with water (with pressure tank):
 *  - C5u2 doesn't change at all
 *  - Both C2 and C5u1 oscillate together (at the same time) between F1 and FF with 8s cycle
 *
 * Activity every 5 minutes (with or without pressure tank):
 *  - C2 goes FF -> F7 -> F1 -> FF very quickly (unmeasurable in HA)
 *  - Variant A: C5u1/2 doesn't change
 *  - Variant B: C5u1/2 change to F7 for 2x the time C2 was not in idle (starting together with C2)
 *  - Variant C: C5u1 changes to F1 for 2x the time C2 was not in idle, C5u2 doesn't change
 *  - Variants change periodically (cycle is 5 minutes):
 *    - Variant A runs for 2 cycles after water use or 9 cycles (but sometimes 6, first time after
 *      water use) after Variant C
 *    - Variant B runs for 3 cycles after Variant A
 *    - Variant C runs for 5 cycles after Variant B (but sometimes 8, first time after water use)
 */

/* A couple values are oscillating together 14 +/- 1:
 * - Message22Slot03::unknown6
 * - MessageC5Slot03::unknown2
 * - MessageC5Slot05::unknown3
 * - MessageC5Slot05::unknown4
 */

struct MessageC2 {
  static constexpr auto COMMAND = frame::Command::COMMAND_C2;

  /**
   * State of the RO unit, but also having effect on faucet screen state.
   *
   * 0xF1 - pump running (or no input water when trying to run, but before E03 is detected)
   * 0xF7 - briefly before pump running for every-5-min-flush
   * 0xFF - pump idle
   * 0xF3, 0xF5, 0xF6 - flushing after boot?
   * 0xF0 - E03?
   */
  uint8_t state;

  /* No effect known. Pump state?
   *
   * 0x03 - pump running (or there's no input water)
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
  uint8_t unknown6 = 0x00;
  uint8_t unknown7 = 0x0A;  // 00 (rare), 0A, 28
};

struct MessageC5Slot02 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_02;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x35;  // 4-36 analog range, but also observed 53 in winter
  uint8_t unknown5 = 0x08;
  uint8_t unknown6 = 0x34;
  uint8_t unknown7 = 0x00;  // 00, 01
};

struct MessageC5Slot03 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_03;

  uint8_t unknown1 = 0x00;

  // 12-15 maybe analog value, but seen 25 once.
  // Very stable, but heavily oscillating when between values.
  uint8_t unknown2 = 0x0E;

  // Total time RO unit was powered on.
  endian::big_uint16_t operating_lifetime_hours = 48;
  // RO firmware counts a full day after 24:05 hours.
  static constexpr float OPERATING_LIFETIME_ERROR = 1440.0f / 1445.0f;

  uint8_t unknown5 = 0x00;

  // Started rising from 40 to 90 (by 10 every 6:21~6:23) after turning off input valve.
  // Also seen 10 when new.
  uint8_t unknown6 = 0x0A;

  uint8_t unknown7 = 0x00;
};

struct MessageC5Slot04 {
  static constexpr MessageC5Slot TAG = MessageC5Slot::SLOT_04;

  // Corelated with MessageC2::state
  // - FF when idle
  // - F0 when E03 (and F1 right before that)
  uint8_t unknown1 = 0xF1;  // F1, F3, FF

  // Corelated with MessageC2::state
  // - F1 when idle
  // - Not reacting to E03
  uint8_t unknown2 = 0xF1;  // F1, F3

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
  uint8_t unknown3 = 0x0E;  // Correlated with MessageC5Slot03::unknown2
  uint8_t unknown4 = 0x0E;  // Correlated with MessageC5Slot03::unknown2
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
   * TODO: Investigate other slots (highest bit is ignored):
   * SS 01 00 00 CA xx xx xx xx (SS=0, 4, 6, 8-12, 17-127)
   * 05 00 00 00 xx 00 00 00 00
   * 07 00 00 00 00 00 00 00 00
   * 10 00 00 00 00 00 00 66 22
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

  // Dynamic bytes observed while flushing.
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
  uint8_t unknown6 = 0x0C;  // Same as MessageC5Slot03::unknown2
};

// CF, RO, CB (H, L): filter used life. Turns orange at 360 units before red
struct Message22Slot02 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_02;

  endian::big_uint16_t filter_used_life_cf = 0xFE96;
  endian::big_uint16_t filter_used_life_ro = 0xFE97;
  endian::big_uint16_t filter_used_life_cb = 0xFFCC;
  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x00;
};

// unknown2/unknown4/unknown6: always the same and same in 01 and 0D slots.
struct Message22Slot0D {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_0D;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

// Seems to be the same as 0D slot
struct Message22Slot01 {
  static constexpr Message22Slot TAG = Message22Slot::SLOT_01;

  uint8_t unknown1 = 0x00;
  uint8_t unknown2 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown3 = 0x00;
  uint8_t unknown4 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown5 = 0x00;
  uint8_t unknown6 = 0x0A;  // 00 (rare), 0A, 28
  uint8_t unknown7 = 0x00;
  uint8_t unknown8 = 0x00;
};

using Message22Response = message_variant<frame::Command::COMMAND_22,
                                        Message22Slot01, Message22Slot02,
                                        Message22Slot03, Message22Slot0D,
                                        Message22Slot0E, Message22Slot0F>;
static_assert(sizeof(Message22Response) == 9);

}  // namespace esphome::waterdrop_serial::message
