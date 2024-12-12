# Relay node that subscribes to the broker
# Subscribe to HiveMQ broker and grab the potentiometer data. Then, push this
# data to a Hostinger database.

import paho.mqtt.client as mqtt
import mysql.connector
from datetime import datetime

# HiveMQ settings
MQTT_SERVER = "broker.mqtt-dashboard.com"
MQTT_PORT = 1883
MQTT_TOPIC = "testtopic/temp/outTopic/potvalue"

# Hostinger database settings
DB_HOST = ""  # Replace with your database host
DB_USER = ""       # Replace with your database username
DB_PASSWORD = ""   # Replace with your database password
DB_NAME = ""           # Replace with your database name

# Callback when connected
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"Failed to connect, return code {rc}")

# Callback when message is received
def on_message(client, userdata, message):
    try:
        payload = str(message.payload.decode("utf-8"))
        print(f"Received data: {payload}")

        # Process only messages containing potentiometer data
        if "Pot Value:" in payload:
            pot_value = payload.split(":")[1].strip()  # Extract the numeric value
            print(f"Extracted Potentiometer Value: {pot_value}")

            # Insert data into Hostinger database
            try:
                conn = mysql.connector.connect(
                    host=DB_HOST,
                    user=DB_USER,
                    password=DB_PASSWORD,
                    database=DB_NAME
                )
                cursor = conn.cursor()
                
                # Define your table and fields; ensure the table exists
                insert_query = "INSERT INTO potentiometer_data (value, timestamp) VALUES (%s, %s)"
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                cursor.execute(insert_query, (pot_value, timestamp))
                conn.commit()
                print("Data inserted into Hostinger database successfully.")
            except mysql.connector.Error as db_err:
                print(f"Database error: {db_err}")
            finally:
                if conn.is_connected():
                    cursor.close()
                    conn.close()
        else:
            print("Message does not contain potentiometer data. Ignoring.")
    except Exception as e:
        print(f"Error processing message: {e}")

# MQTT Client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to HiveMQ broker
client.connect(MQTT_SERVER, MQTT_PORT, 60)
client.loop_forever()
