import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import waterdrop_serial

AUTO_LOAD = ["waterdrop_serial"]
CODEOWNERS = ["@twasilczyk"]
DEPENDENCIES = waterdrop_serial.DEPENDENCIES

waterdrop_serial_faucet_ns = waterdrop_serial.waterdrop_serial_ns.namespace("faucet")
WaterdropSerialFaucet = waterdrop_serial_faucet_ns.class_(
    "WaterdropSerialFaucet", waterdrop_serial.WaterdropSerial
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WaterdropSerialFaucet),
    }
).extend(waterdrop_serial.WATERDROP_SERIAL_SCHEMA)

FINAL_VALIDATE_SCHEMA = waterdrop_serial.uart_final_validate_schema("waterdrop_serial_faucet")


async def to_code(config):
    await waterdrop_serial.register_uart_component(config)
