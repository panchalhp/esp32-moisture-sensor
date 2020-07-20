#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/* -------------------------- GLOBAL CONSTANTS -------------------------- */
#define NUM_SENSORS 6 // Number of moisture sensors currently hooked up

#define DELAY_MS 4000 // Number of ms delay between reading different sensors
#define SERVER_SEND_DELAY_MS 60000 // Number of ms delay between sending data to server

#define SEND_TO_SERVER false
#define DEBUG true

/* ---------------------------------------------------------------------- */


// define structs to make it easier to read multiple moisture inputs
struct MoistureSensor {
  char plantName[20];
  int pinNumber;
  int airValue;
  int waterValue;
};


/* -------------------------- GLOBAL VARIABLES -------------------------- */
const char* ssid = "<YOUR-SSID>";
const char* password = "<YOUR-PASSWORD>";

const char* sensorServerURL = "http://<YOUR-COMPUTER-IP>/update-sensor";

unsigned long lastSentTimestampMs = 0;
char debugBuffer[100];
char jsonBuffer[600];

int moistureValuesPct[NUM_SENSORS]; // to display current reading
int moistureValuesPctTotal[NUM_SENSORS] = {0, 0, 0, 0, 0, 0}; // to calculate avg reading
int numReadings = 0;

// define the moisture sensors here
MoistureSensor sensors[NUM_SENSORS] = {
  {"Fern", 36, 2560, 1140},     // sensor 1
  {"Foliage", 39, 3330, 1700},  // sensor 2
  {"Pothos", 34, 3330, 1700},   // sensor 3
  {"Dumbcane", 35, 3200, 1630}, // sensor 4
  {"Dracaena", 32, 3250, 1650}, // sensor 5
  {"Umbrella", 33, 3220, 1640}  // sensor 6
};


/* ---------------------------------------------------------------------- */

/*
   Reads the value on the sensor and returns the calibrated percentage
*/
int readMoisturePercentage(MoistureSensor sensor) {
  int moistureVal = analogRead(sensor.pinNumber);
  int moisturePct = map(moistureVal, sensor.airValue, sensor.waterValue, 0, 100);

  int moisturePctCapped = -1;
  if (moisturePct <= 0) {
    moisturePctCapped = 0;
  } else if (moisturePct >= 100) {
    moisturePctCapped = 100;
  } else {
    moisturePctCapped = moisturePct;
  }

  if (DEBUG) {
    sprintf(debugBuffer,  "%s on pin(%d): raw=%d pct=%d",
            sensor.plantName, sensor.pinNumber, moistureVal, moisturePctCapped);
    Serial.println(debugBuffer);
  }
  return moisturePctCapped;
}


/*
   Displays the value of the read sensor on the OLED screen
*/
void displayReadingOnScreen(int moisturePct, char plantName[]) {
  char oledPctBuffer[6];
  const char *pctSign = " %";

  sprintf(oledPctBuffer, "%d", moisturePct);
  strcat(oledPctBuffer, pctSign);

  display.setCursor(10, 0); // oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(plantName);
  display.setCursor(10, 18);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");

  display.setCursor(10, 40); // oled display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println(oledPctBuffer);
  display.display();
}


/*
   Displays the status of HTTP request on OLED screen
*/
void displayHTTPStatusOnScreen(char msg[]) {
  display.clearDisplay();

  display.setCursor(10, 0); // oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Sending");

  display.setCursor(10, 20);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Data");

  display.setCursor(10, 40); // oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(msg);

  display.display();
}


/*
   Populates the JSON buffer with sensor and reading information
*/
void populateJsonBuffer() {
  char jsonNestedBuffer[NUM_SENSORS][100];
  for (int i = 0; i < NUM_SENSORS; i++) {
    sprintf(
      jsonNestedBuffer[i],
      "{\"plantName\":\"%s\", \"sensorPinNumber\":%d, \"moistureValue\":%.2f}",
      sensors[i].plantName,
      sensors[i].pinNumber,
      (moistureValuesPctTotal[i] * 1.0 / numReadings) // take the avg of all readings
    );
  }

  char valueBuffer[NUM_SENSORS * 100];
  sprintf(valueBuffer, "");
  for (int i = 0; i < NUM_SENSORS; i++) {
    strcat(valueBuffer, jsonNestedBuffer[i]);
    if (i < NUM_SENSORS - 1) {
      strcat(valueBuffer, ",");
    }
  }

  sprintf(jsonBuffer, "{\"timestamp\":\"%d\",\"values\":[%s]}", millis(), valueBuffer);
}


/*
   POSTs the sensor readings to the server by sending an HTTP request
*/
void sendToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    displayHTTPStatusOnScreen("......");
    Serial.println("WiFi connected, sending data to server..");

    populateJsonBuffer();

    HTTPClient http;
    http.begin(sensorServerURL);
    http.addHeader("Content-Type", "application/json");
    int responseCode = http.POST(jsonBuffer);

    if (responseCode == 200) {
      Serial.println("Sending data to server SUCCESS");
      displayHTTPStatusOnScreen("Success!");
    } else {
      Serial.println("Sending data to server FAILED");
      displayHTTPStatusOnScreen("Failed :(");
    }
  } else {
    Serial.println("WiFi not connected, cannot sending data!!");
    displayHTTPStatusOnScreen("No WiFi!!");
  }
}


/* --------------------------- MAIN --------------------------- */
void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();

  if(SEND_TO_SERVER) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }
}


void loop()
{
  // read value from pins one by one
  for (int i = 0; i < NUM_SENSORS; i++) {
    moistureValuesPct[i] = readMoisturePercentage(sensors[i]);
    moistureValuesPctTotal[i] += moistureValuesPct[i];
    displayReadingOnScreen(moistureValuesPct[i], sensors[i].plantName);

    delay(DELAY_MS);
    display.clearDisplay();
  }

  numReadings += 1;

  if (SEND_TO_SERVER &&
      (lastSentTimestampMs == 0 || (millis() - lastSentTimestampMs) >= SERVER_SEND_DELAY_MS)) {
    sendToServer();

    // variable bookkeeping
    lastSentTimestampMs = millis();
    numReadings = 0;
    for (int i = 0; i < NUM_SENSORS; i++) {
      moistureValuesPctTotal[i] = 0;
    }

    delay(1000);
    display.clearDisplay();
  }
}
