/*
 * Epicure Robotics - ESP32 MQTT to UART Bridge
 * * This firmware connects to WiFi and an MQTT broker.
 * It subscribes to the "epicure/commands" topic.
 * When a message is received, it forwards it over UART2 (Serial2)
 * to the STM32 microcontroller.
 */

#include <WiFi.h>
#include <PubSubClient.h>

// --- Configuration ---
// !! UPDATE THESE WITH YOUR CREDENTIALS !!
const char* WIFI_SSID = "TP-Link_7E13_5G";
const char* WIFI_PASSWORD = "05894895";

const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "epicure/commands";
const char* MQTT_CLIENT_ID = "esp32-epicure-bridge";

// Define the UART for communication with STM32
// We use Serial2 (GPIO 16 (RX2), 17 (TX2))
// This leaves Serial (UART0) free for debugging and uploading.
#define STM_SERIAL Serial2
#define STM_BAUD_RATE 115200
#define STM_RX_PIN 16
#define STM_TX_PIN 17

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);

/**
 * @brief Connects to the WiFi network.
 */
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief Callback function for when an MQTT message arrives.
 * * @param topic The topic the message was published to.
 * @param payload The message payload.
 * @param length The length of the payload.
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // Create a null-terminated string from the payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.println(message);

    // Forward the received message to the STM32 via UART
    // We use println to add a newline, which the STM32 can use as a delimiter.
    STM_SERIAL.println(message);
    Serial.print("Forwarded to STM32: '");
    Serial.print(message);
    Serial.println("'");
}

/**
 * @brief Reconnects to the MQTT broker if the connection is lost.
 */
void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(MQTT_CLIENT_ID)) {
            Serial.println("connected");
            // Subscribe to the command topic
            client.subscribe(MQTT_TOPIC);
            Serial.print("Subscribed to: ");
            Serial.println(MQTT_TOPIC);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

/**
 * @brief Setup function, runs once at startup.
 */
void setup() {
    // Initialize primary serial port for debugging
    Serial.begin(115200);
    
    // Initialize the secondary serial port for STM32 communication
    // begin(baud, config, rxPin, txPin)
    STM_SERIAL.begin(STM_BAUD_RATE, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);

    setup_wifi();
    
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(mqtt_callback);

    Serial.println("Setup complete. ESP32 Bridge is running.");
}

/**
 * @brief Main loop, runs continuously.
 */
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
