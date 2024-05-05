"""
Author: Joshua Dotto
Date: 2023-10-22

This code runs on an internet-connected gateway
that collects sensor data from an end-device via
a serial USB connection and pushes the data to a 
remote server over MQTT
"""

import os
import time
import serial
import json
import paho.mqtt.client as paho
import ssl
from dotenv import load_dotenv

load_dotenv('../.env')

end_device_port = os.getenv('END_DEVICE_PORT')
broker_username = os.getenv('BROKER_USERNAME')
broker_password = os.getenv('BROKER_PASSWORD')
broker_url = os.getenv('BROKER_URL')
broker_port = int(os.getenv('BROKER_PORT'))

# set up mqtt

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")

def on_publish(client, userdata, mid):
    print("message id: "+str(mid))

client = paho.Client()
client.tls_set(ca_certs=None, certfile=None, keyfile=None, cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLS)
client.username_pw_set(broker_username, broker_password)
client.connect(broker_url, broker_port)
client.on_connect = on_connect
client.on_publish = on_publish
client.loop_start()

end_device = serial.Serial(end_device_port, 115200)
end_device.flushInput()

while True:
    try:
        end_device_data_string = end_device.readline().decode('utf-8').rstrip()
        end_device_data_dict = json.loads(end_device_data_string)
        end_device_data_dict['location'] = 'bedroom'
        end_device_data_string = json.dumps(end_device_data_dict)

        if 'lights' in end_device_data_dict:
            client.publish("data/lights", payload=end_device_data_string, qos=1)

        if 'temp' in end_device_data_dict:
            client.publish("data/temperature", payload=end_device_data_string, qos=1)
        
    except KeyboardInterrupt:
        break
     
end_device.close()

