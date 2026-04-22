#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome::waterdrop_serial {

class WaterdropSerial : public Component, protected uart::UARTDevice {
 public:
  using uart::UARTDevice::set_uart_parent;

 protected:
  void dump_base_config(const char *tag) const;
};

}  // namespace esphome::waterdrop_serial
