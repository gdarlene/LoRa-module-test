#include <SPI.h>
#include <RH_RF95.h>
#include <AES.h>
#include <CBC.h>

// Pin definitions
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// LoRa frequency
#define RF95_FREQ 950.0

// Security settings
#define SHARED_KEY "YourSecretKey123" // 16 bytes for AES-128
#define DEVICE_ID "SENDER1"           // Unique device identifier
#define MAX_MESSAGE_SIZE 128

// Message structure
struct SecureMessage {
    char deviceId[8];        // Sender's device ID
    uint32_t timestamp;      // Message timestamp
    uint8_t messageType;     // Type of message (0=data, 1=ack, 2=auth)
    uint8_t messageId;       // Unique message ID
    char payload[MAX_MESSAGE_SIZE]; // Actual message
    uint32_t signature;      // Message signature
};

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Message counter for unique IDs
uint8_t messageCounter = 0;

// Function to generate a simple signature
uint32_t generateSignature(const char* message, const char* key) {
    uint32_t signature = 0;
    for(int i = 0; message[i] != '\0'; i++) {
        signature = signature * 31 + message[i];
    }
    for(int i = 0; key[i] != '\0'; i++) {
        signature = signature * 31 + key[i];
    }
    return signature;
}

// Function to verify message signature
bool verifySignature(const SecureMessage* msg, const char* key) {
    uint32_t calculatedSignature = generateSignature(msg->payload, key);
    return calculatedSignature == msg->signature;
}

// Function to send a secure message
bool sendSecureMessage(const char* message, uint8_t messageType = 0) {
    SecureMessage secureMsg;
    
    // Fill message structure
    strncpy(secureMsg.deviceId, DEVICE_ID, sizeof(secureMsg.deviceId));
    secureMsg.timestamp = millis();
    secureMsg.messageType = messageType;
    secureMsg.messageId = messageCounter++;
    strncpy(secureMsg.payload, message, MAX_MESSAGE_SIZE);
    
    // Generate signature
    secureMsg.signature = generateSignature(message, SHARED_KEY);
    
    // Send the message
    return rf95.send((uint8_t*)&secureMsg, sizeof(SecureMessage));
}

// Function to receive and verify a secure message
bool receiveSecureMessage(SecureMessage* msg) {
    uint8_t len = sizeof(SecureMessage);
    if (rf95.recv((uint8_t*)msg, &len)) {
        // Verify signature
        if (verifySignature(msg, SHARED_KEY)) {
            return true;
        }
        Serial.println("Message signature verification failed!");
    }
    return false;
}

void setup() {
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    delay(100);
    Serial.println("Secure LoRa Chat!");

    // Manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        while (1);
    }
    Serial.println("LoRa radio init OK!");

    // Set frequency
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);

    // Set power level
    rf95.setTxPower(23, false);
    
    // Send authentication message
    sendSecureMessage("AUTH", 2);
}

void loop() {
    // Check if a message is available
    if (rf95.available()) {
        SecureMessage receivedMsg;
        
        if (receiveSecureMessage(&receivedMsg)) {
            Serial.print("Received from ");
            Serial.print(receivedMsg.deviceId);
            Serial.print(" (Type: ");
            Serial.print(receivedMsg.messageType);
            Serial.print(", ID: ");
            Serial.print(receivedMsg.messageId);
            Serial.print("): ");
            Serial.println(receivedMsg.payload);
            
            // Send acknowledgment for data messages
            if (receivedMsg.messageType == 0) {
                char ackMsg[32];
                snprintf(ackMsg, sizeof(ackMsg), "ACK:%d", receivedMsg.messageId);
                sendSecureMessage(ackMsg, 1);
            }
        }
    }

    // Check for user input
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.length() > 0) {
            Serial.print("Sending: ");
            Serial.println(input);

            if (sendSecureMessage(input.c_str())) {
                Serial.println("Message sent successfully!");
            } else {
                Serial.println("Failed to send message!");
            }
        }
    }
}