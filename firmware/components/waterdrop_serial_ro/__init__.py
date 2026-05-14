from dataclasses import dataclass
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor, waterdrop_serial
from esphome.const import (
    CONF_NAME,
    CONF_SENSORS,
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
    UNIT_PERCENT,
)

AUTO_LOAD = ["binary_sensor", "cxxflags", "sensor", "waterdrop_serial"]
CODEOWNERS = ["@twasilczyk"]
DEPENDENCIES = waterdrop_serial.DEPENDENCIES

waterdrop_serial_ro_ns = waterdrop_serial.waterdrop_serial_ns.namespace("ro")
WaterdropSerialRo = waterdrop_serial_ro_ns.class_(
    "WaterdropSerialRo", waterdrop_serial.WaterdropSerial
)
waterdrop_serial_ro_filter_ns = waterdrop_serial_ro_ns.namespace("filter")
FilterType = waterdrop_serial_ro_filter_ns.enum("Type", is_class=True)
FilterSensors = waterdrop_serial_ro_filter_ns.struct("Sensors")

ICON_WATER_QUALITY = "mdi:water-opacity"
ICON_WATER_OK = "mdi:water-check"
ICON_WATER_FILTER = "mdi:air-filter"


@dataclass(frozen=True)
class Filter:
    key: str
    name: str
    enum: object


FILTERS = (
    Filter("cf", "CF filter", FilterType.CF),
    Filter("ro", "RO filter", FilterType.RO),
    Filter("cb", "CB filter", FilterType.CB),
)


def filter_schema(filter_name: str):
    return cv.Schema(
        {
            cv.Optional("total_life", default={CONF_NAME: f"{filter_name} total life"}): sensor.sensor_schema(
                icon=ICON_WATER_FILTER,
                accuracy_decimals=0,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional("remaining_life", default={CONF_NAME: f"{filter_name} remaining life"}): sensor.sensor_schema(
                icon=ICON_WATER_FILTER,
                accuracy_decimals=0,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional("remaining_percent", default={CONF_NAME: f"{filter_name} remaining"}): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_WATER_FILTER,
                accuracy_decimals=2,
                filters=[{"delta": 0}],
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("health", default={CONF_NAME: f"{filter_name} health"}): binary_sensor.binary_sensor_schema(
                icon=ICON_WATER_OK,
                device_class=DEVICE_CLASS_PROBLEM,
            ),
        }
    )


FILTER_CONFIG_SCHEMA = {}
for filter_ in FILTERS:
    FILTER_CONFIG_SCHEMA[cv.Optional(filter_.key, default={})] = filter_schema(filter_.name)

SENSORS_SCHEMA = cv.Schema(
    {
        cv.Optional("tds", default={CONF_NAME: "TDS"}): sensor.sensor_schema(
            unit_of_measurement=UNIT_PARTS_PER_MILLION,
            icon=ICON_WATER_QUALITY,
            accuracy_decimals=0,
            filters=[{"delta": 0}],
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        **FILTER_CONFIG_SCHEMA,
    }
)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WaterdropSerialRo),
        cv.Optional(CONF_SENSORS, default={}): SENSORS_SCHEMA,
    }
).extend(waterdrop_serial.WATERDROP_SERIAL_SCHEMA)

FINAL_VALIDATE_SCHEMA = waterdrop_serial.uart_final_validate_schema("waterdrop_serial_ro")


async def to_code(config):
    var = await waterdrop_serial.register_uart_component(config)
    sensors_config = config[CONF_SENSORS]

    for filter_ in FILTERS:
        filter_config = sensors_config[filter_.key]
        total_life = await sensor.new_sensor(filter_config["total_life"])
        remaining_life = await sensor.new_sensor(filter_config["remaining_life"])
        remaining_percent = await sensor.new_sensor(filter_config["remaining_percent"])
        health = await binary_sensor.new_binary_sensor(filter_config["health"])
        cg.add(
            var.set_filter_sensors(
                filter_.enum,
                FilterSensors(total_life, remaining_life, remaining_percent, health),
            )
        )

    cg.add(var.set_tds_sensor(await sensor.new_sensor(sensors_config["tds"])))
