#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTT.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define DHTPIN  2

const char* ssid     = "<SSID>";
const char* password = "<PASSWORD>";

MQTTClient client;
WiFiClient net;

unsigned long lastMillis = 0;

// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

float humidity, temp_c;  // Values read from sensor

void connect()
{
  Serial.print("\n\r\nConnecting to MQTT broker");
  while(!client.connect("AliAbu", "try", "try"))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to MQTT broker");

  // Subcribe to
  client.subscribe("/ali/test_d1");
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);
}

bool gettemperature() {
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_c = dht.readTemperature();     // Read temperature as Celcius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
      return false;
    }
    Serial.print("Temperature: ");
    Serial.print(temp_c);
    Serial.println("C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    return true;
}

void setup() {
  Serial.begin(115200); // Serial connection from ESP-01 initialization
  dht.begin();           // initialize temperature sensor

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("\n\r\n Connecting to WiFi");

  // Wait for connection
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected to");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MQTT connect
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if(!client.connected()){
    connect();
  }

  if(millis() - lastMillis > 5000) {
    lastMillis = millis();

    if(gettemperature()){
      // client.publish("/ali/dev/temp", String((int)temp_c));
      // client.publish("/ali/dev/humidity", String((int)humidity));
      client.publish("/ali/dev/temp", String(temp_c));
      client.publish("/ali/dev/humidity", String(humidity));
    }
    
  }

}
