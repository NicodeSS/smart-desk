import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome.const import CONF_ID

from .. import SmartDesk, nicode_smart_desk_ns

CODEOWNERS = ["@nicodess"]

CONF_SMART_DESK_ID = "smart_desk_id"

SmartDeskCover = nicode_smart_desk_ns.class_(
    "SmartDeskCover", cover.Cover, cg.Component
)

COVER_SCHEMA = getattr(cover, "COVER_SCHEMA", cover._COVER_SCHEMA)

CONFIG_SCHEMA = COVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SmartDeskCover),
        cv.GenerateID(CONF_SMART_DESK_ID): cv.use_id(SmartDesk),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    await cg.register_parented(var, config[CONF_SMART_DESK_ID])
