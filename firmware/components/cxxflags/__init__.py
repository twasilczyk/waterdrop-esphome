from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv

CODEOWNERS = ["@twasilczyk"]
MULTI_CONF_NO_DEFAULT = True

CONFIG_SCHEMA = cv.ensure_list(cv.COMPONENT_SCHEMA)

async def to_code(config):
    include_dir = Path(__file__).resolve().parent
    cg.add_build_flag(f"-I{include_dir}")
