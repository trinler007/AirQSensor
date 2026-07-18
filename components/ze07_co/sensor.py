import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CARBON_MONOXIDE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
)

CONF_CO = "co"
CONF_FULL_SCALE = "full_scale"
CONF_FRAME_ERRORS = "frame_errors"

ze07_co_ns = cg.esphome_ns.namespace("ze07_co")
ZE07COComponent = ze07_co_ns.class_("ZE07COComponent", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ZE07COComponent),
            cv.Optional(CONF_CO): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CARBON_MONOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FULL_SCALE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_FRAME_ERRORS): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if co_config := config.get(CONF_CO):
        sens = await sensor.new_sensor(co_config)
        cg.add(var.set_co_sensor(sens))
    if full_scale_config := config.get(CONF_FULL_SCALE):
        sens = await sensor.new_sensor(full_scale_config)
        cg.add(var.set_full_scale_sensor(sens))
    if frame_errors_config := config.get(CONF_FRAME_ERRORS):
        sens = await sensor.new_sensor(frame_errors_config)
        cg.add(var.set_frame_errors_sensor(sens))

