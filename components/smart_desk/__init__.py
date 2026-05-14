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
CONF_MOVE_TOLERANCE = 'move_tolerance'
CONF_MOVE_STOP_MARGIN_BASE = 'move_stop_margin_base'
CONF_MOVE_STOP_MARGIN_PER_CM = 'move_stop_margin_per_cm'
CONF_MOVE_STOP_MARGIN_MIN = 'move_stop_margin_min'
CONF_MOVE_STOP_MARGIN_MAX = 'move_stop_margin_max'
CONF_MOVE_ENDPOINT_TOLERANCE = 'move_endpoint_tolerance'
CONF_MOVE_COMMAND_REPEAT = 'move_command_repeat'
CONF_MOVE_COMMAND_INTERVAL = 'move_command_interval'
CONF_MOVE_TIMEOUT = 'move_timeout'
CONF_MOVE_STALL_TIMEOUT = 'move_stall_timeout'
CONF_MOVE_STALL_TOLERANCE = 'move_stall_tolerance'
CONF_MANUAL_MOVE_DEBUG = 'manual_move_debug'
CONF_MANUAL_MOVE_DEBUG_DUMP_FRAMES = 'manual_move_debug_dump_frames'

def validate_config(config):
    if config[CONF_MIN_HEIGHT] >= config[CONF_MAX_HEIGHT]:
        raise cv.Invalid(f"{CONF_MIN_HEIGHT} must be lower than {CONF_MAX_HEIGHT}")
    if config[CONF_MOVE_STOP_MARGIN_MIN] > config[CONF_MOVE_STOP_MARGIN_MAX]:
        raise cv.Invalid(f"{CONF_MOVE_STOP_MARGIN_MIN} must be lower than or equal to {CONF_MOVE_STOP_MARGIN_MAX}")
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
        cv.Optional(CONF_MOVE_TOLERANCE, default=0.4): cv.float_range(min=0.1, max=5.0),
        cv.Optional(CONF_MOVE_STOP_MARGIN_BASE, default=0.3): cv.float_range(min=0.0, max=5.0),
        cv.Optional(CONF_MOVE_STOP_MARGIN_PER_CM, default=0.36): cv.float_range(min=0.0, max=2.0),
        cv.Optional(CONF_MOVE_STOP_MARGIN_MIN, default=0.45): cv.float_range(min=0.0, max=5.0),
        cv.Optional(CONF_MOVE_STOP_MARGIN_MAX, default=1.2): cv.float_range(min=0.0, max=5.0),
        cv.Optional(CONF_MOVE_ENDPOINT_TOLERANCE, default=0.05): cv.float_range(min=0.0, max=1.0),
        cv.Optional(CONF_MOVE_COMMAND_REPEAT, default=4): cv.int_range(min=1, max=16),
        cv.Optional(CONF_MOVE_COMMAND_INTERVAL, default="80ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_MOVE_TIMEOUT, default="30s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_MOVE_STALL_TIMEOUT, default="3s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_MOVE_STALL_TOLERANCE, default=0.2): cv.float_range(min=0.1, max=2.0),
        cv.Optional(CONF_MANUAL_MOVE_DEBUG, default=False): cv.boolean,
        cv.Optional(CONF_MANUAL_MOVE_DEBUG_DUMP_FRAMES, default=True): cv.boolean,
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
    cg.add(var.set_move_tolerance(config[CONF_MOVE_TOLERANCE]))
    cg.add(var.set_move_stop_margin_base(config[CONF_MOVE_STOP_MARGIN_BASE]))
    cg.add(var.set_move_stop_margin_per_cm(config[CONF_MOVE_STOP_MARGIN_PER_CM]))
    cg.add(var.set_move_stop_margin_min(config[CONF_MOVE_STOP_MARGIN_MIN]))
    cg.add(var.set_move_stop_margin_max(config[CONF_MOVE_STOP_MARGIN_MAX]))
    cg.add(var.set_move_endpoint_tolerance(config[CONF_MOVE_ENDPOINT_TOLERANCE]))
    cg.add(var.set_move_command_repeat(config[CONF_MOVE_COMMAND_REPEAT]))
    cg.add(var.set_move_command_interval_ms(config[CONF_MOVE_COMMAND_INTERVAL].total_milliseconds))
    cg.add(var.set_move_timeout_ms(config[CONF_MOVE_TIMEOUT].total_milliseconds))
    cg.add(var.set_move_stall_timeout_ms(config[CONF_MOVE_STALL_TIMEOUT].total_milliseconds))
    cg.add(var.set_move_stall_tolerance(config[CONF_MOVE_STALL_TOLERANCE]))
    cg.add(var.set_manual_move_debug(config[CONF_MANUAL_MOVE_DEBUG]))
    cg.add(var.set_manual_move_debug_dump_frames(config[CONF_MANUAL_MOVE_DEBUG_DUMP_FRAMES]))
