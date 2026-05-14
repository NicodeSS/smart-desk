import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from esphome.const import CONF_STATUS, CONF_DISPLAY, ENTITY_CATEGORY_DIAGNOSTIC, ENTITY_CATEGORY_NONE

from ..__init__ import CONF_ID,smart_desk

CODEOWNERS = ["@nicodess"]

ICON_STATUS = "mdi:adjust"
ICON_DISPLAY = "mdi:text-box-outline"
CONF_MOVEMENT = "movement"
CONF_LAST_MOVE_RESULT = "last_move_result"
ICON_MOVEMENT = "mdi:desk"
ICON_LAST_MOVE_RESULT = "mdi:check-decagram-outline"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
}).extend({
    cv.Optional(CONF_STATUS): text_sensor.text_sensor_schema(icon=ICON_STATUS,entity_category=ENTITY_CATEGORY_NONE),
    cv.Optional(CONF_DISPLAY): text_sensor.text_sensor_schema(
        icon=ICON_DISPLAY,entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_MOVEMENT): text_sensor.text_sensor_schema(
        icon=ICON_MOVEMENT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_LAST_MOVE_RESULT): text_sensor.text_sensor_schema(
        icon=ICON_LAST_MOVE_RESULT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_ID])
    
    if conf_status_val := config.get(CONF_STATUS):
        sensor_status = await text_sensor.new_text_sensor(conf_status_val)
        cg.add(paren.set_text_sensor_status(sensor_status))
        
    if conf_display_val := config.get(CONF_DISPLAY):
        sensor_display = await text_sensor.new_text_sensor(conf_display_val)
        cg.add(paren.set_text_sensor_display(sensor_display))

    if conf_movement_val := config.get(CONF_MOVEMENT):
        sensor_movement = await text_sensor.new_text_sensor(conf_movement_val)
        cg.add(paren.set_text_sensor_movement(sensor_movement))

    if conf_last_move_result_val := config.get(CONF_LAST_MOVE_RESULT):
        sensor_last_move_result = await text_sensor.new_text_sensor(conf_last_move_result_val)
        cg.add(paren.set_text_sensor_last_move_result(sensor_last_move_result))
