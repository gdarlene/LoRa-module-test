# Secure LoRa Communication System

A secure, reliable communication system using LoRa modules with encryption and message retry capabilities.

## Features

### Security
- AES-128 encryption in CBC mode
- Message signing for integrity verification
- Device authentication
- Shared key encryption
- Message signature verification

### Reliability
- Automatic message retry mechanism (3 attempts)
- Message acknowledgment system
- Error detection and reporting
- Signal strength monitoring (RSSI)

### Message Types
- Data messages (Type 0)
- Acknowledgments (Type 1)
- Authentication messages (Type 2)

## Hardware Requirements

### Components
- 2x Arduino-compatible boards
- 2x RFM95 LoRa modules
- Jumper wires
- USB cables for programming

### Pin Configuration
```
RFM95 Module -> Arduino
CS  -> Pin 8
RST -> Pin 4
INT -> Pin 7
LED -> Pin 13 (on receiver)
```

## Software Requirements

### Libraries
- RadioHead (RH_RF95)
- SPI
- AES
- CBC

## Setup Instructions

1. **Hardware Setup**
   - Connect the RFM95 modules to the Arduino boards as per pin configuration
   - Ensure proper power supply to both devices

2. **Software Setup**
   - Install required libraries
   - Upload `LoRa sender.ino` to one Arduino
   - Upload `LoRa receiver.ino` to the other Arduino
   - Set the same frequency on both devices (default: 915.0 MHz)

3. **Security Configuration**
   - Set a unique `DEVICE_ID` for each device
   - Set the same `SHARED_KEY` on both devices (16 bytes for AES-128)
   - Keep the `SHARED_KEY` secure and private

## Usage

1. **Initialization**
   - Power on both devices
   - Sender will automatically attempt authentication
   - Receiver will respond with authentication acknowledgment

2. **Sending Messages**
   - Open Serial Monitor on sender (115200 baud)
   - Type message and press enter
   - System will automatically:
     - Encrypt the message
     - Add signature
     - Send with retry mechanism
     - Wait for acknowledgment

3. **Receiving Messages**
   - Messages are automatically:
     - Verified for integrity
     - Decrypted
     - Acknowledged
   - Received messages are displayed on Serial Monitor

## Configuration Options

### Security Settings
```cpp
#define SHARED_KEY "YourSecretKey123" // 16 bytes for AES-128
#define DEVICE_ID "SENDER1"           // Unique device identifier
```

### Communication Settings
```cpp
#define RF95_FREQ 915.0              // LoRa frequency
#define MAX_MESSAGE_SIZE 128         // Maximum message size
```

### Retry Settings
```cpp
#define MAX_RETRIES 3               // Maximum retry attempts
#define RETRY_DELAY 1000           // Delay between retries (ms)
```

## Message Structure

```cpp
struct SecureMessage {
    char deviceId[8];        // Sender's device ID
    uint32_t timestamp;      // Message timestamp
    uint8_t messageType;     // Type of message
    uint8_t messageId;       // Unique message ID
    uint8_t retryCount;      // Number of retries attempted
    char payload[128];       // Actual message
    uint32_t signature;      // Message signature
};
```

## Troubleshooting

1. **No Communication**
   - Check frequency settings match
   - Verify hardware connections
   - Check power supply
   - Monitor Serial Monitor for error messages

2. **Message Failures**
   - Check signal strength (RSSI)
   - Verify encryption keys match
   - Check message size limits
   - Monitor retry attempts

3. **Security Issues**
   - Verify shared keys match
   - Check device IDs are unique
   - Monitor signature verification messages

## Security Notes

1. **Key Management**
   - Change the default `SHARED_KEY`
   - Keep keys secure
   - Consider implementing key rotation

2. **Limitations**
   - Current implementation uses key as IV
   - No message expiration
   - Basic signature system

## Future Improvements

1. **Security**
   - Implement secure key exchange
   - Add message expiration
   - Improve IV generation
   - Add sequence numbers

2. **Reliability**
   - Add message queuing
   - Implement power management
   - Add signal quality monitoring

3. **Features**
   - Add configuration via serial
   - Implement sleep modes
   - Add message logging
   - Add status reporting

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 
