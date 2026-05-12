#include <cxxflags.inc>
#include "waterdrop_serial_faucet.h"

#include "esphome/components/waterdrop_serial/message.h"
#include "esphome/core/log.h"

namespace esphome::waterdrop_serial::faucet {

using namespace std::chrono_literals;

static const char *const TAG = "waterdrop_serial_faucet";
static constexpr std::chrono::milliseconds MIN_FRAME_SEPARATION = 11ms;

WaterdropSerialFaucet::WaterdropSerialFaucet() : WaterdropSerial(TAG, frame::Source::FAUCET) {}

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
  send_message(cc5_data_.at(cc5_counter_++));
  cc5_counter_ %= cc5_data_.size();
}

void WaterdropSerialFaucet::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial Faucet:");
  WaterdropSerial::dump_config();
}

void WaterdropSerialFaucet::handle_frame(const frame::Frame &frame) {
  ESP_LOGD(TAG, "RX %s", frame.to_string().c_str());
  if (frame.command != frame::Command::COMMAND_22) {
    ESP_LOGW(TAG, "Unexpected request: %02X", static_cast<uint8_t>(frame.command));
    return;
  }
  if (frame.payload_length != 8) {
    ESP_LOGW(TAG, "Invalid request length: %d", frame.payload_length);
    return;
  }

  // TODO: frame.as<Message22Request>(), asserting COMMAND_22.
  auto slot = static_cast<message::Message22Slot>(frame.payload[0]);
  auto response = c22_data_.find(slot);
  if (response == c22_data_.end()) {
    ESP_LOGW(TAG, "Unknown page requested: %02X", frame.payload[0]);
    return;
  }
  send_message(response->second);
  ensure_frame_separation_();
  if (slot == message::Message22Slot::SLOT_03) {
    auto &tds = response->second.get<message::Message22Slot03>();
    if (tds.tds_low == 255) tds.tds_high++;
    tds.tds_low++;
  }
}

}  // namespace esphome::waterdrop_serial::faucet
