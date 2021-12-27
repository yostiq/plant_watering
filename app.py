from flask import Flask, render_template, request
from collections import OrderedDict
import paho.mqtt.client as paho
import sqlite3
import json
import time

app = Flask(__name__)
app.static_folder = 'static'

mqtt_broker = "192.168.43.164"
mqtt_port = 1883
client = paho.Client('Flask')

# Returns list of dates and values from specified sqlite3 table
def getData(table, entries):
    con = sqlite3.connect('python/plant.db')
    cur = con.cursor()
    
    data = {}
    dates = []
    values = []

    for val, row in enumerate(cur.execute(f'SELECT date, value from {table} ORDER BY rowid DESC LIMIT {entries}')):
        data[val] = {
            "date": row[0],
            "value": row[1]
        }
    #reversed_data = dict(reversed(data.items()))
    con.close()
    return data

@app.route('/', methods=['GET', 'POST'])
def root():
    if request.method == 'POST':
        if request.form['mqtt'] == 'Water the plant':
            client.connect(mqtt_broker, mqtt_port)
            client.publish("plant/pump", "water")
        elif request.form['mqtt'] == 'Pump on':
            client.connect(mqtt_broker, mqtt_port)
            client.publish("plant/pump", "on")
        elif request.form['mqtt'] == 'Pump off':
            client.connect(mqtt_broker, mqtt_port)
            client.publish("plant/pump", "off")

    # 96 entries of values is last 24h in 15min intervals
    temperature = getData('temperature', 96)
    humidity = getData('humidity', 96)
    light = getData('light', 96)
    moisture = getData('moisture', 96)
    return render_template('index.html', temperature=temperature, humidity=humidity, light=light, moisture=moisture)