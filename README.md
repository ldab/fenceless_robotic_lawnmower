# fenceless-robotic-lawnmower

[![GitHub version](https://img.shields.io/github/v/release/ldab/fenceless_robotic_lawnmower?include_prereleases)](https://github.com/ldab/fenceless_robotic_lawnmower/releases/latest)
![Build Status](https://github.com/ldab/fenceless_robotic_lawnmower/actions/workflows/workflow.yml/badge.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/ldab/fenceless_robotic_lawnmower/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/fenceless_robotic_lawnmower.svg?style=social)](https://github.com/ldab/fenceless_robotic_lawnmower)

<img src="./images/mower.gif" width="50%"> 

## TODO
- [ ] Upload mower firmware over gateway webportal;
- [ ] Change gateway IP range so WiFi controller is accesible over the "home" network;

## Apps

* ~~Base~~ Now replaced by https://github.com/ldab/esp-gateway-long-range
* Mower

## Concept

### SPARTN and High Precision Position

```mermaid
flowchart LR
    A(Point Perfect) --  SPARTN --> B(Thingstream Broker)
    B <-- MQTT --> C(nora-w1)
    C -- SPARTN --> D(ZED-F9P)
    C -- keys --> D(ZED-F9P)
    D -- Position --> C
```

## WiFi Remote Control

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

## Hardware

### Wheel motor

#### Speed

Commercial units goes 16-18 m/min (), and has 205mm diameter $speed / (π * D) = ~RPM$ therefore we need something between 24 and 28 RPM.

#### Torque

You can go crazy about the numbers [Required Power and Torque](https://www.engineeringtoolbox.com/cars-power-torque-d_1784.html), but I go like this:

$$Fr = 15Kg * 9.81m/s2 * 0.2 (N)$$
$$a = 18 / 60 (m/s2)$$
$$Fa = 15kg * a (N)$$
$$Tr = Fr * 0.1m (Nm)$$
$$Ta = Fa * 0.1m (Nm)$$
$$Tt = Ta + Tr$$

We have 2x motors so `~7.5Nm` should be alright.

### Blade motor

Blade seems to run at 3500RPM

## Scrapbook:

* Board definition: https://github.com/platformio/platform-espressif32/blob/master/boards/esp32-s3-devkitc-1.json
* Circle distance between two points: https://en.wikipedia.org/wiki/Haversine_formula

## Credits:

* Mars Rover: https://github.com/jakkra/Mars-Rover
* Badges by [shields.io](https://shields.io/)