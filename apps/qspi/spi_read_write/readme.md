[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# QSPI flash read write in SPI mode

This example shows how to use the QSPI Peripheral library to perform erase, write and read operation with the QSPI Serial Flash memory in SPI mode.

## Description

This example uses the QSPI peripheral library to erase and write an array of values to the QSPI Serial Flash memory in SPI Mode. It verifies the value written by reading the values back and comparing it to the value written.

The application can be configured to perform operations on below QSPI Serial Flash Memories
- SST26VF064B (Default)
- N25Q256

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/qspi/spi_read_write/firmware** .

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

1. Open the application project using its respective IDE
2. Open **app.h** file and set the macro for the QSPI Flash memory present on the board to 1
    ```
    #define APP_USE_SST26_FLASH     1

    #define APP_USE_N25Q_FLASH      0
    ```

3. Build and Program the application project
4. Reset the device by power cycle
5. The LED indicates the success or failure:
    - The LED is turned ON when the value read from the QSPI Serial Flash memory matches with the written value

The following table provides the LED name

| Board      | LED Name |
| ---------- | -------- |
| [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | LED0 |
