import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

nicode_smart_desk_ns = cg.esphome_ns.namespace('nicode_smart_desk')

smart_desk = nicode_smart_desk_ns.class_('SmartDesk', cg.Component)

CONF_UART_CONTROL = "uart_control"
CONF_UART_HANDSET = "uart_handset"

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(smart_desk),
    cv.Required(CONF_UART_CONTROL): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_UART_HANDSET): cv.use_id(uart.UARTComponent),
})


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    uart_control = await cg.get_variable(config[CONF_UART_CONTROL])
    cg.add(var.set_uart_control(uart_control))
    uart_handset = await cg.get_variable(config[CONF_UART_HANDSET])
    cg.add(var.set_uart_handset(uart_handset))