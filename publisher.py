import paho.mqtt.client as mqtt
import time
import sys

# --- Configuration ---
MQTT_BROKER = "broker.hivemq.com"  # Public test broker
MQTT_PORT = 1883
MQTT_TOPIC = "epicure/commands"
# ---------------------

def on_connect(client, userdata, flags, rc, properties=None):
    """
    Callback for when the client receives a CONNACK response from the server.
    """
    if rc == 0:
        print(f"Successfully connected to MQTT Broker at {MQTT_BROKER}")
    else:
        print(f"Failed to connect, return code {rc}\n")

def on_disconnect(client, userdata, rc, properties=None):
    """
    Callback for when the client disconnects.
    """
    print(f"Disconnected from MQTT Broker. Return code: {rc}. Reconnecting...")
    # The client's automatic reconnect logic will handle this.
    # If using loop_start(), it manages reconnection automatically.

def create_mqtt_client():
    """
    Creates and configures the MQTT client.
    """
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    
    # Set up automatic reconnection
    # These are default settings but good to be explicit
    client.reconnect_delay_set(min_delay=1, max_delay=120)
    
    return client

def main():
    """
    Main function to run the publisher.
    """
    client = create_mqtt_client()
    
    try:
        print(f"Attempting to connect to {MQTT_BROKER}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        
        # loop_start() runs the network loop in a background thread.
        # It handles pings, reconnections, and dispatching callbacks.
        client.loop_start()
        
        print("\n--- Epicure Robotics Command Publisher ---")
        print("Type your command and press Enter.")
        print("Examples: 'led:on', 'led:off', 'motor:100:1'")
        print("Press CTRL+C to quit.\n")
        
        while True:
            # Continuously read user input
            command = input("> ")
            if command:
                # Publish the input to the MQTT topic
                result = client.publish(MQTT_TOPIC, command)
                # Wait for the publish to complete (optional, for QoS 1+)
                # result.wait_for_publish() 
                print(f"Published: '{command}' to '{MQTT_TOPIC}'")
                
    except KeyboardInterrupt:
        print("\nDisconnecting and shutting down...")
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        # Stop the network loop and disconnect gracefully
        client.loop_stop()
        client.disconnect()
        print("Shutdown complete.")
        sys.exit(0)

if __name__ == "__main__":
    main()
