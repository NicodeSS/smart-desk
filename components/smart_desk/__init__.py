import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

nicode_smart_desk_ns = cg.esphome_ns.namespace('nicode_smart_desk')

smart_desk = nicode_smart_desk_ns.class_('SmartDesk', cg.Component)

CONF_UART_CONTROL = "uart_control"
CONF_UART_HANDSET = "uart_handset"
CONF_DEFAULT_TX_COMMAND_REPEAT = 'default_tx_command_repeat'
CONF_MAX_HANDSET_TIMEOUT_COUNT = 'max_handset_timeout_count'
CONF_MIN_HEIGHT = 'min_height'
CONF_MAX_HEIGHT = 'max_height'

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(smart_desk),
    cv.Required(CONF_UART_CONTROL): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_UART_HANDSET): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_DEFAULT_TX_COMMAND_REPEAT): cv.int_,
    cv.Optional(CONF_MAX_HANDSET_TIMEOUT_COUNT): cv.int_,
    cv.Optional(CONF_MIN_HEIGHT): cv.float_,
    cv.Optional(CONF_MAX_HEIGHT): cv.float_,
})


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    uart_control = await cg.get_variable(config[CONF_UART_CONTROL])
    cg.add(var.set_uart_control(uart_control))
    uart_handset = await cg.get_variable(config[CONF_UART_HANDSET])
    cg.add(var.set_uart_handset(uart_handset))
    
    if config_default_tx_command_repeat := config.get(CONF_DEFAULT_TX_COMMAND_REPEAT):
        cg.add(var.set_default_command_repeat(config_default_tx_command_repeat))
        
    if config_max_handset_timeout_count:= config.get(CONF_MAX_HANDSET_TIMEOUT_COUNT):
        cg.add(var.set_max_handset_timeout_count(config_max_handset_timeout_count))
        
    if config_min_height := config.get(CONF_MIN_HEIGHT):
        cg.add(var.set_min_height(config_min_height))
    
    if config_max_height := config.get(CONF_MAX_HEIGHT):
        cg.add(var.set_max_height(config_max_height))
