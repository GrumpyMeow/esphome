#include "comet_blue_climate.h"
#include "esphome/core/log.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace comet_blue {

void CometBlueClimate::setup() {
 
  // restore set points
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->to_call(this).perform();
  } else {
    // restore from defaults, change_away handles those for us
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->change_away_(false);
  }
}
void CometBlueClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature_low().has_value())
    this->target_temperature_low = *call.get_target_temperature_low();
  if (call.get_target_temperature_high().has_value())
    this->target_temperature_high = *call.get_target_temperature_high();
  if (call.get_away().has_value())
    this->change_away_(*call.get_away());

  this->compute_state_();
  this->publish_state();
}
climate::ClimateTraits CometBlueClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_auto_mode(true);
  traits.set_supports_cool_mode(this->supports_cool_);
  traits.set_supports_heat_mode(this->supports_heat_);
  traits.set_supports_two_point_target_temperature(true);
  traits.set_supports_away(this->supports_away_);
  traits.set_supports_action(true);
  return traits;
}
void CometBlueClimate::compute_state_() {
  if (this->mode != climate::CLIMATE_MODE_AUTO) {
    // in non-auto mode, switch directly to appropriate action
    //  - HEAT mode -> HEATING action
    //  - COOL mode -> COOLING action
    //  - OFF mode -> OFF action (not IDLE!)
    //this->switch_to_action_(static_cast<climate::ClimateAction>(this->mode));
    return;
  }
  if (isnan(this->current_temperature) || isnan(this->target_temperature_low) || isnan(this->target_temperature_high)) {
    // if any control parameters are nan, go to OFF action (not IDLE!)
    //this->switch_to_action_(climate::CLIMATE_ACTION_OFF);
    return;
  }
  const bool too_cold = this->current_temperature < this->target_temperature_low;
  const bool too_hot = this->current_temperature > this->target_temperature_high;

  climate::ClimateAction target_action;
  if (too_cold) {
    // too cold -> enable heating if possible, else idle
    if (this->supports_heat_)
      target_action = climate::CLIMATE_ACTION_HEATING;
    else
      target_action = climate::CLIMATE_ACTION_IDLE;
  } else if (too_hot) {
     //too hot -> enable cooling if possible, else idle
    if (this->supports_cool_)
      target_action = climate::CLIMATE_ACTION_COOLING;
    else
      target_action = climate::CLIMATE_ACTION_IDLE;
  } else {
    // neither too hot nor too cold -> in range
    if (this->supports_cool_ && this->supports_heat_) {
      // if supports both ends, go to idle action
      target_action = climate::CLIMATE_ACTION_IDLE;
    } else {
      // else use current mode and don't change (hysteresis)
      target_action = this->action;
    }
  }

  this->switch_to_action_(target_action);
}
void CometBlueClimate::switch_to_action_(climate::ClimateAction action) {
  if (action == this->action)
    // already in target mode
    return;

  if ((action == climate::CLIMATE_ACTION_OFF && this->action == climate::CLIMATE_ACTION_IDLE) ||
      (action == climate::CLIMATE_ACTION_IDLE && this->action == climate::CLIMATE_ACTION_OFF)) {
    // switching from OFF to IDLE or vice-versa
    // these only have visual difference. OFF means user manually disabled,
    // IDLE means it's in auto mode but value is in target range.
    this->action = action;
    this->publish_state();
    return;
  }

  if (this->prev_trigger_ != nullptr) {
    this->prev_trigger_->stop();
    this->prev_trigger_ = nullptr;
  }
  Trigger<> *trig;
  switch (action) {
    case climate::CLIMATE_ACTION_OFF:
    case climate::CLIMATE_ACTION_IDLE:
      trig = this->idle_trigger_;
      break;
    case climate::CLIMATE_ACTION_COOLING:
      trig = this->cool_trigger_;
      break;
    case climate::CLIMATE_ACTION_HEATING:
      trig = this->heat_trigger_;
      break;
    default:
      trig = nullptr;
  }
  assert(trig != nullptr);
  trig->trigger();
  this->action = action;
  this->prev_trigger_ = trig;
  this->publish_state();
}
void CometBlueClimate::change_away_(bool away) {
  if (!away) {
    this->target_temperature_low = this->normal_config_.default_temperature_low;
    this->target_temperature_high = this->normal_config_.default_temperature_high;
  } else {
    this->target_temperature_low = this->away_config_.default_temperature_low;
    this->target_temperature_high = this->away_config_.default_temperature_high;
  }
  this->away = away;
}
void CometBlueClimate::set_normal_config(const CometBlueClimateTargetTempConfig &normal_config) {
  this->normal_config_ = normal_config;
}
void CometBlueClimate::set_away_config(const CometBlueClimateTargetTempConfig &away_config) {
  this->supports_away_ = true;
  this->away_config_ = away_config;
}
CometBlueClimate::CometBlueClimate()
    : idle_trigger_(new Trigger<>()), cool_trigger_(new Trigger<>()), heat_trigger_(new Trigger<>()) {}
Trigger<> *CometBlueClimate::get_idle_trigger() const { return this->idle_trigger_; }
Trigger<> *CometBlueClimate::get_cool_trigger() const { return this->cool_trigger_; }
void CometBlueClimate::set_supports_cool(bool supports_cool) { this->supports_cool_ = supports_cool; }
Trigger<> *CometBlueClimate::get_heat_trigger() const { return this->heat_trigger_; }
void CometBlueClimate::set_supports_heat(bool supports_heat) { this->supports_heat_ = supports_heat; }
void CometBlueClimate::dump_config() {
  LOG_CLIMATE("", "Comet Blue Climate", this);
  ESP_LOGCONFIG(TAG, "  Supports HEAT: %s", YESNO(this->supports_heat_));
  ESP_LOGCONFIG(TAG, "  Supports COOL: %s", YESNO(this->supports_cool_));
  ESP_LOGCONFIG(TAG, "  Supports AWAY mode: %s", YESNO(this->supports_away_));
  ESP_LOGCONFIG(TAG, "  Default Target Temperature Low: %.1f°C", this->normal_config_.default_temperature_low);
  ESP_LOGCONFIG(TAG, "  Default Target Temperature High: %.1f°C", this->normal_config_.default_temperature_high);
}

CometBlueClimateTargetTempConfig::CometBlueClimateTargetTempConfig() = default;
CometBlueClimateTargetTempConfig::CometBlueClimateTargetTempConfig(float default_temperature_low,
                                                                 float default_temperature_high)
    : default_temperature_low(default_temperature_low), default_temperature_high(default_temperature_high) {}

}  // namespace comet_blue
}  // namespace esphome

#endif