#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome::ze07_co {

class ZE07COComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_co_sensor(sensor::Sensor *sensor) { this->co_sensor_ = sensor; }
  void set_full_scale_sensor(sensor::Sensor *sensor) { this->full_scale_sensor_ = sensor; }
  void set_frame_errors_sensor(sensor::Sensor *sensor) { this->frame_errors_sensor_ = sensor; }
  void set_connected_binary_sensor(binary_sensor::BinarySensor *sensor) { this->connected_binary_sensor_ = sensor; }

 protected:
  void consume_byte_(uint8_t byte);
  bool validate_frame_() const;
  void publish_frame_();
  void record_error_();

  sensor::Sensor *co_sensor_{nullptr};
  sensor::Sensor *full_scale_sensor_{nullptr};
  sensor::Sensor *frame_errors_sensor_{nullptr};
  binary_sensor::BinarySensor *connected_binary_sensor_{nullptr};

  uint8_t frame_[9]{};
  uint8_t frame_position_{0};
  uint32_t last_valid_frame_ms_{0};
  uint32_t frame_errors_{0};
  bool connected_{false};
};

}  // namespace esphome::ze07_co

