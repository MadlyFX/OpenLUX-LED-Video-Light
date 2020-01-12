//Code for OpenLUX LED Video Light
//Written by Andy Jarosz

//The code for the Thermistor will almost certainly change depending on the one you use. Be sure to look up the values for your thermistor, or the temp readings will be thrown off. 

#include <AceButton.h>
using namespace ace_button;

// The pin number attached to the button.
const int BUTTON_PIN = 7;

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Encoder.h>

bool powerSelect = true;

bool turboMode = false;

AceButton button(BUTTON_PIN);
void handleEvent(AceButton*, uint8_t, uint8_t);

// which analog pin to connect
#define THERMISTORPIN A7
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000
float steinhart;
// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        8
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         10

float value2700;
float value6500;

int currentEncoderPos;

// For 1.3", 1.54", and 2.0" TFT with ST7789:
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

Encoder myEnc(4, 6);

float p = 3.1415926;
long oldPosition  = -999;

int power = 100;
int temp = 2700;

float powerMultiplier;

#define MENU1Y 60
#define MENU1WIDTH 240
#define MENU1HEIGHT 90

#define MENU2Y 150
#define MENU2WIDTH 240
#define MENU2HEIGHT 90

#define MENU1TEXTY 110
#define MENU2TEXTY 200



void setup(void) {
  analogWriteFrequency(23, 8000);
  analogWriteFrequency(22, 8000);
  Serial.begin(9600);
  pinMode(7, INPUT_PULLUP); //Encoder Pullup
  tft.setFont(&FreeSans24pt7b);
  Serial.print(F("Hello! ST77xx TFT Test"));


  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 320x240

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;


  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);



  tft.fillRect(0, 0, 240, 250, ST77XX_RED);

  tft.setCursor(60, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);
  tft.print("2700K");

  tft.setCursor(60, 200);
  tft.print("100%");

  tft.drawRect(0, MENU1Y, MENU1WIDTH, MENU1HEIGHT, ST77XX_WHITE);

  writeTop();

}

void loop() {

  button.check();

  if (powerSelect == true) {

    if (myEnc.read() > 100) {
      setEnc(100);
    }

    if (myEnc.read() < 0) {
      setEnc(0);
    }

    tft.drawRect(0, 0, 240, 120, ST77XX_RED);
    tft.drawRect(0, MENU2Y, MENU2WIDTH, MENU2HEIGHT, ST77XX_WHITE);
    long newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      tft.setFont(&FreeSans24pt7b);
      tft.setTextColor(ST77XX_WHITE);
      power = constrain(newPosition, 0, 100);
      tft.fillRect(0, 120, 240, 120, ST77XX_RED);
      tft.setCursor(60, MENU2TEXTY);
      tft.print(power);
      tft.print("%");


      tft.fillRect(0, 0, 240, 120, ST77XX_RED);
      if (turboMode == true) {
        tft.setCursor(10, MENU1TEXTY);
        tft.print("~");
      }
      tft.setCursor(60, MENU1TEXTY);
      tft.print(temp);
      tft.print("K");
      writeTop();
    }
  }

  if (powerSelect == false) { //Temp select

    if (myEnc.read() > 6500) {
      setEnc(6500);
    }

    if (myEnc.read() < 2700) {
      setEnc(2700);
    }

    tft.drawRect(0, 120, 240, 120, ST77XX_RED);
    tft.drawRect(0, MENU1Y, MENU1WIDTH, MENU1HEIGHT, ST77XX_WHITE);
    long newPosition = myEnc.read();
    if (newPosition != oldPosition) {

      if (newPosition > oldPosition) {
        temp = temp += 100;
      } else {
        temp = temp -= 100;
      }
      oldPosition = newPosition;
      tft.setFont(&FreeSans24pt7b);
      tft.setTextColor(ST77XX_WHITE);
      // temp = constrain(newPosition, 2700, 6500);


      tft.fillRect(0, 0, 240, 120, ST77XX_RED);
      if (turboMode == true) {
        tft.setCursor(10, MENU1TEXTY);
        tft.print("~");
      }
      tft.setCursor(60, MENU1TEXTY);

      tft.print(temp);
      tft.print("K");

      tft.setCursor(60, MENU2TEXTY);
      tft.print(power);
      tft.print("%");

      writeTop();

    }


  }

  if (turboMode == false) { //Linear dimming mode, standard
    powerMultiplier = map(power, 0, 100, 0, 255); //Convert the 0-100 human value to a 0-255 8-bit value
    value2700 = map(temp, 2700, 6500, 0, powerMultiplier); //Find the % of that value between 2700 and 6500
    value6500 =  map(temp, 2700, 6500, powerMultiplier, 0); //Find the % of that value between 6500 and 2500

    analogWrite(22, value2700);
    analogWrite(23, value6500);
  } else { //Turbo Mode
    powerMultiplier = map(power, 0, 100, 0, 255); 
    value6500 =  map(temp, 2700, 6500, powerMultiplier, 0);
    value2700 = map(temp, 2700, 6500, 0, powerMultiplier);
    analogWrite(22, value2700 * 2);
    analogWrite(23, value6500 * 2);
  }

  //Serial.println(value2700);
  //Serial.println(value6500);
      float average = analogRead(THERMISTORPIN);

      average = 1023 / average - 1;
      average = SERIESRESISTOR * average;
      Serial.print("Thermistor resistance "); 
      Serial.println(average);
      
      
      steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
      steinhart = log(steinhart);                  // ln(R/Ro)
      steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
      steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
      steinhart = 1.0 / steinhart;                 // Invert
      steinhart -= 273.15;                         // convert to C
      
      Serial.print("Temperature "); 
      Serial.print(steinhart);
      Serial.println(" *C");


  if (millis() % 5000 == 0 ) { //Every 5 seconds, update top and check the temp
    writeTop();
    checkTemp();
  }
}


void setEnc(int num) {
  myEnc.write(num);
}

void writeTop() { //Write the top statusbar section
  tft.setCursor(0, 20);
  tft.fillRect(0, 0, 240, 55, ST77XX_WHITE);
  tft.setTextColor(ST77XX_RED);
  tft.setFont(&FreeMonoBoldOblique12pt7b);
  float voltage = analogRead(10) * (3.3 /1023.0);
  tft.print(voltage * 9);
  tft.print("V");
  tft.setCursor(0, 45);
  tft.print("Temp: ");
  tft.print(round(steinhart / 100 * 100));
  tft.print("%");

  if (turboMode == false) {
    tft.setCursor(150, 20);
    tft.print("Linear");
    tft.setFont(&FreeSans24pt7b);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, MENU1TEXTY);
    tft.print("  ");
  } else {
    tft.setCursor(150, 20);
    tft.print("Turbo");
    tft.setFont(&FreeSans24pt7b);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, MENU1TEXTY);
    tft.print("~");
  }
}

void checkTemp() {

  if (steinhart > 80) {
    tft.fillRect(0, 0, 240, 240, ST77XX_RED);
    tft.setFont(&FreeSans24pt7b);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(20, 40);
    tft.setFont(&FreeMonoBoldOblique12pt7b);
    tft.print("OVERTEMP");

    while (1) { //Intentional endless loop crash, turn off the LEDs forever
    analogWrite(22, 0);
    analogWrite(23, 0);
    }
  }
}

void handleEvent(AceButton* /* button */, uint8_t eventType,
                 uint8_t buttonState) {

  // Control the LED only for the Pressed and Released events.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventClicked:
      powerSelect = !powerSelect;
      tft.fillRect(0, 0, 240, 240, ST77XX_RED);
      if (powerSelect == true) {
        setEnc(power);
      } else {
        setEnc(temp);
      }
      break;
    case AceButton::kEventLongPressed:
      turboMode = !turboMode;
      writeTop();
      break;
  }


}
