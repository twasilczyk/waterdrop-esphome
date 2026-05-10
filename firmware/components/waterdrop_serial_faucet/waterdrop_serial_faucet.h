#pragma once

#include "esphome/components/waterdrop_serial/frame.h"
#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/components/waterdrop_serial/waterdrop_serial.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <vector>

namespace esphome::waterdrop_serial::faucet {

class WaterdropSerialFaucet : public WaterdropSerial {
 public:
  WaterdropSerialFaucet();
  void dump_config() override;
  void loop() override;

 private:
  void handle_frame_(const frame::Frame &frame) override;
  void ensure_frame_separation_();

  void send_c2_();
  void send_c5_();

  frame::Command next_command_ = frame::Command::COMMAND_C2;
  std::chrono::steady_clock::time_point next_command_at_ = {};
  bool was_busy_ = false;

  uint8_t cc5_counter_ = 0;
  std::vector<message::MessageC5> cc5_data_{
    message::MessageC5Slot01{},
    message::MessageC5Slot02{},
    message::MessageC5Slot03{},
    message::MessageC5Slot04{},
    message::MessageC5Slot05{},
  };

  std::map<message::Message22Slot, message::Message22Response> c22_data_{
    {message::Message22Slot::SLOT_0E, message::Message22Slot0E{}},
    {message::Message22Slot::SLOT_0F, message::Message22Slot0F{}},
    {message::Message22Slot::SLOT_03, message::Message22Slot03{}},
    {message::Message22Slot::SLOT_02, message::Message22Slot02{}},
    {message::Message22Slot::SLOT_0D, message::Message22Slot0D{}},
    {message::Message22Slot::SLOT_01, message::Message22Slot01{}},
  };
};

}  // namespace esphome::waterdrop_serial::faucet
