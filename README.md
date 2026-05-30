# waterdrop-esphome

Smarten Waterdrop RO systems: a plug-in Wi-Fi add-on board to integrate with Home Assistant, read hidden diagnostic parameters, and (potentially) work around refrigerator line limitations.

See the story: [Hacking a water filter through a 7-segment display](docs/story.md).

## Environment setup

1. Set up ESPHome through the [pip method](https://esphome.io/guides/installing_esphome/#pip)
2. `cd path/to/repo`
3. `ln -s "$HOME/path/to/your/esphome-venv" .venv`

## Building

1. Ctrl+Shift+P
2. `Tasks: Run Task`
3. `ESPHome: Run without debugging`

## Electronics

MCU board: [Waveshare ESP32-S3-Tiny](https://www.waveshare.com/wiki/ESP32-S3-Tiny).
