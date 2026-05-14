import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
    CONF_STEP,
    DEVICE_CLASS_DISTANCE,
    UNIT_CENTIMETER,
)

from .. import (
    CONF_MAX_HEIGHT,
    CONF_MIN_HEIGHT,
    SmartDesk,
    nicode_smart_desk_ns,
)

CODEOWNERS = ["@nicodess"]

CONF_SMART_DESK_ID = "smart_desk_id"

SmartDeskTargetHeightNumber = nicode_smart_desk_ns.class_(
    "SmartDeskTargetHeightNumber", number.Number, cg.Component
)


def validate_config(config):
    if config[CONF_MIN_HEIGHT] >= config[CONF_MAX_HEIGHT]:
        raise cv.Invalid(f"{CONF_MIN_HEIGHT} must be lower than {CONF_MAX_HEIGHT}")
    return config


CONFIG_SCHEMA = cv.All(
    number.number_schema(
        SmartDeskTargetHeightNumber,
        unit_of_measurement=UNIT_CENTIMETER,
        device_class=DEVICE_CLASS_DISTANCE,
    )
    .extend(
        {
            cv.GenerateID(CONF_SMART_DESK_ID): cv.use_id(SmartDesk),
            cv.Optional(CONF_MIN_HEIGHT, default=62.0): cv.float_range(min=0),
            cv.Optional(CONF_MAX_HEIGHT, default=127.0): cv.float_range(min=0),
            cv.Optional(CONF_STEP, default=0.5): cv.float_range(min=0.1, max=10.0),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    validate_config,
)


async def to_code(config):
    var = await number.new_number(
        config,
        min_value=config[CONF_MIN_HEIGHT],
        max_value=config[CONF_MAX_HEIGHT],
        step=config[CONF_STEP],
    )
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_SMART_DESK_ID])
