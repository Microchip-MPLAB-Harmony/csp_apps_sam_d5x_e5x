[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# PORT polling

This example application demonstrate how to poll the switch input, and indicate the switch status using the LED.

## Description

This application uses the PORT Peripheral library to read the GPIO pin connected to the switch, and drives the GPIO pin connected to the LED to indicate the switch status. LED is turned ON when the switch is pressed and turned OFF when the switch is released.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/port/port_led_on_off_polling/firmware** .

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

1. Build and Program the application using its IDE
2. LED is turned on when the switch is pressed and turned off when the switch is released.

Refer the table below for details of switch and LED

| Board      | Switch Name | LED Name |
| ---------- | ---------| ------------|
| [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | SW0 | LED0 |
||||
