#include <cxxflags.inc>
#include "filter.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"

#include <cassert>

namespace esphome::waterdrop_serial::ro::filter {

static const char *const TAG = "waterdrop_serial_ro_filter";
static constexpr uint16_t FILTER_REPLACE_SOON = 360;

Sensors::Sensors()
    : total_life(nullptr), remaining_life(nullptr), remaining_percent(nullptr), health(nullptr) {}

Sensors::Sensors(sensor::Sensor *total_life_, sensor::Sensor *remaining_life_,
    sensor::Sensor *remaining_percent_, binary_sensor::BinarySensor *health_)
    : total_life(total_life_), remaining_life(remaining_life_),
      remaining_percent(remaining_percent_), health(health_) {
  assert(total_life != nullptr);
  assert(remaining_life != nullptr);
  assert(remaining_percent != nullptr);
  assert(health != nullptr);
}

void Filter::set_sensors(Sensors sensors) {
  sensors_ = sensors;
}

void Filter::set_total_life(uint16_t total_life) {
  if (total_life == 0) {
    ESP_LOGW(TAG, "Total life cannot be zero");
    return;
  }
  total_life_ = total_life;
  if (sensors_.total_life != nullptr) {
    sensors_.total_life->publish_state(total_life);
  }
  publish_remaining_();
}

void Filter::set_used_life(uint16_t used_life) {
  used_life_ = used_life;
  publish_remaining_();
}

void Filter::publish_remaining_() {
  if (!total_life_.has_value() || !used_life_.has_value()) {
    return;
  }

  const auto total_life = *total_life_;
  assert(total_life > 0);
  const auto used_life = *used_life_;
  const auto remaining_life = static_cast<int32_t>(total_life) - static_cast<int32_t>(used_life);
  const float remaining_life_percent = std::max(0.0f, remaining_life * 100.0f / total_life);

  if (sensors_.remaining_life != nullptr) {
    sensors_.remaining_life->publish_state(remaining_life);
  }
  if (sensors_.remaining_percent != nullptr) {
    sensors_.remaining_percent->publish_state(remaining_life_percent);
  }
  if (sensors_.health != nullptr) {
    sensors_.health->publish_state(remaining_life <= FILTER_REPLACE_SOON);
  }
}

}  // namespace esphome::waterdrop_serial::ro::filter
