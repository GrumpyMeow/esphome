import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, esp32_ble_tracker
from esphome.const import CONF_AWAY_CONFIG, \
    CONF_DEFAULT_TARGET_TEMPERATURE_HIGH, CONF_DEFAULT_TARGET_TEMPERATURE_LOW,  \
    CONF_ID, CONF_MAC_ADDRESS, CONF_PASSWORD

DEPENDENCIES = ['esp32_ble_tracker']

comet_blue_ns = cg.esphome_ns.namespace('comet_blue')
CometBlueClimate = comet_blue_ns.class_('CometBlueClimate', climate.Climate, cg.Component,
                                        esp32_ble_tracker.ESPBTDeviceListener)
CometBlueClimateTargetTempConfig = comet_blue_ns.struct('CometBlueClimateTargetTempConfig')

CONFIG_SCHEMA = cv.All(climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(CometBlueClimate),
    cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
    cv.Optional(CONF_PASSWORD, default=0): cv.int_range(min_included=0, max_included=9999999),
    cv.Required(CONF_DEFAULT_TARGET_TEMPERATURE_LOW): cv.temperature,
    cv.Required(CONF_DEFAULT_TARGET_TEMPERATURE_HIGH): cv.temperature,
    cv.Optional(CONF_AWAY_CONFIG): cv.Schema({
        cv.Required(CONF_DEFAULT_TARGET_TEMPERATURE_LOW): cv.temperature,
        cv.Required(CONF_DEFAULT_TARGET_TEMPERATURE_HIGH): cv.temperature,
    })
}).extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield esp32_ble_tracker.register_ble_device(var, config)
    yield climate.register_climate(var, config)

    normal_config = CometBlueClimateTargetTempConfig(
        config[CONF_DEFAULT_TARGET_TEMPERATURE_LOW],
        config[CONF_DEFAULT_TARGET_TEMPERATURE_HIGH]
    )
    cg.add(var.set_normal_config(normal_config))

    if CONF_MAC_ADDRESS in config:
        cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))

    cg.add(var.set_password(config[CONF_PASSWORD]))

    if CONF_AWAY_CONFIG in config:
        away = config[CONF_AWAY_CONFIG]
        away_config = CometBlueClimateTargetTempConfig(
            away[CONF_DEFAULT_TARGET_TEMPERATURE_LOW],
            away[CONF_DEFAULT_TARGET_TEMPERATURE_HIGH]
        )
        cg.add(var.set_away_config(away_config))
