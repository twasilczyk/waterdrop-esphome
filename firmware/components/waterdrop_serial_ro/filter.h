#pragma once

#include <cstdint>
#include <optional>

namespace esphome::binary_sensor {
class BinarySensor;
}

namespace esphome::sensor {
class Sensor;
}

namespace esphome::waterdrop_serial::ro::filter {

enum class Type : uint8_t {
  CF,
  RO,
  CB,
  COUNT_,
};

struct Sensors {
  Sensors();
  Sensors(sensor::Sensor *total_life, sensor::Sensor *remaining_life,
      sensor::Sensor *remaining_percent, binary_sensor::BinarySensor *health);

  sensor::Sensor *total_life;
  sensor::Sensor *remaining_life;
  sensor::Sensor *remaining_percent;
  binary_sensor::BinarySensor *health;
};

class Filter {
 public:
  void set_sensors(Sensors sensors);
  void set_total_life(uint16_t total_life);
  void set_used_life(uint16_t used_life);

 private:
  void publish_remaining_();

  Sensors sensors_{};
  std::optional<uint16_t> total_life_;
  std::optional<uint16_t> used_life_;
};

}  // namespace esphome::waterdrop_serial::ro::filter
