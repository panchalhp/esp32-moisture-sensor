#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayOnScreen(int moisturePct, char plantName[]) {
  char oledPctBuffer[5];
  char *pctSign = " %";
  
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


void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
}


void loop()
{
  displayOnScreen(35, "Pothos");
  delay(4000);
  display.clearDisplay();
  
  displayOnScreen(40, "Fern");
  delay(4000);
  display.clearDisplay();
  
  displayOnScreen(70, "Dracaena");
  delay(4000);
  display.clearDisplay();

  displayOnScreen(75, "Umbrella");
  delay(4000);
  display.clearDisplay();
}
