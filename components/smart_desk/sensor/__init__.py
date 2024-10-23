import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import CONF_ID,CONF_HEIGHT,UNIT_CENTIMETER,DEVICE_CLASS_DISTANCE,STATE_CLASS_MEASUREMENT

from ..__init__ import CONF_ID,smart_desk

CODEOWNERS = ["@nicodess"]

ICON_HEIGHT = "mdi:human-male-height-variant"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
}).extend({
    cv.Optional(CONF_HEIGHT): sensor.sensor_schema(
        unit_of_measurement=UNIT_CENTIMETER,
        icon=ICON_HEIGHT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_DISTANCE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_ID])
    
    if conf_height_val := config.get(CONF_HEIGHT):
        sensor_height = await sensor.new_sensor(conf_height_val)
        cg.add(paren.set_sensor_height(sensor_height))
