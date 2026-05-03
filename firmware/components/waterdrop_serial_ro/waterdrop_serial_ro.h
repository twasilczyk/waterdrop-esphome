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
  void send_request_frame_();

  uint8_t request_slot_ = 0;

  // TODO: remove
  void send_frame(frame::Command command, const std::vector<uint8_t> &payload);
};

}  // namespace esphome::waterdrop_serial::ro
