# Light Box

## Overview

A battery-powered 16x16 addressable LED panel with motion sensing and interactive controls. Features a gyroscope-controlled mazeball game, dynamic visual patterns, and intelligent battery management.

## Hardware

- ESP32 DevKit v1 (30-pin)
- WS2812B 16x16 LED Panel (256 pixels)
- 10000mAh 1S Li-ion Battery
- TPS61088 Boost Converter (5A)
- BQ25606 USB-C Charging Module
- Max17048 LiPo Fuel Gauge
- MPU6050 Gyroscope/Accelerometer
- 3x Tactile Push Buttons

The LED panel is controlled by the ESP32 which is powered by a 10000mAh Li-ion battery with USB-C charging. The TPS61088 boost converter steps up the battery voltage to 5V for the LED panel, while the Max17048 fuel gauge provides accurate battery monitoring. An MPU6050 gyroscope enables tilt-based controls for the interactive mazeball game and some patterns. Three buttons provide physical controls for pattern cycling, battery display, and brightness control. The entire system features auto-dimming based on battery level to extend runtime up to 8 hours. Housed in a custom designed and 3d printed case and light diffuser.

## Software

- Arduino/ESP32 Framework
- FastLED Library for WS2812B Control
- I2C Communication for Sensors
- FreeRTOS for Multi-Core Processing

The ESP32 runs a fully embedded system with no external connectivity required. The device features a gyroscope-controlled physics-based mazeball game with procedurally generated levels, a library of pre-built visual patterns (ambient lighting, geometric designs, motion-responsive effects), and intelligent power management with automatic brightness adjustment as the battery depletes. Three physical buttons provide complete control: pattern cycling, battery status display, and brightness adjustment.

## Gallery

<table>
  <tr>
    <td><img src="images/IMG_0962.png" width="200"/></td>
    <td><img src="images/IMG_0963.png" width="200"/></td>
    <td><img src="images/IMG_0964.png" width="200"/></td>
    <td><img src="images/IMG_0968.png" width="200"/></td>
  </tr>
  <tr>
    <td><img src="images/IMG_0965.png" width="200"/></td>
    <td><img src="images/IMG_0966.png" width="200"/></td>
    <td><img src="images/IMG_0967.png" width="200"/></td>
    <td><img src="images/IMG_0969.png" width="200"/></td>
  </tr>
  <tr>
    <td colspan="4" align="center"><img src="images/IMG_0971.gif" width="800"/></td>
  </tr>
</table>
