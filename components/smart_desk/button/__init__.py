import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from ..__init__ import (
    CONF_ID,
    smart_desk,
    nicode_smart_desk_ns,
)

CODEOWNERS = ["@nicodess"]

SmartDeskButton = nicode_smart_desk_ns.class_("SmartDeskButton", button.Button)


CONF_MOVE_TO_LEVEL_1 = "move_to_level_1"
CONF_MOVE_TO_LEVEL_2 = "move_to_level_2"
CONF_MOVE_TO_LEVEL_3 = "move_to_level_3"
CONF_MOVE_TO_LEVEL_4 = "move_to_level_4"
CONF_MOVE_UP = "move_up"
CONF_MOVE_DOWN = "move_down"
CONF_SET_MEMORY = "set_memory"
CONF_AWAKE_HANDSET = "awake_handset"
CONF_RESET_DESK = "reset_desk"

ICON_MOVE_TO_LEVEL_1 = "mdi:numeric-1-box"
ICON_MOVE_TO_LEVEL_2 = "mdi:numeric-2-box"
ICON_MOVE_TO_LEVEL_3 = "mdi:numeric-3-box"
ICON_MOVE_TO_LEVEL_4 = "mdi:numeric-4-box"
ICON_MOVE_UP = "mdi:chevron-up-box"
ICON_MOVE_DOWN = "mdi:chevron-down-box"
ICON_SET_MEMORY = "mdi:alpha-m-box"
ICON_AWAKE_HANDSET = "mdi:text-box"
ICON_RESET_DESK = "mdi:restore"

BUTTON_COMMANDS = {
    CONF_MOVE_TO_LEVEL_1: "1",
    CONF_MOVE_TO_LEVEL_2: "2",
    CONF_MOVE_TO_LEVEL_3: "3",
    CONF_MOVE_TO_LEVEL_4: "4",
    CONF_MOVE_UP: "U",
    CONF_MOVE_DOWN: "D",
    CONF_SET_MEMORY: "M",
    CONF_AWAKE_HANDSET: "P",
}

BUTTON_REPEATS = {
    CONF_AWAKE_HANDSET: 1,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
        cv.Optional(CONF_MOVE_TO_LEVEL_1): button.button_schema(SmartDeskButton, icon=ICON_MOVE_TO_LEVEL_1),
        cv.Optional(CONF_MOVE_TO_LEVEL_2): button.button_schema(SmartDeskButton, icon=ICON_MOVE_TO_LEVEL_2),
        cv.Optional(CONF_MOVE_TO_LEVEL_3): button.button_schema(SmartDeskButton, icon=ICON_MOVE_TO_LEVEL_3),
        cv.Optional(CONF_MOVE_TO_LEVEL_4): button.button_schema(SmartDeskButton, icon=ICON_MOVE_TO_LEVEL_4),
        cv.Optional(CONF_MOVE_UP): button.button_schema(SmartDeskButton, icon=ICON_MOVE_UP),
        cv.Optional(CONF_MOVE_DOWN): button.button_schema(SmartDeskButton, icon=ICON_MOVE_DOWN),
        cv.Optional(CONF_SET_MEMORY): button.button_schema(SmartDeskButton, icon=ICON_SET_MEMORY),
        cv.Optional(CONF_AWAKE_HANDSET): button.button_schema(SmartDeskButton, icon=ICON_AWAKE_HANDSET),
        cv.Optional(CONF_RESET_DESK): button.button_schema(SmartDeskButton, icon=ICON_RESET_DESK),
    }
)


async def to_code(config):
    cg.add_global(cg.RawStatement('#include "esphome/components/smart_desk/button/smart_desk_button.h"'))
    for button_type, command in BUTTON_COMMANDS.items():
        if conf := config.get(button_type):
            btn = await button.new_button(conf)
            await cg.register_parented(btn, config[CONF_ID])
            cg.add(btn.set_command(command))
            if repeat := BUTTON_REPEATS.get(button_type):
                cg.add(btn.set_repeat(repeat))
    if conf := config.get(CONF_RESET_DESK):
        btn = await button.new_button(conf)
        await cg.register_parented(btn, config[CONF_ID])
        cg.add(btn.set_reset(True))
