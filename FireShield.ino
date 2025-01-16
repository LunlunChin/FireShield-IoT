#include "VOneMqttClient.h"
#include "DHT.h" // Include the DHT library
#define timeSeconds 3

float gasValue;
float temperature;
float humidity;

// Define device IDs
const char* MQ2sensor = "351ca33b-2d85-4916-80d4-5e577c519348"; 
const char* DHT11sensor = "edfa8a2c-980a-465c-98e7-05ecfdda8091";   
const char* PIRsensor = "cd3355de-d71d-4b41-8f6a-c18b9692e2e0";       

// Used Pins
const int MQ2pin = A0;      // Middle Maker Port for MQ2
const int dht11Pin = 21;
const int motionSensor = 4;      //Left side Maker Port

#define DHTTYPE DHT11 

// Input sensors
DHT dht(dht11Pin, DHTTYPE);

// Create an instance of VOneMqttClient
VOneMqttClient voneClient;

// Last message time
unsigned long lastMsgTime = 0;
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;


void IRAM_ATTR detectsMovement() {
  startTimer = true;
  lastTrigger = millis();
}


void setup_wifi() {
  delay(10);
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  setup_wifi();
  voneClient.setup();
  
  Serial.println("Initializing sensors...");
  
  // Initialize the DHT sensor
  dht.begin();

  // Warm up the MQ2 sensor
  Serial.println("Gas sensor warming up!");
  pinMode(motionSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  delay(10000); // Allow the MQ2 to warm up
}

void loop() {
  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "MQ2 Sensor Fail";
    voneClient.publishDeviceStatusEvent(MQ2sensor, true);
    voneClient.publishDeviceStatusEvent(DHT11sensor, true);
    voneClient.publishDeviceStatusEvent(PIRsensor,true);

  }
  voneClient.loop();

  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    // Read and publish MQ2 sensor data
    gasValue = analogRead(MQ2pin);
    voneClient.publishTelemetryData(MQ2sensor, "Gas detector", gasValue);

    // Read and publish DHT11 sensor data
    float h = dht.readHumidity();
    int t = dht.readTemperature();

    JSONVar payloadObject;    
    payloadObject["Humidity"] = h;
    payloadObject["Temperature"] = t;
    voneClient.publishTelemetryData(DHT11sensor, payloadObject);

    // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    startTimer = false;
  }
    voneClient.publishTelemetryData(PIRsensor,"Motion", startTimer);
  }
}
