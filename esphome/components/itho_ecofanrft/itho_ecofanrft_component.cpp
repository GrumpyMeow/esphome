#include "itho_ecofanrft.h"
#include "esphome/core/log.h"

namespace esphome {
namespace itho_ecofanrft {

static const char *TAG = "itho_ecofanrft.component";

void itho_ecofanrft::IthoEcoFanRftComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Fan '%s':", this->fan_->get_name().c_str());
}
void IthoEcoFanRftComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up IthoEcoFanRft...");
  this->spi_setup();

  auto traits = fan::FanTraits(false, true);    // No oscillating, just speed
  this->fan_->set_traits(traits);
  this->fan_->add_on_state_callback([this]() { this->next_update_ = true; });
}
void IthoEcoFanRftComponent::loop() {
  if (!this->next_update_) {
    return;
  }
  this->next_update_ = false;

  {
    bool enable = this->fan_->state;
    if (enable) {
     // this->output_->turn_on();
    } else {
     // this->output_->turn_off();
    }
    ESP_LOGD(TAG, "Setting itho_ecofanrft state: %s", ONOFF(enable));
  }
}
float IthoEcoFanRftComponent::get_setup_priority() const { return setup_priority::DATA; }

void IthoEcoFanRftComponent::join() {
  ESP_LOGVV(TAG, "Fan '%s': join() called", this->fan_->get_name().c_str());
}

} // namespace itho_ecofanrft
}  // namespace esphome
