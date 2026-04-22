import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

CODEOWNERS = ["@twasilczyk"]
DEPENDENCIES = ["uart"]
MULTI_CONF_NO_DEFAULT = True

waterdrop_serial_ns = cg.esphome_ns.namespace("waterdrop_serial")
WaterdropSerial = waterdrop_serial_ns.class_("WaterdropSerial", cg.Component)

WATERDROP_SERIAL_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.ensure_list(cv.COMPONENT_SCHEMA)


def uart_final_validate_schema(name):
    return uart.final_validate_device_schema(
        name,
        uart_bus=CONF_UART_ID,
        baud_rate=9600,
        require_rx=True,
        require_tx=True,
        data_bits=8,
        parity="NONE",
        stop_bits=1,
    )


async def register_uart_component(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(parent))
    return var
