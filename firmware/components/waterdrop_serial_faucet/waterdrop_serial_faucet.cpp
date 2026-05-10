#include <cxxflags.inc>
#include "waterdrop_serial_faucet.h"

#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/core/log.h"

namespace esphome::waterdrop_serial::faucet {

using namespace std::chrono_literals;

static const char *const TAG = "waterdrop_serial_faucet";
static constexpr std::chrono::milliseconds MIN_FRAME_SEPARATION = 11ms;

WaterdropSerialFaucet::WaterdropSerialFaucet() : WaterdropSerial(frame::Source::FAUCET) {}

void WaterdropSerialFaucet::loop() {
  WaterdropSerial::loop();

  ensure_frame_separation_();

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

void WaterdropSerialFaucet::ensure_frame_separation_() {
  if (is_tx_idle()) {
    if (!was_busy_) return;
    // We don't know when TX became idle, so consider we just finished transmition.
    was_busy_ = false;
  } else {
    was_busy_ = true;
  }
  next_command_at_ = std::max(next_command_at_,
      std::chrono::steady_clock::now() + MIN_FRAME_SEPARATION);
}

void WaterdropSerialFaucet::send_c2_() {
  send_message(message::MessageC2{
    .state = 0xF1,
  });
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
  ensure_frame_separation_();
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
