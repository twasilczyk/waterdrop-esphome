#include "waterdrop_serial_ro.h"

#include "esphome/core/log.h"
#ifdef USE_ESP32
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif

namespace esphome::waterdrop_serial_ro {

static const char *const TAG = "waterdrop_serial_ro";
static constexpr int READ_BUDGET_PER_LOOP = 64;

void WaterdropSerialRo::loop() {
  frame::Frame parsed_frame;
  uint8_t byte;
  int budget = READ_BUDGET_PER_LOOP;

  while (budget-- > 0 && available()) {
    if (!read_byte(&byte)) {
      break;
    }

    if (parser_.feed(byte, parsed_frame)) {
      // TODO: handle_frame in message.h Parser, then call handle_message_ here
      handle_frame_(parsed_frame);
    }
  }
}

void WaterdropSerialRo::dump_config() {
  ESP_LOGCONFIG(TAG, "Waterdrop Serial RO:");

#ifdef USE_ESP32
  if (parent_ != nullptr) {
    auto idf_uart = static_cast<uart::IDFUARTComponent *>(parent_);
    ESP_LOGCONFIG(TAG, "  UART Bus: %u", idf_uart->get_hw_serial_number());
  }
#endif

  const auto &counters = parser_.counters();
  ESP_LOGCONFIG(TAG,
                "  Counters:\n"
                "    RX bytes: %zu\n"
                "    Correct frames: %zu\n"
                "    Junk bytes: %zu\n"
                "    Invalid lengths: %zu\n"
                "    Invalid payload checksums: %zu\n"
                "    Invalid frame checksums: %zu",
                counters.rx_bytes, counters.frames_ok, counters.rx_junk_bytes, counters.invalid_lengths,
                counters.invalid_checksum_payload, counters.invalid_checksum_frame);
}

void WaterdropSerialRo::handle_frame_(const frame::Frame &frame) {
  ESP_LOGD(TAG, "RX %s", frame.toString().c_str());
}

}  // namespace esphome::waterdrop_serial_ro
