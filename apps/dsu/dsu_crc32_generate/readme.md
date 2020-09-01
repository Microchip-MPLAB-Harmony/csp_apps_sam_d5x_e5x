---
parent: Harmony 3 peripheral library application examples for SAM D5X/E5X family
title: DSU compute CRC32 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# DSU compute CRC32

This example application demonstrates how to use the DSU Peripheral library to compute 32-bit Cyclic Redundancy Checksum (CRC).

## Description

The DSU unit provides support for calculating a 32-bit CRC value for a memory area. This example application computes the 32-bit hardware CRC value using the DSU Peripheral and validates it using a software-calculated CRC value.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/dsu/dsu_crc32_generate/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_e54_xpro.X    | MPLABX Project for [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)|
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_e54_xpro.X    | [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)|
|||

### [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)

- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the application using its IDE
2. The LED indicates the success or failure:
    - LED is turned ON when the hardware CRC value matched with the software calculated CRC value
    - LED is turned OFF when the hardware CRC value did not match with the software calculated CRC value

The following table provides the LED name

| Board      | LED Name |
| ---------- | ---------|
| [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | LED0 |
||||
