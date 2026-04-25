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
}

}  // namespace esphome::waterdrop_serial::ro
