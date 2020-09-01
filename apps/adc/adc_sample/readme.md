---
parent: Harmony 3 peripheral library application examples for SAM D5X/E5X family
title: ADC Sample 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# ADC Sample

This example application shows how to sample an analog input using the ADC and displays the converted samples on a serial terminal.

## Description

In this application, DAC output is fed to an analog input for the ADC conversion. An analog input is converted by a software trigger and the converted value is displayed on the console.DAC output is changed by 0.1 V upon switch press.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and then click **Clone** button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/adc/adc_sample/firmware** .

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

- Analog input (AIN1 - PA03) of the ADC is fed from the DAC0 output(VOUT[0] - PA02)
  - PA03 is routed to pin 4 of the EXT2 header
  - PA02 is routed to pin 1 of the ADC/DAC Header
- Use a jumper wire (male to female) to connect pin 1 of ADC/DAC Header to pin 4 of EXT2
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
4. Press switch to change the DAC value
5. ADC input voltage changes by 0.1 V when the switch is pressed, from 1.2 V to 2.4 V
6. Refer to the table below for the switch details:

    | Board| Switch name |
    |:---------|:---------:|
    | [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | SW0|
    |||

7. Console displays the ADC count and the ADC input voltage in the console as shown below:

    ![output](images/output_adc_sample.png)
