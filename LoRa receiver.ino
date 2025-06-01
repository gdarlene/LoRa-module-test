#include <SPI.h>
#include <RH_RF95.h>
#include <AES.h>
#include <CBC.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0
#define LED 13

// Security settings - must match sender!
#define SHARED_KEY "YourSecretKey123" // 16 bytes for AES-128
#define DEVICE_ID "RECEIVER1"         // Unique device identifier
#define MAX_MESSAGE_SIZE 128

// Message structure - must match sender!
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

// Function to send a secure message
bool sendSecureMessage(const char* message, uint8_t messageType = 0) {
    SecureMessage secureMsg;
    
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
    
    // Send the message
    return rf95.send((uint8_t*)&secureMsg, sizeof(SecureMessage));
}

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    delay(100);
    Serial.println("Secure LoRa Receiver!");

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

    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: "); 
    Serial.println(RF95_FREQ);

    rf95.setTxPower(23, false);
}

void loop() {
    if (rf95.available()) {
        SecureMessage receivedMsg;
        uint8_t len = sizeof(SecureMessage);
        
        if (rf95.recv((uint8_t*)&receivedMsg, &len)) {
            // Verify signature
            if (verifySignature(&receivedMsg, SHARED_KEY)) {
                digitalWrite(LED, HIGH);
                
                // Decrypt the message
                decryptMessage(receivedMsg.payload, SHARED_KEY);
                
                // Print received message details
                Serial.print("Received from ");
                Serial.print(receivedMsg.deviceId);
                Serial.print(" (Type: ");
                Serial.print(receivedMsg.messageType);
                Serial.print(", ID: ");
                Serial.print(receivedMsg.messageId);
                Serial.print(", Retry: ");
                Serial.print(receivedMsg.retryCount);
                Serial.print("): ");
                Serial.println(receivedMsg.payload);
                
                // Print RSSI
                Serial.print("RSSI: ");
                Serial.println(rf95.lastRssi(), DEC);
                
                // Handle different message types
                switch(receivedMsg.messageType) {
                    case 0: // Data message
                        // Send acknowledgment
                        char ackMsg[32];
                        snprintf(ackMsg, sizeof(ackMsg), "ACK:%d", receivedMsg.messageId);
                        sendSecureMessage(ackMsg, 1);
                        break;
                        
                    case 1: // Acknowledgment
                        Serial.println("Received acknowledgment");
                        break;
                        
                    case 2: // Authentication
                        Serial.println("Received authentication request");
                        // Send authentication response
                        sendSecureMessage("AUTH_OK", 2);
                        break;
                }
                
                digitalWrite(LED, LOW);
            } else {
                Serial.println("Message signature verification failed!");
            }
        }
    }
}


Message Reine:brown-heart-ios:Nabonye Umukunzi mwiza









