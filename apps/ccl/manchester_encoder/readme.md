---
parent: Harmony 3 peripheral library application examples for SAM D5X/E5X family
title: CCL Manchester Encoder 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# CCL Manchester Encoder

This example application shows how to use the CCL peripheral library and generate a Manchester-encoded output.

## Description

This demonstrates a way to generate a Manchester-encoded output using a SPI port and the CCL. The SPI port is sending out a predefined buffer of data in a circular fashion. Data is sent out LSB first, with CCL_OUT being the Manchester-encoded output. Pins are configured such that a logic analyzer can be attached to see the input (MOSI and SCK) and the output (CCL_OUT) simultaneously.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/ccl/manchester_encoder/firmware** .

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

### Setting up  [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)

- Use jumper from PA05 (EXT1 pin 13) to PB01 (EXT2 pin 5) (This routes SCK signal to CCL_IN[2])
- PA04 (EXT1 pin 14) has MOSI output
- PB02 (EXT2 pin 10) has CCL output (CCL_OUT)
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Connect a logic analyzer to MOSI pin
2. Connect a logic analyzer to SCK pin
3. Connect a logic analyzer to the Manchester-encoded output CCL_OUT pin
4. Refer to the following table for pin details:

    |Board| MOSI pin | SCK pin  | CCL_OUT pin |
    |-----|----------|----------|-------------|
    |[SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)| PA04 (EXT1 pin 14) | PA05 (EXT1 pin 13) | PB02 (EXT2 pin 10) |
    ||||

5. Build and Program the application using its IDE
6. Observe the output on logic analyzer, it should follow the truth table as shown in the following diagram:

    ![output](images/truth_table_manchester_encoder.png)
