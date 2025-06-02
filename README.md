# Secure LoRa(Long Range) Communication System

A secure and reliable communication system using LoRa modules, equipped with encryption and message retry mechanisms.

## Features

### Security
- AES-128 encryption in CBC mode.
- Message signing for integrity verification.
- Device authentication.
- Shared key encryption.
- Message signature verification.

### Reliability
- Automatic message retry mechanism (3 attempts).
- Message acknowledgment system.
- Error detection and reporting.
- Signal strength monitoring (RSSI).

### Message Types
- Data messages (Type 0).
- Acknowledgments (Type 1).
- Authentication messages (Type 2).

## Hardware Requirements

### Components
- 2x Arduino-compatible boards.
- 2x RFM95 LoRa modules.
- Jumper wires.
- USB cables for programming.

### Pin Configuration
RFM95 Module -> Arduino
CS  -> Pin 8
RST -> Pin 4
INT -> Pin 7
LED -> Pin 13 (on receiver)

## Software Requirements

### Libraries
- RadioHead (RH_RF95).
- SPI.
- AES.
- CBC.

## Setup Instructions

1. Hardware Setup
   - Connect the RFM95 modules to the Arduino boards according to the pin configuration.
   - Ensure proper power supply to both devices.

2. Software Setup
   - Install the required libraries.
   - Upload 'LoRa sender.ino' to one Arduino.
   - Upload 'LoRa receiver.ino' to the other Arduino.
   - Set the same frequency on both devices (default: 915.0 MHz).

3. Security Configuration
   - Assign a unique DEVICE_ID to each device.
   - Set the same SHARED_KEY on both devices (16 bytes for AES-128).
   - Keep the SHARED_KEY secure and private.

## Usage

1. Initialization
   - Power on both devices.
   - Sender will attempt authentication automatically.
   - Receiver will reply with an authentication acknowledgment.

2. Sending Messages
   - Open the Serial Monitor on the sender (115200 baud).
   - Type your message and press Enter.
   - The system will:
     - Encrypt the message.
     - Add a signature.
     - Send the message with the retry mechanism.
     - Await acknowledgment.

3. Receiving Messages
   - Messages are automatically:
     - Verified for integrity.
     - Decrypted.
     - Acknowledged.
   - Received messages are displayed on the Serial Monitor.

## Configuration Options

### Security Settings
#define SHARED_KEY "YourSecretKey123" // 16 bytes for AES-128
#define DEVICE_ID "SENDER1"           // Unique device identifier

### Communication Settings
#define RF95_FREQ 915.0               // LoRa frequency
#define MAX_MESSAGE_SIZE 128          // Maximum message size

### Retry Settings
#define MAX_RETRIES 3                 // Maximum retry attempts
#define RETRY_DELAY 1000              // Delay between retries (ms)

## Message Structure

struct SecureMessage {
    char deviceId[8];        // Sender's device ID
    uint32_t timestamp;      // Message timestamp
    uint8_t messageType;     // Type of message
    uint8_t messageId;       // Unique message ID
    uint8_t retryCount;      // Number of retries attempted
    char payload[128];       // Actual message
    uint32_t signature;      // Message signature
};

## Troubleshooting

1. No Communication
   - Ensure frequency settings match.
   - Verify all hardware connections.
   - Confirm power supply is adequate.
   - Monitor Serial Monitor for any error messages.

2. Message Failures
   - Check signal strength (RSSI).
   - Verify encryption keys match.
   - Ensure message size is within the limit.
   - Monitor retry attempts.

3. Security Issues
   - Ensure shared keys match.
   - Confirm device IDs are unique.
   - Monitor signature verification results.

## Security Notes

1. Key Management
   - Change the default SHARED_KEY.
   - Keep all keys secure and private.
   - Consider implementing periodic key rotation.

2. Limitations
   - Current version uses key as IV.
   - No support for message expiration.
   - Basic signature mechanism.

## Future Improvements

1. Security
   - Implement secure key exchange.
   - Add message expiration support.
   - Improve IV generation.
   - Introduce sequence numbers.

2. Reliability
   - Introduce message queuing.
   - Implement power management.
   - Add detailed signal quality monitoring.

3. Features
   - Add configuration via Serial.
   - Implement device sleep modes.
   - Enable message logging.
   - Add system status reporting.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
