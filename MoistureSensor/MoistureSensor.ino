#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define NUM_SENSORS 1 // Number of moisture sensors currently hooked up

#define DEBUG true
char debugBuffer[100];

const char *pctSign = " %";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// define structs to make it easier to read multiple moisture inputs
struct MoistureSensor {
  char plantName[20];
  bool shouldDisplay;
  int pin;
  int airValue;
  int waterValue;
};

// define the moisture sensors here
MoistureSensor sensors[NUM_SENSORS] = {
  {"Pothos", true, 4, 2640, 1140}
};
int moisturePct[NUM_SENSORS];


/*
   reads the value on the sensor and returns the calibrated percentage
*/
int readMoisturePercentage(MoistureSensor sensor) {
  int moistureVal = analogRead(sensor.pin);
  int moisturePct = map(moistureVal, sensor.airValue, sensor.waterValue, 0, 100);
  if (DEBUG) {
    sprintf(debugBuffer,  "%s on pin(%d): raw=%d pct=%d",
            sensor.plantName, sensor.pin, moistureVal, moisturePct);
    Serial.println(debugBuffer);
  }
  return moisturePct;
}

/*
   Displays the value of the read sensor on the OLED screen
*/
void displayOnScreen(int moisturePct, char plantName[]) {
  char oledPctBuffer[5];
  int displayPct = -1;
  
  if (moisturePct <= 0) {
    displayPct = 0;
  } else if (moisturePct >= 100) {
    displayPct = 100;
  } else {
    displayPct = moisturePct;
  }
  sprintf(oledPctBuffer, "%d", displayPct);
  strcat(oledPctBuffer, pctSign);

  display.setCursor(45, 0); // oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(plantName);
  display.setCursor(20, 15);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");

  display.setCursor(30, 40); // oled display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println(oledPctBuffer);
  display.display();
}


void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
}


void loop()
{
  // read value from pins
  int numSensors = sizeof(sensors) / sizeof(sensors[0]);

  for (int idx=0; idx < numSensors; idx++) {
    moisturePct[idx] = readMoisturePercentage(sensors[idx]);
    if (sensors[idx].shouldDisplay) {
      displayOnScreen(moisturePct[idx], sensors[idx].plantName);
    }
  }
  
  delay(250);
  display.clearDisplay();
}
