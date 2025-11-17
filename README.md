# **Epicure Robotics \- MQTT-to-Hardware Control System**

This repository contains the complete code base for the Epicure Robotics Robotic Software Engineer task. It implements a communication system to send commands from a PC to an STM32 microcontroller (controlling a stepper motor and LED) via an ESP32 bridge.

## **1\. System Architecture**

The system operates on a publish-subscribe model, passing commands through the following flow:

Python (User Input) → MQTT Broker → ESP32 (Subscriber) → UART → STM32 (Hardware Control)

### **Message Format**

The system uses a simple, colon-delimited string format for commands:

* led:on: Turns the LED on.  
* led:off: Turns the LED off.  
* motor:\<steps\>:\<direction\>: Moves the motor.  
  * \<steps\>: Number of steps (e.g., 100).  
  * \<direction\>: 0 or 1 (e.g., 1 for forward, 0 for reverse).  
  * **Example:** motor:200:1

## **2\. Repository Structure**

This repository is organized into three main components and this guide:

* publisher.py: The Python script that runs on the PC, takes user input, and publishes commands to the MQTT broker.  
* esp32\_mqtt\_uart\_bridge/esp32\_mqtt\_uart\_bridge.ino: The ESP32 firmware. It connects to WiFi, subscribes to the MQTT topic, and forwards all received commands to the STM32 via UART.  
* stm32\_controller/stm32\_controller.ino: The STM32 firmware. It listens for commands on its UART port, parses them, and executes hardware actions (toggling the LED or driving the stepper motor).  
* README.md: This setup and documentation file.

## **3\. How to Set Up and Run**

Here are the instructions to set up and test each component of the system.

### **Part A: Python Publisher (publisher.py)**

This script runs on your computer, takes user input, and publishes it to the MQTT broker.

#### **Dependencies**

You only need the paho-mqtt library.

pip install paho-mqtt

#### **Setup**

1. The script is pre-configured to use a public test broker: broker.hivemq.com. No changes are needed for testing.  
2. If you have a local broker (like Mosquitto), change the MQTT\_BROKER variable.

#### **How to Run**

1. Open your terminal or command prompt.  
2. Run the script:  
   python publisher.py

3. The script will connect and prompt you for commands. Type led:on and press Enter.

### **Part B: ESP32 Bridge (esp32\_mqtt\_uart\_bridge.ino)**

This firmware runs on the ESP32, subscribing to MQTT and forwarding commands to the STM32 via UART.

#### **IDE & Libraries**

* **IDE:** [Arduino IDE](https://www.arduino.cc/en/software)  
* **Board:** Add ESP32 boards to your Arduino IDE.  
  1. Go to File \> Preferences.  
  2. Add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package\_esp32\_index.json to "Additional Board Manager URLs".  
  3. Go to Tools \> Board \> Boards Manager....  
  4. Search for "esp32" and install the package by Espressif Systems.  
* **Library:** PubSubClient  
  1. Go to Tools \> Manage Libraries....  
  2. Search for "PubSubClient" by Nick O'Leary and install it.

#### **Setup**

1. Open esp32\_mqtt\_uart\_bridge/esp32\_mqtt\_uart\_bridge.ino in the Arduino IDE.  
2. **\!\! IMPORTANT \!\!** Update the following lines with your WiFi credentials:  
   const char\* WIFI\_SSID \= "YOUR\_WIFI\_SSID";  
   const char\* WIFI\_PASSWORD \= "YOUR\_WIFI\_PASSWORD";

3. Select your board (Tools \> Board \> ESP32 Arduino \> ESP32 Dev Module).  
4. Select the correct COM port and click **Upload**.

### **Part C: STM32 Controller (stm32\_controller.ino)**

This firmware runs on the STM32F407VET6, listening for UART commands and controlling the hardware.

#### **IDE & Board Setup**

* **IDE:** Arduino IDE  
* **Board:** Add STM32 boards to your Arduino IDE.  
  1. Go to File \> Preferences.  
  2. Add https://github.com/stm32duino/BoardManagerFiles/raw/main/package\_stm32\_index.json to "Additional Board Manager URLs".  
  3. Go to Tools \> Board \> Boards Manager....  
  4. Search for "STM32 MCU based boards" and install it.  
* **Board Selection:**  
  1. Go to Tools \> Board \> STM32 Boards.  
  2. Select STM32F4 series.  
  3. For Board part number, select STM32F407VE(T).  
  4. Set Upload method to STM32CubeProgrammer (SWD).

#### **Setup**

1. Open stm32\_controller/stm32\_controller.ino in the Arduino IDE.  
2. **Verify Pin Definitions:** The code uses PC13 (LED), PA0 (DIR), and PA1 (STEP). If the test PCB uses different pins, these constants must be updated.  
3. Connect your ST-Link programmer and click **Upload**.

## **4\. Hardware & Testing**

### **Hardware Connections**

This is the physical wiring required between the two microcontrollers.

| ESP32 (wroom32d) | STM32 (STM32F407VET6) | Purpose |
| :---- | :---- | :---- |
| **GND** | **GND** | **Common Ground (CRITICAL)** |
| **GPIO 17 (TX2)** | **PA10 (RX1)** | ESP32 Transmit \-\> STM32 Receive |
| **GPIO 16 (RX2)** | **PA9 (TX1)** | ESP32 Receive \<- STM32 Transmit |

*Note: We use Serial2 on the ESP32 and Serial1 on the STM32. The pins PA10/PA9 correspond to Serial1 on the STM32F407.*

### **End-to-End Test Flow**

1. **Hardware:** Wire the ESP32 and STM32 together as shown.  
2. **Firmware:** Upload the firmware to both boards.  
3. **Run:**  
   * Open Serial Monitors for *both* boards (baud rate 115200).  
   * Power on the microcontrollers.  
   * The ESP32 monitor should show a connection to WiFi and MQTT.  
   * The STM32 monitor should show "Waiting for commands...".  
4. **Execute:**  
   * Run the python publisher.py script on your computer.  
   * Type led:on in the Python console and press Enter.  
5. **Observe:**  
   * **Python:** Will show "Published: 'led:on'".  
   * **ESP32 Monitor:** Will show "Message arrived... Forwarded to STM32".  
   * **STM32 Monitor:** Will show "Received command: 'led:on'... Action: Turning LED ON".  
   * **Hardware:** The LED on the STM32 board will turn on.
