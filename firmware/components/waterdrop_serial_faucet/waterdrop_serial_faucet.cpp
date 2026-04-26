#include <cxxflags.inc>
#include "waterdrop_serial_faucet.h"

#include "esphome/core/log.h"

namespace esphome::waterdrop_serial::faucet {

using namespace std::chrono_literals;

static const char *const TAG = "waterdrop_serial_faucet";

WaterdropSerialFaucet::WaterdropSerialFaucet() : WaterdropSerial(frame::Source::FAUCET) {}

void WaterdropSerialFaucet::loop() {
  WaterdropSerial::loop();

  const auto now = std::chrono::steady_clock::now();
  if (now >= next_command_at_) {
    switch (next_command_) {
      case frame::Command::COMMAND_C2:
        next_command_at_ = now + 80ms;
        next_command_ = frame::Command::COMMAND_C5;
        send_c2_();
        break;
      case frame::Command::COMMAND_C5:
        next_command_at_ = now + 90ms;
        next_command_ = frame::Command::COMMAND_C2;
        send_c5_();
        break;
      default:
        assert(false);
    }
  }
}

void WaterdropSerialFaucet::send_c2_() {
  // STATE:
  //  Boot: FF
  //  Screen on: F1, F7
  //  Flush: F3, F5, F6
  //  Screen off: all others

  // FLG2: no effect
  //  0x03 - regular
  //  0x01 - present during flush and boot

  // ERR:
  //  Display priority: E02 > E03 > E04 > E01
  //  0b00000100: E02
  //  0b00000001: E03
  //  0b00000010: E04
  //  0b10000000: E01

  //                                                         STATE   FLG   ERR
  send_frame(frame::Command::COMMAND_C2, std::vector<uint8_t>{0xF1, 0x03, 0x00});
}

void WaterdropSerialFaucet::send_c5_() {
  send_frame(frame::Command::COMMAND_C5, cc5_data_.at(cc5_counter_++));
  cc5_counter_ %= cc5_data_.size();
}

void WaterdropSerialFaucet::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial Faucet:");
  WaterdropSerial::dump_base_config(TAG);
}

void WaterdropSerialFaucet::handle_frame_(const frame::Frame &frame) {
  ESP_LOGD(TAG, "RX %s", frame.to_string().c_str());
  if (frame.command != frame::Command::COMMAND_22) {
    ESP_LOGW(TAG, "Unexpected request: %02X", static_cast<uint8_t>(frame.command));
    return;
  }
  if (frame.payload_length != 8) {
    ESP_LOGW(TAG, "Invalid request length: %d", frame.payload_length);
    return;
  }

  uint8_t page = frame.payload[0];
  if (c22_data_.count(page) == 0) {
    ESP_LOGW(TAG, "Unknown page requested: %02X", page);
    return;
  }
  send_frame(frame::Command::COMMAND_22, c22_data_.at(page));
  if (page == 3) {
    if (c22_data_[page][6] == 255) c22_data_[page][5]++;
    c22_data_[page][6]++;
  }
}

void WaterdropSerialFaucet::send_frame(frame::Command command, const std::vector<uint8_t> &payload) {
  frame::Frame frame;
  assert(payload.size() <= frame.payload.size());
  frame.source = frame::Source::RO;
  frame.command = command;
  frame.payload_length = static_cast<uint8_t>(payload.size());
  std::copy(payload.begin(), payload.end(), frame.payload.begin());

  frame.write(*this);
}

}  // namespace esphome::waterdrop_serial::faucet
