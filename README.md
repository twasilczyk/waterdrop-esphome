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

## Bill of materials

| Ref | Qty | Value     | Product          | Total cost [$] | Optional |
| --- | --- | --------- | ---------------- | -------------- | -------- |
| C1  | 1   | 100µF/50V | 50YXJ100M8X11.5  | 0.43 |
| C2  | 1   | 220µF/16V | 25YXJ220M6.3X11  | 0.31 |
| C3  | 1   | 22nF      | CL21B223KBANFNC  | 0.10 |
| C4  | 1   | 100nF     | CL21B104KBCNNNC  | 0.10 |
| D1  | 1   | SMAJ30A   | SMAJ30A          | 0.22 |
| D2  | 1   | B360A     | B360A-13-F       | 0.51 |
| D3  | 1   | 1N4148    | 1N4148W-7-F      | 0.17 | Yes (relay) |
| F1  | 1   | 500mA     | 0PTF0078P + 0217.500MXP | 1.35 <!-- 0.78 + 0.57 --> |
| J1  | 1   | DC5525 m  | PJ-002B          | 0.66 |
| J2  | 1   | DC5525 f  | 10-02950         | 3.17 | Yes (relay) |
| J3  | 1   |           | Waterdrop        | 0.00 |
| J4  | 1   | 5pin 2.54 | PH1RB-05-UA      | 0.15 |
| J5  | 1   | 2pin 2.54 | PH1RB-02-UA      | 0.10 | Yes (valve) |
| K1  | 1   | SPDT 24V  | G5LE-1-E DC24    | 2.74 | Yes (relay) |
| L1  | 1   | 33µH      | 12RS333C         | 1.35 |
| L2  | 1   | 1µH       | MLZ2012A1R0WT000 | 0.10 |
| Q1  | 1   | BC817     | BC817-40-7-F     | 0.23 | Yes (relay) |
| R1  | 1   | 3.6k      | 0805             | 0.10 |
| R2  | 1   | 1.2k      | 0805             | 0.10 |
| R3,R11 | 2 | 10k      | 0805             | 0.20 | R3 (relay) |
| R4  | 1   | 47k       | 0805             | 0.10 | Yes (relay) |
| R5,R6,R9,R13 | 4 | 100k | 0805           | 0.40 | R13 (valve) |
| R7,R10,R12 | 3 | 51k  | 0805             | 0.30 | R12 (valve) |
| R14 | 1   | 1Ω        | 0805             | 0.10 | Yes (valve) |
| U1  | 1   | LM2596DS-ADJ | LM2596DSADJG  | 2.34 |
| U2  | 1   | ESP32-S3-Tiny | [ESP32-S3-Tiny-Kit](https://docs.waveshare.com/ESP32-S3-Tiny?variant=ESP32-S3-Tiny-Kit) | 11.99 |
| U3  | 1   | 74AHCT125 | SN74AHCT125QDRQ1 | 0.70 |
| U4  | 1   | TB67H450A | TB67H450AFNG     | 0.85 | Yes (valve) |
|     | 34  |           |                 | 28.87 | 21.11 w/o |