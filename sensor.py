import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import uart, sensor
from esphome.const import CONF_ID, CONF_VARIANT

DEPENDENCIES = ["uart"]

standing_desk_height_ns = cg.esphome_ns.namespace("standing_desk_height")

StandingDeskHeightSensor = standing_desk_height_ns.class_("StandingDeskHeightSensor", cg.PollingComponent, uart.UARTDevice, sensor.Sensor)

# StandingDeskStatusSensor = standing_desk_height_ns.class_("StandingDeskStatusSensor", cg.PollingComponent, uart.UARTDevice,sensor.Sensor)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        unit_of_measurement="cm",
        icon="mdi:human-male-height-variant",
        accuracy_decimals=1
    )
    .extend({
        cv.GenerateID(): cv.declare_id(StandingDeskHeightSensor),
    })
    .extend(cv.polling_component_schema("500ms"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    if CONF_VARIANT in config:
        cg.add(var.set_decoder_variant(config[CONF_VARIANT]))

    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    yield uart.register_uart_device(var, config)