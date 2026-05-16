from dataclasses import dataclass
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    binary_sensor,
    sensor,
    switch,
    text_sensor,
    waterdrop_serial,
)
from esphome.const import (
    CONF_NAME,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_RUNNING,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_RESTART,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
    UNIT_PERCENT,
)

AUTO_LOAD = [
    "binary_sensor",
    "cxxflags",
    "sensor",
    "switch",
    "text_sensor",
    "waterdrop_serial",
]
CODEOWNERS = ["@twasilczyk"]
DEPENDENCIES = waterdrop_serial.DEPENDENCIES

waterdrop_serial_ro_ns = waterdrop_serial.waterdrop_serial_ns.namespace("ro")
WaterdropSerialRo = waterdrop_serial_ro_ns.class_(
    "WaterdropSerialRo", waterdrop_serial.WaterdropSerial
)
DiagnosticSwitch = waterdrop_serial_ro_ns.class_("DiagnosticSwitch", switch.Switch)
waterdrop_serial_ro_filter_ns = waterdrop_serial_ro_ns.namespace("filter")
FilterType = waterdrop_serial_ro_filter_ns.enum("Type", is_class=True)
FilterSensors = waterdrop_serial_ro_filter_ns.struct("Sensors")

CONF_BOOTING = "booting"
CONF_ENTITIES = "entities"
CONF_FAUCET_OPEN = "faucet_open"
CONF_FLUSHING = "flushing"
CONF_HEALTH = "health"
CONF_REMAINING_LIFE = "remaining_life"
CONF_REMAINING_PERCENT = "remaining_percent"
CONF_TDS = "tds"
CONF_TOTAL_LIFE = "total_life"
CONF_UNEXPECTED_FRAME = "unexpected_frame"

ICON_FAUCET = "mdi:faucet"
ICON_RAW_BYTE = "mdi:hexadecimal"
ICON_WATER_QUALITY = "mdi:water-opacity"
ICON_WATER_OK = "mdi:water-check"
ICON_WATER_FILTER = "mdi:air-filter"
ICON_WATER_PUMP = "mdi:water-pump"


@dataclass(frozen=True)
class Filter:
    key: str
    name: str
    enum: object


@dataclass(frozen=True)
class ErrorType:
    key: str
    name: str


@dataclass(frozen=True)
class RawByteSensor:
    key: str
    name: str


FILTERS = (
    Filter("cf", "CF filter", FilterType.CF),
    Filter("ro", "RO filter", FilterType.RO),
    Filter("cb", "CB filter", FilterType.CB),
)

ERROR_SENSORS = (
    ErrorType("e01", "E01 error"),
    ErrorType("e02", "E02 error"),
    ErrorType("e03", "E03 error"),
    ErrorType("e04", "E04 error"),
)

RAW_BYTE_SENSORS = (
    RawByteSensor("c2_state", "C2 raw state"),
    RawByteSensor("c2_unknown", "C2 unknown"),
    RawByteSensor("c2_error", "C2 unknown error"),
    RawByteSensor("c5_01_unknown7", "C5 01 unknown 7"),
    RawByteSensor("c5_02_unknown4", "C5 02 unknown 4"),
    RawByteSensor("c5_02_unknown7", "C5 02 unknown 7"),
    RawByteSensor("c5_03_unknown2", "C5 03 unknown 2"),
    RawByteSensor("c5_03_unknown3", "C5 03 unknown 3"),
    RawByteSensor("c5_03_unknown4", "C5 03 unknown 4"),
    RawByteSensor("c5_03_unknown6", "C5 03 unknown 6"),
    RawByteSensor("c5_04_unknown1", "C5 04 unknown 1"),
    RawByteSensor("c5_04_unknown2", "C5 04 unknown 2"),
    RawByteSensor("c5_04_unknown6", "C5 04 unknown 6"),
    RawByteSensor("c5_04_unknown7", "C5 04 unknown 7"),
    RawByteSensor("c5_05_unknown3", "C5 05 unknown 3"),
    RawByteSensor("c5_05_unknown4", "C5 05 unknown 4"),
    RawByteSensor("22_01_unknown2", "22 01 unknown 2"),
    RawByteSensor("22_01_unknown4", "22 01 unknown 4"),
    RawByteSensor("22_01_unknown6", "22 01 unknown 6"),
    RawByteSensor("22_03_unknown6", "22 03 unknown 6"),
    RawByteSensor("22_0d_unknown2", "22 0D unknown 2"),
    RawByteSensor("22_0d_unknown4", "22 0D unknown 4"),
    RawByteSensor("22_0d_unknown6", "22 0D unknown 6"),
    RawByteSensor("22_0e_unknown1", "22 0E unknown 1"),
    RawByteSensor("22_0e_unknown2", "22 0E unknown 2"),
    RawByteSensor("22_0e_unknown3", "22 0E unknown 3"),
    RawByteSensor("22_0e_unknown4", "22 0E unknown 4"),
    RawByteSensor("22_0e_unknown5", "22 0E unknown 5"),
    RawByteSensor("22_0e_unknown6", "22 0E unknown 6"),
    RawByteSensor("22_0e_unknown7", "22 0E unknown 7"),
    RawByteSensor("22_0f_unknown1", "22 0F unknown 1"),
)


def filter_schema(filter_name: str):
    return cv.Schema(
        {
            cv.Optional(
                CONF_TOTAL_LIFE, default={CONF_NAME: f"{filter_name} total life"}
            ): sensor.sensor_schema(
                icon=ICON_WATER_FILTER,
                accuracy_decimals=0,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(
                CONF_REMAINING_LIFE,
                default={CONF_NAME: f"{filter_name} remaining life"},
            ): sensor.sensor_schema(
                icon=ICON_WATER_FILTER,
                accuracy_decimals=0,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(
                CONF_REMAINING_PERCENT,
                default={CONF_NAME: f"{filter_name} remaining"},
            ): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_WATER_FILTER,
                accuracy_decimals=2,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(
                CONF_HEALTH, default={CONF_NAME: f"{filter_name} health"}
            ): binary_sensor.binary_sensor_schema(
                icon=ICON_WATER_OK,
                device_class=DEVICE_CLASS_PROBLEM,
            ),
        }
    )


FILTER_CONFIG_SCHEMA = {
    cv.Optional(filter_.key, default={}): filter_schema(filter_.name)
    for filter_ in FILTERS
}

ERROR_CONFIG_SCHEMA = {
    cv.Optional(
        error.key, default={CONF_NAME: error.name}
    ): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_PROBLEM,
    )
    for error in ERROR_SENSORS
}

RAW_BYTE_CONFIG_SCHEMA = {
    cv.Optional(
        raw_byte.key, default={CONF_NAME: raw_byte.name}
    ): sensor.sensor_schema(
        icon=ICON_RAW_BYTE,
        accuracy_decimals=0,
        filters=[{"delta": 0}],
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )
    for raw_byte in RAW_BYTE_SENSORS
}

ENTITIES_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_TDS, default={CONF_NAME: "TDS"}): sensor.sensor_schema(
            unit_of_measurement=UNIT_PARTS_PER_MILLION,
            icon=ICON_WATER_QUALITY,
            accuracy_decimals=0,
            filters=[{"delta": 0}],
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(
            CONF_BOOTING, default={CONF_NAME: "Booting"}
        ): binary_sensor.binary_sensor_schema(
            icon=ICON_RESTART,
            device_class=DEVICE_CLASS_RUNNING,
        ),
        cv.Optional(
            CONF_FLUSHING, default={CONF_NAME: "Flushing"}
        ): binary_sensor.binary_sensor_schema(
            icon=ICON_WATER_PUMP,
            device_class=DEVICE_CLASS_RUNNING,
        ),
        **RAW_BYTE_CONFIG_SCHEMA,
        **ERROR_CONFIG_SCHEMA,
        **FILTER_CONFIG_SCHEMA,
        cv.Optional(
            CONF_UNEXPECTED_FRAME, default={CONF_NAME: "Unexpected frame"}
        ): text_sensor.text_sensor_schema(
            icon=ICON_RAW_BYTE,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(
            CONF_FAUCET_OPEN, default={CONF_NAME: "Faucet open"}
        ): switch.switch_schema(
            DiagnosticSwitch,
            icon=ICON_FAUCET,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            block_inverted=True,
        ),
    }
)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WaterdropSerialRo),
        cv.Optional(CONF_ENTITIES, default={}): ENTITIES_SCHEMA,
    }
).extend(waterdrop_serial.WATERDROP_SERIAL_SCHEMA)

FINAL_VALIDATE_SCHEMA = waterdrop_serial.uart_final_validate_schema("waterdrop_serial_ro")


async def to_code(config):
    var = await waterdrop_serial.register_uart_component(config)
    entities_config = config[CONF_ENTITIES]

    for filter_ in FILTERS:
        filter_config = entities_config[filter_.key]
        total_life = await sensor.new_sensor(filter_config[CONF_TOTAL_LIFE])
        remaining_life = await sensor.new_sensor(filter_config[CONF_REMAINING_LIFE])
        remaining_percent = await sensor.new_sensor(filter_config[CONF_REMAINING_PERCENT])
        health = await binary_sensor.new_binary_sensor(filter_config[CONF_HEALTH])
        cg.add(
            var.set_filter_sensors(
                filter_.enum,
                FilterSensors(total_life, remaining_life, remaining_percent, health),
            )
        )

    cg.add(var.set_tds_sensor(await sensor.new_sensor(entities_config[CONF_TDS])))
    booting = await binary_sensor.new_binary_sensor(entities_config[CONF_BOOTING])
    cg.add(var.set_booting_sensor(booting))
    flushing = await binary_sensor.new_binary_sensor(entities_config[CONF_FLUSHING])
    cg.add(var.set_flushing_sensor(flushing))

    error_sensors = [
        await binary_sensor.new_binary_sensor(entities_config[error.key])
        for error in ERROR_SENSORS
    ]
    cg.add(var.set_error_sensors(cg.ArrayInitializer(*error_sensors)))

    raw_byte_sensors = [
        await sensor.new_sensor(entities_config[raw_byte.key])
        for raw_byte in RAW_BYTE_SENSORS
    ]
    cg.add(var.set_raw_byte_sensors(cg.ArrayInitializer(*raw_byte_sensors)))

    unexpected_frame = await text_sensor.new_text_sensor(
        entities_config[CONF_UNEXPECTED_FRAME]
    )
    cg.add(var.set_unexpected_frame_sensor(unexpected_frame))

    faucet_state_switch = await switch.new_switch(entities_config[CONF_FAUCET_OPEN])
    cg.add(var.set_faucet_state_switch(faucet_state_switch))
