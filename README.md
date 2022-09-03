# fenceless-robotic-lawnmower

[![GitHub version](https://img.shields.io/github/v/release/ldab/fenceless_robotic_lawnmower?include_prereleases)](https://github.com/ldab/fenceless_robotic_lawnmower/releases/latest)
![Build Status](https://github.com/ldab/fenceless_robotic_lawnmower/actions/workflows/workflow.yml/badge.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/ldab/fenceless_robotic_lawnmower/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/fenceless_robotic_lawnmower.svg?style=social)](https://github.com/ldab/fenceless_robotic_lawnmower)

## TODO
- [ ] Bridge AP and STA https://github.com/espressif/esp-gateway

## Apps

* Base
* Mower

## Concept

```mermaid
flowchart TD
    A[Start] --> B{Is it?}
    B -- Yes --> C[OK]
    C --> D[Rethink]
    D --> B
    B -- No ----> E[End]
```

### WiFi Remote Control

<img src="./images/wifi_control.jpg" width="30%"> 

## Mission Planner

Use [Mission Planner](Mission Planner: https://ardupilot.org/planner/#) in order to rout the area to be covered:

<img src="https://ardupilot.org/copter/_images/mp_auto_mission_grid.jpg" width="30%"> 


File format is:

```
QGC WPL <VERSION>\r\n
<INDEX>\t<CURRENT WP>\t<COORD FRAME>\t<COMMAND>\t<PARAM1>\t<PARAM2>\t<PARAM3>\t<PARAM4>\t<PARAM5/X/LATITUDE>\t<PARAM6/Y/LONGITUDE>\t<PARAM7/Z/ALTITUDE>\t<AUTOCONTINUE>\r\n
```

https://mavlink.io/en/file_formats/

## Scrapbook:

* Board definition: https://github.com/platformio/platform-espressif32/blob/master/boards/esp32-s3-devkitc-1.json
* Circle distance between two points: https://en.wikipedia.org/wiki/Haversine_formula

## Credits:

* Mars Rover: https://github.com/jakkra/Mars-Rover
* Badges by [shields.io](https://shields.io/)