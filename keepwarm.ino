// Pigu-warmer 

#include <ESP8266WiFi.h>
#include <TroykaDHT.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

// Define the GPIO pins.
#define relayPin 12
#define blueLed 13
#define tempPin 14
#define buttonPin 1

// Define the temperature range.
#define onTemp 20
#define offTemp 22

// Set the DHT type and define DHT.
#define dhtType DHT21
DHT dht = DHT(tempPin, dhtType);

// Define HTTP server variables.
ESP8266WebServer server(80);

float temp = 0;
float humi = 0;
bool state = 0;

void setup() {
  // Set up the GPIO pins.
  pinMode(relayPin, OUTPUT);
  pinMode(blueLed, OUTPUT);

  // Start serial
  Serial.begin(9600);

  // Start DHT
  dht.begin();

  // Connect to WiFi
  WiFi.begin("ssid", "password");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(blueLed, 0);
    delay(500);
    digitalWrite(blueLed, 1);
    delay(500);
    
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(blueLed, 0);

  server.on("/", handleRoot);
  server.on("/temp", handleTemp);
  server.on("/humi", handleHumi);
  server.on("/state", handleState);
  server.begin();
}

void loop() {
  // Read from the sensor
  dht.read();

  switch(dht.getState()) {
   case DHT_OK:
     temp = dht.getTemperatureC();
     humi = dht.getHumidity();
     Serial.println(temp);
     Serial.println(humi);
     break;
       
     case DHT_ERROR_CHECKSUM:
       Serial.println("Checksum error");
       break;
       
     case DHT_ERROR_TIMEOUT:
       Serial.println("Time out error");
       break;
       
     case DHT_ERROR_NO_REPLY:
       Serial.println("Sensor not connected");
       break;
   }

   // Aim to keep the cage between 18 and 21 degrees
   if(temp < onTemp){
    digitalWrite(relayPin, 1);
    state = 1;
   }
   else if(temp > offTemp){
    digitalWrite(relayPin, 0);
    state = 0;
   }
   
   server.handleClient();
}

void handleRoot() {

  String stateText;
  
  if (state == 1){
    stateText = "On";
  }

  else if (state == 0){
    stateText = "Off";
  }
  
  String response = "<html><head><title>Patch Temperature</title></head><body><p><h3>Current temperature: </h3>" + String(temp) + "&#176C</p><p><h3>Current humidity: </h3> " + String(humi) + "%</p><p><h3>Heater is: </h3> " + String(stateText) + "</p></body></html>";
  
  server.send(200, "text/html", response);  
}

void handleTemp() {
  server.send(200, "text/plain", String(temp));
}

void handleHumi() {
  server.send(200, "text/plain", String(humi));
}

void handleState() {
  server.send(200, "text/plain", String(state));
}
