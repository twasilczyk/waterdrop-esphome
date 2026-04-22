#include <cxxflags.inc>
#include "waterdrop_serial.h"

#include "esphome/core/log.h"
#ifdef USE_ESP32
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif

namespace esphome::waterdrop_serial {

void WaterdropSerial::dump_base_config(const char *tag) const {
#ifdef USE_ESP32
  if (parent_ != nullptr) {
    auto idf_uart = static_cast<uart::IDFUARTComponent *>(parent_);
    ESP_LOGCONFIG(tag, "  UART Bus: %u", idf_uart->get_hw_serial_number());
  }
#endif
}

}  // namespace esphome::waterdrop_serial
