/*
 * Epicure Robotics - STM32 Controller
 * * This firmware listens for commands on a UART peripheral (Serial1)
 * coming from the ESP32.
 * * It parses commands and controls an LED and a stepper motor.
 * * Hardware (Example for STM32F407VE):
 * - UART1: PA10 (RX1), PA9 (TX1)
 * - On-board LED: PC13 (or specify another)
 * - Stepper DIR Pin: PA0
 * - Stepper STEP Pin: PA1
 */

// --- Pin Configuration ---
// Adjust these pins to match your actual hardware setup
const int LED_PIN = PC13;       // On-board LED on many "Black Pill" / F407VE boards
const int STEPPER_DIR_PIN = PA0; // Stepper motor Direction pin
const int STEPPER_STEP_PIN = PA1; // Stepper motor Step pin

// --- UART Configuration ---
// Define the UART port connected to the ESP32.
// For STM32F407VE, Serial1 is on PA10 (RX) and PA9 (TX).
// We must cross-connect: ESP_TX -> STM_RX (PA10), ESP_RX -> STM_TX (PA9)
#define ESP_SERIAL Serial1
#define ESP_BAUD_RATE 115200

// --- Globals ---
String uartBuffer = "";     // Buffer to store incoming UART data
bool commandReady = false;  // Flag when a full command is received

/**
 * @brief Setup function, runs once at startup.
 */
void setup() {
    // Initialize primary serial (over USB) for debugging
    Serial.begin(115200);
    Serial.println("STM32 Controller Initializing...");

    // Initialize the UART for communication with ESP32
    ESP_SERIAL.begin(ESP_BAUD_RATE);
    
    // Configure hardware pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(STEPPER_DIR_PIN, OUTPUT);
    pinMode(STEPPER_STEP_PIN, OUTPUT);
    
    // Set initial state
    digitalWrite(LED_PIN, LOW); // LED off (or HIGH if it's active-low like PC13)
    // digitalWrite(LED_PIN, HIGH); // Use this for active-low PC13
    
    Serial.println("Setup complete. Waiting for commands...");
}

/**
 * @brief Main loop, runs continuously.
 */
void loop() {
    // 1. Read incoming data from ESP32
    while (ESP_SERIAL.available() > 0) {
        char inChar = (char)ESP_SERIAL.read();
        
        // The ESP32 sends a newline ('\n') at the end of each command
        if (inChar == '\n') {
            commandReady = true;
            break; // Process the command
        } else if (inChar != '\r') {
            // Add character to the buffer (ignore carriage return)
            uartBuffer += inChar;
        }
    }

    // 2. Process the command if a full one is received
    if (commandReady) {
        Serial.print("Received command: '");
        Serial.print(uartBuffer);
        Serial.println("'");
        
        parseCommand(uartBuffer);
        
        // Clear the buffer and reset the flag
        uartBuffer = "";
        commandReady = false;
    }
}

/**
 * @brief Parses the received command string and executes the action.
 * * @param cmd The command string to parse.
 */
void parseCommand(String cmd) {
    cmd.trim(); // Remove any leading/trailing whitespace

    if (cmd.equals("led:on")) {
        Serial.println("Action: Turning LED ON");
        digitalWrite(LED_PIN, HIGH); // (or LOW for active-low PC13)
        // digitalWrite(LED_PIN, LOW); // Use for active-low PC13

    } else if (cmd.equals("led:off")) {
        Serial.println("Action: Turning LED OFF");
        digitalWrite(LED_PIN, LOW); // (or HIGH for active-low PC13)
        // digitalWrite(LED_PIN, HIGH); // Use for active-low PC13
        
    } else if (cmd.startsWith("motor:")) {
        Serial.println("Action: Parsing motor command...");
        // Command format: "motor:<steps>:<direction>"
        // Example: "motor:100:1" (100 steps, direction 1)
        
        // Find the colons
        int firstColon = cmd.indexOf(':');
        int secondColon = cmd.indexOf(':', firstColon + 1);

        if (firstColon > 0 && secondColon > firstColon) {
            // Extract substrings
            String stepsStr = cmd.substring(firstColon + 1, secondColon);
            String dirStr = cmd.substring(secondColon + 1);
            
            // Convert to integers
            int steps = stepsStr.toInt();
            int dir = dirStr.toInt();
            
            if (steps > 0) {
                Serial.print("  Moving motor ");
                Serial.print(steps);
                Serial.print(" steps in direction ");
                Serial.println(dir);
                moveMotor(steps, dir);
            } else {
                Serial.println("  Error: Invalid steps value.");
            }
        } else {
            Serial.println("  Error: Malformed motor command.");
        }
    } else {
        Serial.println("Action: Unknown command.");
    }
}

/**
 * @brief Controls the stepper motor.
 * * @param steps Number of steps to move.
 * @param dir Direction (1 for forward, 0 for reverse).
 */
void moveMotor(int steps, int dir) {
    // Set the direction
    digitalWrite(STEPPER_DIR_PIN, (dir == 1) ? HIGH : LOW);
    
    // Wait for direction to set (optional, good practice)
    delayMicroseconds(100); 

    // Generate step pulses
    // This is a simple, blocking implementation.
    // A non-blocking timer-based approach is better for real applications
    // but this satisfies the logic requirement.
    for (int i = 0; i < steps; i++) {
        digitalWrite(STEPPER_STEP_PIN, HIGH);
        delayMicroseconds(500); // Pulse width (controls max speed)
        digitalWrite(STEPPER_STEP_PIN, LOW);
        delayMicroseconds(500); // Time between pulses
    }
    
    Serial.println("  Motor move complete.");
}
