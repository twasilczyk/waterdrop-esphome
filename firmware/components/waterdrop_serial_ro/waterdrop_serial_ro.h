#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "frame.h"

namespace esphome::waterdrop_serial_ro {

class WaterdropSerialRo : public Component, private uart::UARTDevice {
 public:
  void loop() override;
  void dump_config() override;
  using uart::UARTDevice::set_uart_parent;

 private:
  void handle_frame_(const frame::Frame &frame);

  frame::Parser parser_{};
};

}  // namespace esphome::waterdrop_serial_ro
