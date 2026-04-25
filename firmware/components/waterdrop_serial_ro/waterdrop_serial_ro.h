#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"

namespace esphome::waterdrop_serial::ro {

class WaterdropSerialRo : public WaterdropSerial {
 public:
  WaterdropSerialRo();
  void dump_config() override;

 private:
  void handle_frame_(const frame::Frame &frame) override;
};

}  // namespace esphome::waterdrop_serial::ro
