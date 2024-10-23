import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from esphome.const import CONF_ID,DEVICE_CLASS_CONNECTIVITY,ENTITY_CATEGORY_DIAGNOSTIC

from ..__init__ import CONF_ID,smart_desk

CODEOWNERS = ["@nicodess"]

CONF_HANDSET_ONLINE = "handset_online"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
}).extend({
    cv.Optional(CONF_HANDSET_ONLINE): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_ID])
    
    if conf_handset_online_val := config.get(CONF_HANDSET_ONLINE):
        binary_sensor_handset_online = await binary_sensor.new_binary_sensor(conf_handset_online_val)
        cg.add(paren.set_binary_sensor_handset_online(binary_sensor_handset_online))
