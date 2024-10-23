import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from ..__init__ import (
    CONF_ID,
    smart_desk,
    nicode_smart_desk_ns,
)

CODEOWNERS = ["@nicodess"]

MoveToLevel1Button = nicode_smart_desk_ns.class_("SmartDeskMoveToLevel1Button", button.Button)
MoveToLevel2Button = nicode_smart_desk_ns.class_("SmartDeskMoveToLevel2Button", button.Button)
MoveToLevel3Button = nicode_smart_desk_ns.class_("SmartDeskMoveToLevel3Button", button.Button)
MoveToLevel4Button = nicode_smart_desk_ns.class_("SmartDeskMoveToLevel4Button", button.Button)
MoveUpButton = nicode_smart_desk_ns.class_("SmartDeskMoveUpButton", button.Button)
MoveDownButton = nicode_smart_desk_ns.class_("SmartDeskMoveDownButton", button.Button)
SetMemoryButton = nicode_smart_desk_ns.class_("SmartDeskSetMemoryButton", button.Button)
AwakeHandsetButton = nicode_smart_desk_ns.class_("SmartDeskAwakeHandsetButton", button.Button)


CONF_MOVE_TO_LEVEL_1 = "move_to_level_1"
CONF_MOVE_TO_LEVEL_2 = "move_to_level_2"
CONF_MOVE_TO_LEVEL_3 = "move_to_level_3"
CONF_MOVE_TO_LEVEL_4 = "move_to_level_4"
CONF_MOVE_UP = "move_up"
CONF_MOVE_DOWN = "move_down"
CONF_SET_MEMORY = "set_memory"
CONF_AWAKE_HANDSET = "awake_handset"

ICON_MOVE_TO_LEVEL_1 = "mdi:numeric-1-box"
ICON_MOVE_TO_LEVEL_2 = "mdi:numeric-2-box"
ICON_MOVE_TO_LEVEL_3 = "mdi:numeric-3-box"
ICON_MOVE_TO_LEVEL_4 = "mdi:numeric-4-box"
ICON_MOVE_UP = "mdi:chevron-up-box"
ICON_MOVE_DOWN = "mdi:chevron-down-box"
ICON_SET_MEMORY = "mdi:alpha-m-box"
ICON_AWAKE_HANDSET = "mdi:text-box"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(smart_desk),
        cv.Optional(CONF_MOVE_TO_LEVEL_1): button.button_schema(MoveToLevel1Button,icon=ICON_MOVE_TO_LEVEL_1),
        cv.Optional(CONF_MOVE_TO_LEVEL_2): button.button_schema(MoveToLevel2Button,icon=ICON_MOVE_TO_LEVEL_2),
        cv.Optional(CONF_MOVE_TO_LEVEL_3): button.button_schema(MoveToLevel3Button,icon=ICON_MOVE_TO_LEVEL_3),
        cv.Optional(CONF_MOVE_TO_LEVEL_4): button.button_schema(MoveToLevel4Button,icon=ICON_MOVE_TO_LEVEL_4),
        cv.Optional(CONF_MOVE_UP): button.button_schema(MoveUpButton,icon=ICON_MOVE_UP),
        cv.Optional(CONF_MOVE_DOWN): button.button_schema(MoveDownButton,icon=ICON_MOVE_DOWN),
        cv.Optional(CONF_SET_MEMORY): button.button_schema(SetMemoryButton,icon=ICON_SET_MEMORY),
        cv.Optional(CONF_AWAKE_HANDSET): button.button_schema(AwakeHandsetButton,icon=ICON_AWAKE_HANDSET),
    }
)


async def to_code(config):
    for button_type in [CONF_MOVE_TO_LEVEL_1,CONF_MOVE_TO_LEVEL_2,CONF_MOVE_TO_LEVEL_3,CONF_MOVE_TO_LEVEL_4,CONF_MOVE_UP,CONF_MOVE_DOWN,CONF_SET_MEMORY,CONF_AWAKE_HANDSET]:
        if conf := config.get(button_type):
            btn = await button.new_button(conf)
            await cg.register_parented(btn, config[CONF_ID])