#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"

#include <cstdint>
#include <vector>

namespace esphome::waterdrop_serial::faucet {

class WaterdropSerialFaucet : public WaterdropSerial {
 public:
  WaterdropSerialFaucet();
  void dump_config() override;
  void send_frame(uint8_t command, const std::vector<uint8_t> &payload);

 private:
  void handle_frame_(const frame::Frame &frame) override;
};

}  // namespace esphome::waterdrop_serial::faucet
