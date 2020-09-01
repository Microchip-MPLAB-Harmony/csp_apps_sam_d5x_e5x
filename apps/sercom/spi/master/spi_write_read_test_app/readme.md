---
parent: Harmony 3 peripheral library application examples for SAM D5X/E5X family
title: SERCOM SPI Master read write test application 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SERCOM SPI Master read write test application

This is a SPI Master test application which is provided to demonstrate communication between SPI master and the corresponding SPI slave application available under -  apps/sercom/spi/slave/spi_write_read/firmware 

## Description

This example uses the SERCOM SPI peripheral library in master mode and serves as a test application to demonstrate communication between SPI master and corresponding SPI slave application available at apps/sercom/spi/slave/spi_write_read/firmware.

The SPI master writes data by sending a write command followed by two bytes of memory address followed by the data to be written.

< WR_CMD > < ADDR_MSB > < ADDR_LSB > < DATA0 > ... < DATA n >

The SPI slave asserts the Busy line to indicate to the SPI master that it is busy. Once ready, the SPI slave de-asserts the Busy line. Once the SPI slave is ready, the SPI master reads the data by sending read command followed by two bytes of memory address and the number of bytes to read.

< RD_CMD > < ADDR_MSB > < ADDR_LSB > < NUM_BYTES >

The SPI slave responds by sending the data at the requested memory address. The SPI master compares the sent data with the received data and repeats the test if it matches.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and click

![clone](../../../../docs/images/clone.png)

Path of the application within the repository is **apps/sercom/spi/master/spi_write_read_test_app/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_e54_xpro.X    | MPLABX Project for [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)|

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_e54_xpro.X | [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)|
|||

### Setting up [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)

- To run this demo two [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro) are required. One will be progammed as SPI master and other will be programmed as SPI slave.
- Connect Pin 15, 16, 17, 18 on EXT2 of SPI slave to Pin 15, 16, 17, 18 on EXT2 of SPI master 
    - Pin 15 - SPI Chip Select
    - Pin 16 - MOSI
    - Pin 17 - MISO
    - Pin 18 - SCK
- Connect Pin 9 on EXT2 of SPI slave to Pin 9 on EXT2 of SPI master. Pin 9 is configured as GPIO and serves as the slave busy pin.
- Connect the Debug USB port on the SPI master and slave boards to the computer using a micro USB cable 

## Running the Application

1. Build and Program the SPI master application project using its IDE
2. Build and Program the SPI slave application project using its IDE. Path of the SPI Slave application within the repository is **apps/sercom/spi/slave/spi_write_read/firmware** 
3. On the SPI master board, press switch to start reading and writing data
2. LED on SPI master board indicates the success or failure:
    - LED is turned ON when the data read from the SPI slave matches with the data written
    - LED is turned OFF when the data read from the SPI slave does not match with the data written

Following table provides the LED name:

| Board      | Switch Name | LED Name |
| ---------- | ---------| ------------|
| [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | SW0 | LED0 |
||||
