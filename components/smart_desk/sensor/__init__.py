import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import (
    CONF_ID,
    CONF_HEIGHT,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_DURATION,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
    UNIT_MILLISECOND,
)

from ..__init__ import CONF_ID,smart_desk

CODEOWNERS = ["@nicodess"]

ICON_HEIGHT = "mdi:human-male-height-variant"
CONF_TARGET_HEIGHT = "target_height"
CONF_LEARNED_IDLE_INTERVAL = "learned_idle_interval"
ICON_TARGET_HEIGHT = "mdi:target"
ICON_LEARNED_IDLE_INTERVAL = "mdi:timer-sync-outline"

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
    cv.Optional(CONF_TARGET_HEIGHT): sensor.sensor_schema(
        unit_of_measurement=UNIT_CENTIMETER,
        icon=ICON_TARGET_HEIGHT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_DISTANCE,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_LEARNED_IDLE_INTERVAL): sensor.sensor_schema(
        unit_of_measurement=UNIT_MILLISECOND,
        icon=ICON_LEARNED_IDLE_INTERVAL,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DURATION,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_ID])
    
    if conf_height_val := config.get(CONF_HEIGHT):
        sensor_height = await sensor.new_sensor(conf_height_val)
        cg.add(paren.set_sensor_height(sensor_height))

    if conf_target_height_val := config.get(CONF_TARGET_HEIGHT):
        sensor_target_height = await sensor.new_sensor(conf_target_height_val)
        cg.add(paren.set_sensor_target_height(sensor_target_height))

    if conf_learned_idle_interval_val := config.get(CONF_LEARNED_IDLE_INTERVAL):
        sensor_learned_idle_interval = await sensor.new_sensor(conf_learned_idle_interval_val)
        cg.add(paren.set_sensor_learned_idle_interval(sensor_learned_idle_interval))
