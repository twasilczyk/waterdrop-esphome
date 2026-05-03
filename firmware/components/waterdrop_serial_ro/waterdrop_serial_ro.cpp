#include <cxxflags.inc>
#include "waterdrop_serial_ro.h"

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
    send_request_frame_();
  }
}

void WaterdropSerialRo::send_request_frame_() {
  static constexpr std::array<uint8_t, 6> request_slots{
      0x0D, 0x01, 0x0E, 0x0F, 0x03, 0x02
  };
  uint8_t slot = request_slots[request_slot_++];
  request_slot_ %= request_slots.size();

  send_frame(frame::Command::COMMAND_22, std::vector<uint8_t>{
    // A - faucet state: 0x02 (open), 0x03 (closed)
    //                  A
    slot, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00
  });
}

void WaterdropSerialRo::send_frame(frame::Command command, const std::vector<uint8_t> &payload) {
  frame::Frame frame;
  assert(payload.size() <= frame.payload.size());
  frame.source = frame::Source::FAUCET;
  frame.command = command;
  frame.payload_length = static_cast<uint8_t>(payload.size());
  std::copy(payload.begin(), payload.end(), frame.payload.begin());

  frame.write(*this);
}

}  // namespace esphome::waterdrop_serial::ro
