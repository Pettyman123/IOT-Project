#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include<PubSubClient.h>
#include<WiFi.h>


#define DHTPIN 13          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11      // Type of DHT sensor

#define RAIN_PIN 14        // Analog pin connected to the rain sensor
#define RAIN_THRESHOLD 1000 // Adjust this value according to your sensor

#define BMP_SDA 21         // I2C SDA pin
#define BMP_SCL 22         // I2C SCL pin

const char* ssid = "GalaxyA545GA539";
const char* password = "zsvnq7uptu7iktj";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(9600);
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1);
  }
  setup_wifi();
  client.setServer(mqtt_server,1883);//1883 is the default port for MQTT server

}

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); //
    Serial.print(".");
    c=c+1;
    if(c>15){
        ESP.restart(); //restart ESP after 10 seconds
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

void connect_mqttServer() {
  while (!client.connected()) {
        if(WiFi.status() != WL_CONNECTED){
          setup_wifi();
        }

        //now attemt to connect to MQTT server
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32_client1")) { // Change name of client if multiple ESP32 are connected
          Serial.println("connected");
          // Subscribe to topics here
          
        } 
        else {
          //attempt not successful
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" trying again in 3 seconds");
          // Wait 3 seconds before retrying
          delay(3000);
        }
  }
}
long lstmsg = 0;

void loop() {
  // Read temperature and humidity from DHT sensor
    if (!client.connected()) {
    connect_mqttServer();
  }
  client.loop();
  long now = millis();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read rain sensor
  int rainValue = analogRead(RAIN_PIN);
  bool isRaining = (rainValue < RAIN_THRESHOLD);

  // Read pressure from BMP180
  float pressure = bmp.readPressure() / 100.0;

  // Print sensor values
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Rain: ");
  Serial.println(isRaining ? "Yes" : "No");
  

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  if(now - lstmsg > 2000){ 
    lstmsg = now;
    client.publish("iotproject59283273_esp32/temperature", String(temperature).c_str());
    client.publish("iotproject59283273_esp32/humidity", String(humidity).c_str());
    client.publish("iotproject59283273_esp32/pressure", String(pressure).c_str());
    client.publish("iotproject59283273_esp32/israining", String(isRaining).c_str());
}

  delay(2000); // Delay for 2 seconds before taking readings again
}
