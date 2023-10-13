import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
from esphome.core import CORE, HexInt, coroutine_with_priority
from esphome.const import (
    CONF_ID
)

CONFLICTS_WITH = ["wifi", "ethernet"]
DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["network"]

openthread_ns = cg.esphome_ns.namespace("openthread")
OpenthreadComponent = openthread_ns.class_("OpenthreadComponent", cg.Component)

CONF_NETWORK_KEY = 'network_key'
CONF_CHANNEL = 'channel'

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(): cv.declare_id(OpenthreadComponent),
  cv.Required(CONF_NETWORK_KEY): cv.string,
  cv.Optional(CONF_CHANNEL, default=10): cv.uint8_t,
}).extend(cv.COMPONENT_SCHEMA)


@coroutine_with_priority(60.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_network_key(config[CONF_NETWORK_KEY]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))

    await cg.register_component(var, config)
    cg.add_define("USE_OPENTHREAD")

