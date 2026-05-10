#include <cxxflags.inc>
#include "waterdrop_serial_ro.h"

#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/core/log.h"

namespace esphome::waterdrop_serial::ro {

static const char *const TAG = "waterdrop_serial_ro";

WaterdropSerialRo::WaterdropSerialRo() : WaterdropSerial(frame::Source::RO) {}

void WaterdropSerialRo::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial RO:");
  WaterdropSerial::dump_base_config(TAG);
}

void WaterdropSerialRo::handle_frame_(const frame::Frame &frame) {
  ESP_LOGD(TAG, "RX %s", frame.to_string().c_str());

  if (frame.command == frame::Command::COMMAND_C2) {
    send_request_message_();
  }
}

void WaterdropSerialRo::send_request_message_() {
  static constexpr std::array<message::Message22Slot, 6> request_slots{
      message::Message22Slot::SLOT_0D, message::Message22Slot::SLOT_01, message::Message22Slot::SLOT_0E,
      message::Message22Slot::SLOT_0F, message::Message22Slot::SLOT_03, message::Message22Slot::SLOT_02
  };
  auto slot = request_slots[request_slot_++];
  request_slot_ %= request_slots.size();

  send_message(message::Message22Request{
    .slot = slot,
    .faucetState = message::Message22Request::FaucetState::CLOSED,
  });
}

}  // namespace esphome::waterdrop_serial::ro
