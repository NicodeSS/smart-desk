import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

nicode_smart_desk_ns = cg.esphome_ns.namespace('nicode_smart_desk')

SmartDesk = nicode_smart_desk_ns.class_('SmartDesk', cg.Component)
smart_desk = SmartDesk

CONF_UART_CONTROL = "uart_control"
CONF_UART_HANDSET = "uart_handset"
CONF_DEFAULT_TX_COMMAND_REPEAT = 'default_tx_command_repeat'
CONF_MAX_HANDSET_TIMEOUT_COUNT = 'max_handset_timeout_count'
CONF_MIN_HEIGHT = 'min_height'
CONF_MAX_HEIGHT = 'max_height'
CONF_OFFLINE_TX_INTERVAL = 'offline_tx_interval'

def validate_config(config):
    if config[CONF_MIN_HEIGHT] >= config[CONF_MAX_HEIGHT]:
        raise cv.Invalid(f"{CONF_MIN_HEIGHT} must be lower than {CONF_MAX_HEIGHT}")
    return config


CONFIG_SCHEMA = cv.All(
    cv.COMPONENT_SCHEMA.extend({
        cv.GenerateID(): cv.declare_id(SmartDesk),
        cv.Required(CONF_UART_CONTROL): cv.use_id(uart.UARTComponent),
        cv.Required(CONF_UART_HANDSET): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_DEFAULT_TX_COMMAND_REPEAT, default=5): cv.positive_not_null_int,
        cv.Optional(CONF_MAX_HANDSET_TIMEOUT_COUNT, default=5000): cv.positive_not_null_int,
        cv.Optional(CONF_MIN_HEIGHT, default=62.0): cv.float_range(min=0),
        cv.Optional(CONF_MAX_HEIGHT, default=127.0): cv.float_range(min=0),
        cv.Optional(CONF_OFFLINE_TX_INTERVAL, default="20ms"): cv.positive_time_period_milliseconds,
    }),
    validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    uart_control = await cg.get_variable(config[CONF_UART_CONTROL])
    cg.add(var.set_uart_control(uart_control))
    uart_handset = await cg.get_variable(config[CONF_UART_HANDSET])
    cg.add(var.set_uart_handset(uart_handset))

    cg.add(var.set_default_command_repeat(config[CONF_DEFAULT_TX_COMMAND_REPEAT]))
    cg.add(var.set_max_handset_timeout_count(config[CONF_MAX_HANDSET_TIMEOUT_COUNT]))
    cg.add(var.set_min_height(config[CONF_MIN_HEIGHT]))
    cg.add(var.set_max_height(config[CONF_MAX_HEIGHT]))
    cg.add(var.set_offline_tx_interval_ms(config[CONF_OFFLINE_TX_INTERVAL].total_milliseconds))
