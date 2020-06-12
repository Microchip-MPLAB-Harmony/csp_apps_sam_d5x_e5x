[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# DAC waveform generation with DMA

This example application shows how to use the DAC with the DMA to generate a 5 KHz sinusoidal waveform without CPU intervention.

## Description

The DAC Peripheral library is used in hardware trigger mode to generate a Sine wave. The TC peripheral is configured to generate a trigger every two microseconds. Trigger is connected to the DAC using the EVSYS peripheral. DMA is used to setup a linked list to transfer sine wave samples from lookup table to the DAC "DATABUF" register. In this application, the number of DAC samples per cycle(0 to 360 degrees sine wave) is 100.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_d5x_e5x) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/dac/dac_wav_gen_dma/firmware** .

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

- Connect an oscilloscope to monitor DAC pin PA02 (Pin 1 of the ADC/DAC Header)
- Connect the Debug USB port on the board to the computer using a micro USB cable

## Running the Application

1. Build and Program the application using its IDE
2. Observe a sine wave of 5 KHz frequency on DAC output pin
3. Refer to the below table for dac output pin details:

| Board      | DAC output pins |
| ----------------- |-----------|
| [SAM E54 Xplained Pro board](https://www.microchip.com/developmenttools/ProductDetails/atsame54-xpro)    | PA02 (Pin 1 of the ADC/DAC Header) |
|||
