#include "ze07_co.h"

#include <cmath>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::ze07_co {

static const char *const TAG = "ze07_co";
static const uint32_t CONNECTION_TIMEOUT_MS = 5000;

void ZE07COComponent::setup() {
  if (this->connected_binary_sensor_ != nullptr) this->connected_binary_sensor_->publish_state(false);
  if (this->frame_errors_sensor_ != nullptr) this->frame_errors_sensor_->publish_state(0);
}

void ZE07COComponent::loop() {
  while (this->available()) this->consume_byte_(this->read());

  const bool now_connected = this->last_valid_frame_ms_ != 0 &&
                             (millis() - this->last_valid_frame_ms_) <= CONNECTION_TIMEOUT_MS;
  if (now_connected != this->connected_) {
    this->connected_ = now_connected;
    if (this->connected_binary_sensor_ != nullptr) this->connected_binary_sensor_->publish_state(now_connected);
  }
}

void ZE07COComponent::consume_byte_(uint8_t byte) {
  if (this->frame_position_ == 0) {
    if (byte == 0xFF) this->frame_[this->frame_position_++] = byte;
    return;
  }

  this->frame_[this->frame_position_++] = byte;
  if (this->frame_position_ < sizeof(this->frame_)) return;

  if (this->validate_frame_()) {
    this->publish_frame_();
    this->frame_position_ = 0;
    return;
  }

  this->record_error_();
  // A 0xFF checksum can also be the beginning of the next frame.
  if (this->frame_[8] == 0xFF) {
    this->frame_[0] = 0xFF;
    this->frame_position_ = 1;
  } else {
    this->frame_position_ = 0;
  }
}

bool ZE07COComponent::validate_frame_() const {
  if (this->frame_[0] != 0xFF || this->frame_[1] != 0x04 || this->frame_[2] != 0x03) return false;
  if (this->frame_[3] > 3) return false;

  uint8_t checksum = 0;
  for (uint8_t i = 1; i <= 7; i++) checksum += this->frame_[i];
  checksum = static_cast<uint8_t>(~checksum + 1);
  return checksum == this->frame_[8];
}

void ZE07COComponent::publish_frame_() {
  const uint16_t raw_concentration = (static_cast<uint16_t>(this->frame_[4]) << 8) | this->frame_[5];
  const uint16_t raw_full_scale = (static_cast<uint16_t>(this->frame_[6]) << 8) | this->frame_[7];
  const float divisor = std::pow(10.0f, this->frame_[3]);
  const float concentration = raw_concentration / divisor;
  const float full_scale = raw_full_scale / divisor;

  if (concentration < 0.0f || full_scale <= 0.0f || concentration > full_scale * 1.2f) {
    this->record_error_();
    return;
  }

  this->last_valid_frame_ms_ = millis();
  if (this->co_sensor_ != nullptr) this->co_sensor_->publish_state(concentration);
  if (this->full_scale_sensor_ != nullptr) this->full_scale_sensor_->publish_state(full_scale);
  ESP_LOGV(TAG, "CO %.1f ppm (range %.0f ppm)", concentration, full_scale);
}

void ZE07COComponent::record_error_() {
  this->frame_errors_++;
  if (this->frame_errors_sensor_ != nullptr) this->frame_errors_sensor_->publish_state(this->frame_errors_);
  ESP_LOGW(TAG, "Invalid UART frame (%" PRIu32 " total)", this->frame_errors_);
}

void ZE07COComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Winsen ZE07-CO:");
  LOG_SENSOR("  ", "CO", this->co_sensor_);
  LOG_SENSOR("  ", "Full scale", this->full_scale_sensor_);
  LOG_SENSOR("  ", "Frame errors", this->frame_errors_sensor_);
  LOG_BINARY_SENSOR("  ", "Connected", this->connected_binary_sensor_);
  this->check_uart_settings(9600, 1, uart::UART_CONFIG_PARITY_NONE, 8);
}

}  // namespace esphome::ze07_co

