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
#define MAX_RETRIES 3
#define RETRY_DELAY 1000  // 1 second delay between retries

// Message structure
struct SecureMessage {
    char deviceId[8];        // Sender's device ID
    uint32_t timestamp;      // Message timestamp
    uint8_t messageType;     // Type of message (0=data, 1=ack, 2=auth)
    uint8_t messageId;       // Unique message ID
    uint8_t retryCount;      // Number of retries attempted
    char payload[MAX_MESSAGE_SIZE]; // Actual message
    uint32_t signature;      // Message signature
};

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Message counter for unique IDs
uint8_t messageCounter = 0;

// AES encryption objects
AES128 aes128;
CBC<AES128> cbc;

// Function to encrypt message
void encryptMessage(char* message, const char* key) {
    // Pad message to 16 bytes (AES block size)
    int messageLen = strlen(message);
    int paddedLen = ((messageLen + 15) / 16) * 16;
    char paddedMessage[paddedLen];
    memcpy(paddedMessage, message, messageLen);
    memset(paddedMessage + messageLen, 0, paddedLen - messageLen);
    
    // Set up encryption
    cbc.setKey(key, 16);
    cbc.setIV(key, 16);  // Using key as IV for simplicity
    
    // Encrypt
    cbc.encrypt(paddedMessage, paddedMessage, paddedLen);
    
    // Copy back to original message
    memcpy(message, paddedMessage, paddedLen);
    message[paddedLen] = '\0';
}

// Function to decrypt message
void decryptMessage(char* message, const char* key) {
    int messageLen = strlen(message);
    
    // Set up decryption
    cbc.setKey(key, 16);
    cbc.setIV(key, 16);
    
    // Decrypt
    cbc.decrypt(message, message, messageLen);
    
    // Remove padding
    int i = messageLen - 1;
    while (i >= 0 && message[i] == 0) {
        message[i] = '\0';
        i--;
    }
}

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

// Function to send a secure message with retry
bool sendSecureMessageWithRetry(const char* message, uint8_t messageType = 0) {
    SecureMessage secureMsg;
    bool success = false;
    uint8_t retries = 0;
    
    // Fill message structure
    strncpy(secureMsg.deviceId, DEVICE_ID, sizeof(secureMsg.deviceId));
    secureMsg.timestamp = millis();
    secureMsg.messageType = messageType;
    secureMsg.messageId = messageCounter++;
    secureMsg.retryCount = 0;
    
    // Encrypt the message
    char encryptedMessage[MAX_MESSAGE_SIZE];
    strncpy(encryptedMessage, message, MAX_MESSAGE_SIZE);
    encryptMessage(encryptedMessage, SHARED_KEY);
    strncpy(secureMsg.payload, encryptedMessage, MAX_MESSAGE_SIZE);
    
    // Generate signature
    secureMsg.signature = generateSignature(encryptedMessage, SHARED_KEY);
    
    // Try sending with retries
    while (!success && retries < MAX_RETRIES) {
        if (rf95.send((uint8_t*)&secureMsg, sizeof(SecureMessage))) {
            // Wait for acknowledgment
            uint32_t startTime = millis();
            while (millis() - startTime < RETRY_DELAY) {
                if (rf95.available()) {
                    SecureMessage ackMsg;
                    uint8_t len = sizeof(SecureMessage);
                    if (rf95.recv((uint8_t*)&ackMsg, &len)) {
                        if (ackMsg.messageType == 1 && // It's an ACK
                            ackMsg.messageId == secureMsg.messageId) {
                            success = true;
                            break;
                        }
                    }
                }
                delay(10);
            }
        }
        
        if (!success) {
            retries++;
            secureMsg.retryCount = retries;
            Serial.print("Retry ");
            Serial.print(retries);
            Serial.print(" of ");
            Serial.println(MAX_RETRIES);
            delay(RETRY_DELAY);
        }
    }
    
    return success;
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
    if (sendSecureMessageWithRetry("AUTH", 2)) {
        Serial.println("Authentication successful!");
    } else {
        Serial.println("Authentication failed!");
    }
}

void loop() {
    // Check if a message is available
    if (rf95.available()) {
        SecureMessage receivedMsg;
        uint8_t len = sizeof(SecureMessage);
        
        if (rf95.recv((uint8_t*)&receivedMsg, &len)) {
            // Verify signature
            if (verifySignature(&receivedMsg, SHARED_KEY)) {
                // Decrypt the message
                decryptMessage(receivedMsg.payload, SHARED_KEY);
                
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
                    sendSecureMessageWithRetry(ackMsg, 1);
                }
            } else {
                Serial.println("Message signature verification failed!");
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

            if (sendSecureMessageWithRetry(input.c_str())) {
                Serial.println("Message sent successfully!");
            } else {
                Serial.println("Failed to send message after retries!");
            }
        }
    }
}