import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from esphome.const import CONF_STATUS,CONF_DISPLAY,DEVICE_CLASS_RUNNING,ENTITY_CATEGORY_DIAGNOSTIC,ENTITY_CATEGORY_NONE

from ..__init__ import CONF_ID,smart_desk

CODEOWNERS = ["@nicodess"]

ICON_STATUS = "mdi:adjust"
ICON_DISPLAY = "mdi:text-box-outline"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
}).extend({
    cv.Optional(CONF_STATUS): text_sensor.text_sensor_schema(icon=ICON_STATUS,entity_category=ENTITY_CATEGORY_NONE),
    cv.Optional(CONF_DISPLAY): text_sensor.text_sensor_schema(
        icon=ICON_DISPLAY,entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    )
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_ID])
    
    if conf_status_val := config.get(CONF_STATUS):
        sensor_status = await text_sensor.new_text_sensor(conf_status_val)
        cg.add(paren.set_text_sensor_status(sensor_status))
        
    if conf_display_val := config.get(CONF_DISPLAY):
        sensor_display = await text_sensor.new_text_sensor(conf_display_val)
        cg.add(paren.set_text_sensor_display(sensor_display))
