#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>

#define DHTTYPE DHT22

const char *wifi_ssid = "wifi_ssid";
const char *wifi_password = "wifi_password";
const char *mqtt_broker = "mqtt-broker-ip";
const int DHTPIN = 33;
const int MOIST_SIG = 32;
const int MOIST_VCC = 13;

const int PUMP_A1 = 27;
const int PUMP_A2 = 26;
const int PUMP_PWM = 14;

const int PHOTO_SIG = 35;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

void setup_wifi();

void setup() {
	Serial.begin(115200);

	//Pump setup
	pinMode(PUMP_A1, OUTPUT);
	pinMode(PUMP_A2, OUTPUT);
	pinMode(PUMP_PWM, OUTPUT);
	digitalWrite(PUMP_A1, LOW);
	digitalWrite(PUMP_A2, LOW);
	ledcSetup(0, 30000, 8);
	ledcAttachPin(PUMP_PWM, 0);

	//Moisture setup
	pinMode(MOIST_SIG, INPUT);
	pinMode(MOIST_VCC, OUTPUT);
	digitalWrite(MOIST_VCC, LOW);

	//Temperature setup
	dht.begin();

	//Light setup
	pinMode(PHOTO_SIG, INPUT);

	//WiFi setup
	setup_wifi();
	client.setServer(mqtt_broker, 1883);
	client.setCallback(callback);
}

void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(wifi_ssid);

	WiFi.begin(wifi_ssid, wifi_password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length) {
	Serial.print("Message arrived on topic: ");
	Serial.print(topic);
	Serial.print(". Message: ");
	String messageTemp;

	for (int i = 0; i < length; i++) {
		Serial.print((char)message[i]);
		messageTemp += (char)message[i];
	}
	Serial.println();

	if (String(topic) == "plant/pump") {
		if (messageTemp == "on") {
			Serial.println("Turning pump ON");
			ledcWrite(0, 220);
			digitalWrite(PUMP_A1, LOW);
			digitalWrite(PUMP_A2, HIGH);
		}
		else if (messageTemp == "off") {
			Serial.println("Turning pump OFF");
			ledcWrite(0, 0);
			digitalWrite(PUMP_A1, LOW);
			digitalWrite(PUMP_A2, LOW);
		}
		else if (messageTemp == "water") {
			// Automatic watering set to only 2 seconds to avoid overwatering
			// Delay is fine here, program pauses for 2 seconds to water the plant with no need to listen to other messages
			Serial.println("Watering plant");
			ledcWrite(0, 220);
			digitalWrite(PUMP_A1, LOW);
			digitalWrite(PUMP_A2, HIGH);
			delay(2000);
			ledcWrite(0, 0);
			digitalWrite(PUMP_A1, LOW);
			digitalWrite(PUMP_A2, LOW);
		}
	}
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266Client")) {
			Serial.println("connected");
			// Subscribe
			client.subscribe("plant/pump");
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void loop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	long now = millis();
	if (now - lastMsg > 900000) {

		//Plant moisture reading
		digitalWrite(MOIST_VCC, HIGH);
		delay(10);
		int moisture = analogRead(MOIST_SIG);
		digitalWrite(MOIST_VCC, LOW);
		char moistureString[8];
		itoa(moisture, moistureString, 10);

		//Room temperature reading
		float temperature = dht.readTemperature();
		char temperatureString[8];
		dtostrf(temperature, 1, 2, temperatureString);

		//Room humidity reading
		float humidity = dht.readHumidity();
		char humidityString[8];
		dtostrf(humidity, 1, 2, humidityString);

		//Room light reading
		int light = analogRead(PHOTO_SIG);
		char lightString[8];
		itoa(light, lightString, 10);

		Serial.print("Moisture: ");
		Serial.println(moistureString);
		client.publish("plant/moisture", moistureString);

		Serial.print("Temperature: ");
		Serial.println(temperatureString);
		client.publish("room/temp", temperatureString);

		Serial.print("Light: ");
		Serial.println(lightString);
		client.publish("room/light", lightString);

		Serial.print("Humidity: ");
		Serial.println(humidityString);
		client.publish("room/humidity", humidityString);
		lastMsg = now;
	}
}
