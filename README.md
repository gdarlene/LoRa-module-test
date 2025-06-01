# LoRa Module Test Project

This project implements a two-way communication system using LoRa (Long Range communication protocol) radio modules with Arduino. It consists of two main components: a sender and a receiver, enabling long-range wireless communication.

## Project Overview

The system allows for bidirectional communication between two Arduino devices equipped with LoRa modules. Features include:

- Real-time message transmission and reception
- Signal strength (RSSI) monitoring
- Configurable frequency and transmission power
- Serial monitor interface for sending and receiving messages
- LED indication for message reception (on receiver)

## Hardware Requirements

- 2× Arduino boards (any Arduino board with SPI support)
- 2× LoRa modules (RFM95/96/97/98)
- LED (for receiver unit)
- Connecting wires
- USB cables for programming and serial communication

## Pin Connections

Connect the LoRa module to the Arduino using the following pin configuration:

| LoRa Module | Arduino Pin |
|-------------|-------------|
| CS          | 8          |
| RST         | 4          |
| INT         | 7          |
| SCK         | SCK        |
| MOSI        | MOSI       |
| MISO        | MISO       |
| VCC         | 3.3V       |
| GND         | GND        |

## Software Dependencies

This project requires the following Arduino libraries:
- SPI.h (built into Arduino IDE)
- RadioHead (RH_RF95.h)

To install the RadioHead library:
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "RadioHead"
4. Click Install

## Installation and Setup

1. Clone this repository or download the source files
2. Open Arduino IDE
3. Install the required libraries
4. Connect your Arduino boards according to the pin configuration
5. Upload `LoRa sender.ino` to the first Arduino
6. Upload `LoRa receiver.ino` to the second Arduino
7. Open the Serial Monitor (115200 baud) for both devices

## Usage

1. Power up both Arduino devices
2. Open Serial Monitor for both devices (baud rate: 115200)
3. On the sender device:
   - Type a message in the Serial Monitor and press send
   - The message will be transmitted to the receiver
4. On the receiver device:
   - Received messages will be displayed in the Serial Monitor
   - The onboard LED will blink upon message reception
   - RSSI (signal strength) will be displayed for each message

## Configuration

The default configuration uses:
- Frequency: 915.0 MHz (configurable in code)
- Transmission Power: 23 dBm
- Baud Rate: 115200

To modify the frequency or other parameters, edit the corresponding values in both sketch files.

## Troubleshooting

- Ensure both modules are set to the same frequency
- Check all wire connections
- Verify the correct libraries are installed
- Make sure the Serial Monitor baud rate matches the code (115200)
- Keep the modules at least a few feet apart during testing

## License

This project is open-source and available under the MIT License. 
