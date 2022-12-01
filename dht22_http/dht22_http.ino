#include "WiFi.h"
#include "HTTPClient.h"
#include "DHT.h"

// set the data pin of the DHT22 sensor
#define DHTPIN 33

#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

// Configuration of your WIFI
const char* ssid = "YOUR-SSID";
const char* password = "YOUR-WIFI-PASS";

// where to send the measurement data
const char* serverName = "http://192.168.1.105:12201/gelf";

void setup() {
  // initialize internal led
  pinMode(LED_BUILTIN, OUTPUT);
  // start serial communication
  Serial.begin(115200);
  while (!Serial);
  // initialize wifi connection
  connectToWifi(ssid, password);
  // initialize DHT22 sensor
  dht.begin(); 
}


void loop() {
  // Wait a few seconds between measurements. DHT22 needs 2s delays between readings
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  // log those readings 
  Serial.println("Temperature:" + String(temperature) + " C");
  Serial.println("Humidity:" + String(humidity) + " %");
  Serial.println("Heat index:" + String(heatIndex));

  // if we have wifi, we want to publish the data to our server
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // where should the request go
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      
      // Data to send with HTTP POST
      String httpRequestData = "{\"version\": \"1.1\", \"host\": \"tomas-dvorak.cz\", \"short_message\": \"ESP32 temp data\", \"_temperature\":" + String(temperature) + ", \"_humidity\":" + String(humidity) + ", \"_heat_index\":" + String(heatIndex) + "}";           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
           
      // print response code via serial
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      // blink to celebrate one sending cycle :-)
      blink(300);

      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
}

void connectToWifi(const char* name, const char* pass) {
  Serial.println("Preparing connection");
  WiFi.begin(name, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // Print IP address of this device
  IPAddress myIP = WiFi.localIP();
  Serial.println(myIP);  
  // visually represent successful connection to wifi
  blink(5000);
}

void blink(int delayMs) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(delayMs);                   // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
}
