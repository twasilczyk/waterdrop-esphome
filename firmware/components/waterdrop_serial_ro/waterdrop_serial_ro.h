#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"

namespace esphome::waterdrop_serial::ro {

class WaterdropSerialRo : public WaterdropSerial {
 public:
  void loop() override;
  void dump_config() override;

 private:
  void handle_frame_(const frame::Frame &frame);

  frame::Parser parser_{};
};

}  // namespace esphome::waterdrop_serial::ro
