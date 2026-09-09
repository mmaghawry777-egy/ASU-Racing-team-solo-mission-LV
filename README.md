# ASU Racing Team – Low Voltage Solo Mission

This repository contains my work for the ASU Racing Team Low Voltage Individual Technical Mission.

The project covers STM32 embedded programming, Proteus simulation, SPI communication, and PCB design using Altium Designer.

## Repository Structure

```text
ASU-Racing-team-solo-mission-LV/
├── STM32/
│   ├── Milestone 1/
│   ├── Milestone 2/
│   └── Milestone 3/
├── Proteus/
├── PCB/
├── Videos/
└── README.md
```

## Milestone 1 – GPIO LED Blink

### Objective
Use an STM32F103C8 microcontroller to toggle an LED connected to PC13.

### Implementation
- STM32F103C8 used as the target MCU.
- PC13 configured as a GPIO output.
- LED state is toggled inside the main loop.
- A 500 ms delay is used between toggles.
- The circuit was tested in Proteus.

### Main HAL Functions
| Function | Description | Documentation |
|---|---|---|
| `HAL_Init()` | Initializes the STM32 HAL library and system time base. | UM1850, HAL/user application section |
| `HAL_GPIO_Init()` | Configures the GPIO pin and its operating mode. | UM1850, Section 20.2, GPIO API, p. 225 |
| `HAL_GPIO_WritePin()` | Sets or resets a GPIO output pin. | UM1850, Section 20.2.4/20.2.5, pp. 226+ |
| `HAL_GPIO_TogglePin()` | Toggles the current state of a GPIO output pin. | UM1850, Section 20.2.4/20.2.5, pp. 226+ |
| `HAL_Delay()` | Provides a blocking delay using the HAL time base/SysTick. | UM1850, HAL/user application section |

## Milestone 2 – ADC and UART

### Objective
Read two analog signals using ADC1 and send the processed values to a Virtual Terminal through USART1.

### Implementation
- STM32F103C8 used as the target MCU.
- ADC1 configured with two channels.
- Example inputs are PA0 and PA1.
- Continuous conversion and DMA are not used.
- USART1 configured for serial communication.
- ADC readings are converted from the 12-bit range (0–4095) into the required application values.
- Results are formatted as text and transmitted to the Virtual Terminal.
- The system was tested in Proteus.

### Main HAL Functions
| Function | Description | Documentation |
|---|---|---|
| `HAL_ADC_Init()` | Initializes the ADC peripheral using the selected configuration. | UM1850, Section 7.2, ADC API, pp. 58–61 |
| `HAL_ADC_ConfigChannel()` | Configures an ADC channel and its rank in the conversion sequence. | UM1850, Section 7.2, p. 58 |
| `HAL_ADC_Start()` | Starts a regular ADC conversion. | UM1850, Section 7.2.4, p. 59 |
| `HAL_ADC_PollForConversion()` | Waits until the ADC conversion is completed. | UM1850, Section 7.2.4, p. 59 |
| `HAL_ADC_GetValue()` | Retrieves the result of the ADC conversion. | UM1850, Section 7.2.4, p. 59 |
| `HAL_UART_Init()` | Initializes UART/USART communication according to the configured parameters. | UM1850, Section 38.2, pp. 548–549 |
| `HAL_UART_Transmit()` | Sends a specified amount of data through UART in blocking mode. | UM1850, Section 38.2.7, p. 555 |
| `HAL_UART_Receive()` | Receives a specified amount of data through UART in blocking mode. | UM1850, Section 38.2.7, p. 555 |

## Milestone 3 – SPI Master/Slave Communication

### Objective
Use two STM32F103C8 microcontrollers to communicate through SPI. The Master sends a Case ID to the Slave, and the Slave returns the corresponding voltage and wheel-speed data.

### Master
- STM32F103C8 configured as SPI1 Master.
- PB0 used as the chip-select (CS) signal.
- USART1 used for communication with the Virtual Terminal.
- The user enters a Case ID from 1 to 5.
- The Master sends the selected Case ID to the Slave.
- The Master handles the required two-cycle SPI synchronization.
- The received 24-bit message is decoded and displayed.

### Slave
- STM32F103C8 configured as SPI1 Slave.
- Receives the Case ID from the Master.
- Stores five predefined voltage/speed cases.
- Encodes the selected values into a 24-bit message.
- Sends the message back to the Master.

### Predefined Cases

| Case ID | Voltage | Wheel Speed |
|---|---:|---:|
| `0x1001` | 12.3 V | 200 |
| `0x1002` | 14.2 V | 150 |
| `0x1003` | 13.8 V | 100 |
| `0x1004` | 11.5 V | 220 |
| `0x1005` | 10.9 V | 280 |

### Main HAL Functions
| Function | Description | Documentation |
|---|---|---|
| `HAL_SPI_Init()` | Initializes SPI using the selected mode, data size, clock settings, NSS management, and other parameters. | UM1850, Section 35.2, pp. 446–448 |
| `HAL_SPI_Transmit()` | Sends data through the SPI peripheral. | UM1850, Section 35.2.3, pp. 448+ |
| `HAL_SPI_Receive()` | Receives data through the SPI peripheral. | UM1850, Section 35.2.3, pp. 448+ |
| `HAL_SPI_TransmitReceive()` | Performs simultaneous SPI transmission and reception. | UM1850, Section 35.2.3, pp. 448+ |
| `HAL_GPIO_WritePin()` | Controls the Master CS/NSS GPIO pin. | UM1850, GPIO API, Section 20.2 |
| `HAL_UART_Init()` | Initializes the UART used for the Virtual Terminal. | UM1850, Section 38.2, pp. 548–549 |
| `HAL_UART_Transmit()` | Sends the decoded values to the Virtual Terminal. | UM1850, Section 38.2.7, p. 555 |
| `HAL_UART_Receive()` | Receives the Case ID entered through the terminal. | UM1850, Section 38.2.7, p. 555 |

## Proteus Simulation

The Proteus projects are included in the `Proteus/` directory.

### Milestone 1
The STM32 PC13 output drives an LED through a resistor. The LED is expected to blink with a 500 ms interval.

### Milestone 2
Two potentiometers provide analog inputs to ADC1. The measured values are displayed on a Virtual Terminal through USART1.

### Milestone 3
Two STM32 Blue Pill devices communicate through SPI. The Master receives a Case ID from the Virtual Terminal and displays the corresponding voltage and wheel-speed values returned by the Slave.

## PCB – Altium Designer

The `PCB/` directory contains the Altium Designer project, schematic/PCB documents, libraries used by the design, and generated manufacturing files.

The PCB design was created using Altium Designer and includes the required schematic, PCB layout, footprints, and manufacturing outputs.

Generated manufacturing files include:
- Gerber files
- NC Drill files
- Board outline / routing information
- Plated and non-plated drill information
- CAMtastic/manufacturing output files

## Demonstration Videos

- Milestone 1 – GPIO LED Blink: [Add video link here]
- Milestone 2 – ADC + UART: [Add video link here]
- Milestone 3 – SPI Master/Slave: [Add video link here]

## HAL Documentation

The HAL functions used in this project are documented in the official STMicroelectronics user manual:

**UM1850 – Description of STM32F1 HAL and low-layer drivers, Rev. 3**

The relevant sections used in this project are:

- GPIO: Section 20.2 – GPIO Firmware driver API description, printed page 225
- ADC: Section 7.2 – ADC Firmware driver API description, printed pages 58–61
- SPI: Section 35.2 – SPI Firmware driver API description, printed pages 446–450
- UART: Section 38.2 – UART Firmware driver API description, printed pages 548–555

Official STMicroelectronics documentation:
https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf

## Tools Used

- STM32CubeIDE
- STM32F103C8T6
- STM32 HAL drivers
- Proteus
- Altium Designer
- Git / GitHub

## Notes

The STM32 projects contain the source code and project configuration files. The Proteus directory contains the simulation projects and required simulation files. The PCB directory contains the Altium project and manufacturing outputs.
