# Setup

## ESP32
.ino file found in esp32/plant.ino  
Setup following variables before flashing:  
- `*wifi_ssid`
- `*wifi_password`
- `*mqtt_broker`

## Server

### Flask
git clone https://github.com/yostiq/plant_watering  
cd plant_watering  
python3 -m venv venv  
source venv/bin/activate  
pip3 install -r requirements.txt  
gunicorn -w4 -b \<LOCAL IP\>:5000 app:app  

### MQTT sub
Setup `mqtt_broker` and `mqtt_port` variables in `python/mqtt_sub.py` before running.  
cd plant_watering  
source venv/bin/activate  
python3 python/mqtt_sub.py

### SQLite database
Database with example values already in python/plant.db  

Schema:  
`CREATE TABLE sqlite_sequence(name,seq);`  
`CREATE TABLE light (date TEXT, value REAL);`  
`CREATE TABLE humidity (date TEXT, value REAL);`  
`CREATE TABLE moisture (date TEXT, value REAL);`  
`CREATE TABLE temperature (date TEXT, value REAL);`  