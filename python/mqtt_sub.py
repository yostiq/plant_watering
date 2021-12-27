import paho.mqtt.client as paho
import time
import sqlite3
from datetime import datetime
mqtt_broker = "mqtt-broker-ip"
mqtt_port = 1883

# check_same_thread set to false because flask is also accessing db. Should be safe as flask is only reading it.
con = sqlite3.connect('plant.db', check_same_thread=False)
cur = con.cursor()

def on_message(client, userdata, message):
    msg = str(message.payload.decode("utf-8"))
    topic = message.topic

    now = datetime.now()
    date = now.strftime('%H:%M')
    # Optionally date can include the date/month/year as well
    #date = now.strftime('%d/%m/%y %H:%M')

    # ESP32 ADC resolution is 4096, so value/4096*100 is the percentage of the value
    if topic == 'plant/moisture':
        msg = float(msg)
        msg = int(msg / 4096 * 100)
        cur.execute(f"INSERT INTO moisture (date, value) VALUES ('{date}', {msg})")
        # Automatic watering if moisture is between 1-50%
        if (msg < 50 and msg > 0 ):
            client.publish("plant/pump", "water")
    elif topic == 'room/light':
        msg = float(msg)
        msg = int(msg / 4096 * 100)
        cur.execute(f"INSERT INTO light (date, value) VALUES ('{date}', {msg})")
    elif topic == 'room/temp':
        cur.execute(f"INSERT INTO temperature (date, value) VALUES ('{date}', {msg})")
    elif topic == 'room/humidity':
        cur.execute(f"INSERT INTO humidity (date, value) VALUES ('{date}', {msg})")
    con.commit()


client = paho.Client("raspi_sub")
client.connect(mqtt_broker, mqtt_port)
client.on_message = on_message
client.subscribe("plant/moisture")
client.subscribe("room/temp")
client.subscribe("room/light")
client.subscribe("room/humidity")
client.loop_start()

while 1:
    time.sleep(3600)

client.loop_stop()
con.close()

# Testing sqlite3 commands, left here for reference
#cur.execute('''CREATE TABLE temperature (date TEXT, value REAL)''')
#cur.execute("INSERT INTO temperature (id, date, value) VALUES ('25-11-2021 12:59', 24.10)")
