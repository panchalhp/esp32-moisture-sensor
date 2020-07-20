#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define NUM_SENSORS 6 // Number of moisture sensors currently hooked up
#define DELAY_MS 4000 // Number of ms delay between reading different sensors

#define DEBUG true
char debugBuffer[100];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// define structs to make it easier to read multiple moisture inputs
struct MoistureSensor {
  char plantName[20];
  bool shouldDisplay;
  int pinNumber;
  int airValue;
  int waterValue;
};

// define the moisture sensors here
MoistureSensor sensors[NUM_SENSORS] = {
  {"Sensor1", true, 36, 2560, 1140},
  {"Sensor2", true, 39, 3330, 1700},
  {"Sensor3", true, 34, 3330, 1700},
  {"Sensor4", true, 35, 3200, 1630},
  {"Sensor5", true, 32, 3250, 1650},
  {"Sensor6", true, 33, 3220, 1640}
};
int moisturePct[NUM_SENSORS];


/*
   reads the value on the sensor and returns the calibrated percentage
*/
int readMoisturePercentage(MoistureSensor sensor) {
  int moistureVal = analogRead(sensor.pinNumber);
  int moisturePct = map(moistureVal, sensor.airValue, sensor.waterValue, 0, 100);
  if (DEBUG) {
    sprintf(debugBuffer,  "%s on pin(%d): raw=%d pct=%d",
            sensor.plantName, sensor.pinNumber, moistureVal, moisturePct);
    Serial.println(debugBuffer);
  }
  return moisturePct;
}

/*
   Displays the value of the read sensor on the OLED screen
*/
void displayOnScreen(int moisturePct, char plantName[]) {
  char oledPctBuffer[6];
  const char *pctSign = " %";
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


void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
}


void loop()
{
  // read value from pins one by one
  for (int idx=0; idx < NUM_SENSORS; idx++) {
    moisturePct[idx] = readMoisturePercentage(sensors[idx]);
    if (sensors[idx].shouldDisplay) {
      displayOnScreen(moisturePct[idx], sensors[idx].plantName);
    }

    delay(DELAY_MS);
    display.clearDisplay();
  }
}
