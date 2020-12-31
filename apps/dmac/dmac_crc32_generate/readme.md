---
parent: Harmony 3 peripheral library application examples for SAM D5X/E5X family
title: DMAC CRC-32 Generation
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# DMAC CRC-32 Generation

This example application demonstrates how to use the DMAC peripheral to compute 32-bit Cyclic Redundancy Checksum (CRC).

## Description

The DMAC unit provides support for calculating a CRC-16 and CRC-32 value for data passing through any DMA channel or using an IO interface.

This example application computes the 32-bit CRC value for the poynomial **(0x04C11DB7)** using below two data sources for the CRC engine in DMAC peripheral

1. It uses a DMA Channel with software trigger to initiate a memory-memory transfer from the source buffer to the destination buffer with 8-bit beat size. Once the transfer is completed a 32-bit CRC will be generated for the source buffer.

2. It uses the IO mode to generate a 32-bit CRC value for the source buffer.

Once the hardware CRC are generated using above two methods they are validated using a software-calculated CRC value.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/dmac/dmac_crc32_generate/firmware** .

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

### Setting up [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)

- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Open the Terminal application (Ex.:Tera term) on the computer
2. Connect to the EDBG Virtual COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application using its IDE
4. LED indicates the success or failure:
    - LED is turned ON when the DMAC memory transfer is successful and hardware CRC values matched with the software calculated CRC value
    - Following table provides the LED name

    | Board Name | LED Name |
    |------------| -------- |
    |[SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro) | LED0 |

5. Observe output message in console as follows:
    - Expected CRC Value for the polynomial **(0x04C11DB7)** is **(0xcbf43926)**

    ![output](images/output_dmac_crc32_generate.png)
