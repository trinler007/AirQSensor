import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from .sensor import ZE07COComponent

CONF_ZE07_CO_ID = "ze07_co_id"
CONF_CONNECTED = "connected"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ZE07_CO_ID): cv.use_id(ZE07COComponent),
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ZE07_CO_ID])
    if connected_config := config.get(CONF_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(connected_config)
        cg.add(parent.set_connected_binary_sensor(sens))

