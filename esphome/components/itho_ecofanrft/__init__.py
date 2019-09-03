import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import spi
from esphome.const import CONF_ID

DEPENDENCIES = ['spi']

CONF_ITHO_ECOFANRFT_ID = 'itho_ecofanrft_id'

itho_ecofanrft_ns = cg.esphome_ns.namespace('itho_ecofanrft')
IthoEcoFanRftComponent = itho_ecofanrft_ns.class_('IthoEcoFanRftComponent',
                                                  cg.Component, spi.SPIDevice)

# Actions
JoinAction = itho_ecofanrft_ns.class_('JoinAction', automation.Action)

MULTI_CONF = True
AUTOLOAD = ['fan']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(IthoEcoFanRftComponent),
}).extend(cv.COMPONENT_SCHEMA).extend(spi.SPI_DEVICE_SCHEMA)


ECOFAN_ACTION_SCHEMA = maybe_simple_id({
    cv.Required(CONF_ID): cv.use_id(IthoEcoFanRftComponent),
})


@automation.register_action('itho_ecofanrft.join', JoinAction, ECOFAN_ACTION_SCHEMA)
def fan_join_to_code(config, action_id, template_arg, args):
    paren = yield cg.get_variable(config[CONF_ID])
    yield cg.new_Pvariable(action_id, template_arg, paren)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield spi.register_spi_device(var, config)
