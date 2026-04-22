#include <cxxflags.inc>
#include "waterdrop_serial_faucet.h"

#include "esphome/core/log.h"

namespace esphome::waterdrop_serial::faucet {

static const char *const TAG = "waterdrop_serial_faucet";

void WaterdropSerialFaucet::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial Faucet:");
  WaterdropSerial::dump_base_config(TAG);
}

void WaterdropSerialFaucet::send_frame(uint8_t command, const std::vector<uint8_t> &payload) {
  frame::Frame frame;
  assert(payload.size() <= frame.payload.size());
  frame.command = static_cast<frame::Command>(command);
  frame.payload_length = static_cast<uint8_t>(payload.size());
  std::copy(payload.begin(), payload.end(), frame.payload.begin());

  frame.write(*this);
}

}  // namespace esphome::waterdrop_serial::faucet
