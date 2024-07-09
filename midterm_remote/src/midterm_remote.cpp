/* 
 * Project midterm_remote
 * Author: Matthew Call
 * Date: 7/9/24
 */

#include "Particle.h"
#include "Adafruit_BME280.h"
#include "Adafruit_SSD1306.h"
#include "neopixel.h"
#include "linear_conversion.h"
#include "IoTClassroom_CNM.h"
#include "DFRobotDFPlayerMini.h"
#include "Encoder.h"

// MP3 player constants, variable, objects
const int NEXTBUTTONPIN = A1;
const int BACKBUTTONPIN = A2;
const int PLAYBUTTONPIN = D15;

// Hue Bulb and Wemo constants, variable, objects
const int BULB1 = 1;
const int MYWEMO = 0;
int color, previousColor;

// Encoder constants, variable, objects
const int ENCODERPINA = D9;
const int ENCODERPINB = D8;
const int ENCODERSWITCHPIN = D19;
Encoder encoder(ENCODERPINA, ENCODERPINB);

// Ultrasonic Sensor constants, variable, objects
const int TRIGPIN = D16;
const int ECHOPIN = A0;

// BME/OLED constants, variable, objects
const int SDAPIN = A0;
const int SCLPIN = A4;
const int OLED_RESET = -1;
byte sensorAddress = 0x76;
float tempCel;
float tempFar;
int tempColor;
float previousTempFar;

Adafruit_SSD1306 myOLED(OLED_RESET);
Adafruit_BME280 bmeSensor;



// Neopixel constants, variable, objects
int pixelCount = 12;
Adafruit_NeoPixel pixel(pixelCount, SPI1, WS2812B);


// Linear conversion variables
// HueBulb
float x1EncoderLow = 0.0;
float y1HueBulbLow = 0.0;
float x2EncoderHigh = 96.0;
float y2HueBulbHigh = 255.0;
float slopeHueBulb;
float yInterceptHueBulb;
float mappedEncoderToHueBulb;
float previousInputHueBulb;

// volume
float y1VolumeLow = 0.0;
float y2VolumeHigh = 255.0;
float slopeVolume;
float yInterceptVolume;
float mappedEncoderToVolume;
float previousInputVolume;

// Timer variables and objects
int currentTime;
int previousTime;


// Button variables and objects
const int BUTTONPINPOWERALL = D3;
const int BUTTONPINBULBS = D6;
const int BUTTONPINWEMOS = D7;
bool buttonPowerToggle;
bool previousPowerToggle;
bool previousBulbInput = 0;
Button buttonPowerAll(BUTTONPINPOWERALL);
Button buttonPowerBulb(BUTTONPINBULBS);
Button buttonPowerWemo(BUTTONPINWEMOS);
Button encoderButton(ENCODERSWITCHPIN);
Button mp3NextButton(NEXTBUTTONPIN);
Button mp3BackButton(BACKBUTTONPIN);


// Function Prototypes
void pixelFill(int startPixel, int endPixel, int red, int green, int blue);
float celToFar(float inputTempCel);

SYSTEM_MODE(MANUAL);

// SYSTEM_THREAD(ENABLED);

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected,10000);
    WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork");
    WiFi.connect();
    //while(WiFi.connecting()) {
        //Serial.printf(".");
    //}
    Serial.printf("\n\n");
}

void loop() {
    currentTime = millis();
    myOLED.clearDisplay();

if (( currentTime - previousTime ) > 500) {
        tempFar = bmeSensor.readTemperature();

        tempFar = celToFar(tempCel);
        Serial.printf("\nTemp in Fahrenheit is: %0.2f.\nPressure in inches of mercury is: %0.2f.\nRelative humidity is: %0.2f.\n", tempFar);

        // light minipixels based on the MBE readings while keeping the values inside the analog output limits.
        tempColor = tempFar;
        if((tempColor * 2) >= 254) {
            tempColor = 127;
        }
        if((tempColor * 2) <= 1) {
            tempColor = 1;
        }

        // maps volume percentage and lights up an equal percentage of minipixels.
        slopeHumidity = findSlope(0, y1NoPixels, 100, y2AllPixels);
        yIntercept = findYInstercept(slopeHumidity, 0, y1NoPixels);
        mappedHumidity = findLinearConversion(slopeHumidity, yIntercept, airWater);

        pixelCount = mappedHumidity;

        // sets color based on temperature. Pure blue at 32 degrees farenheight, pure red at 100 degrees farenheight.
        pixelFill(0, pixelCount, (tempColor * 2) - 64, 0, 200 - (tempColor * 2));
        pixel.show();
        if(tempFar < previousTempFar) {
            pixel.clear();
            pixel.show();
            pixelFill(0, pixelCount, (tempColor * 2) - 64, 0, 200 - (tempColor * 2));
        }

        // prints data to the OLED display
        myOLED.setCursor(0, 0);
        myOLED.printf("---\nTemp: %0.2f F\n---\nPressure: %0.2f inHg\n---\nHumidity: %0.2f %%\n---\n", tempFar);
        myOLED.display();
        previousTime = currentTime;
        previousTempFar = tempFar;
    }
}

// Functions
// converts Celsius to Fahrenheit
float celToFar(float inputTempCel) {
    float outputTempFar = (inputTempCel * 1.8) + 32.0;
    return outputTempFar;
}

// pixelFill function for lighting up a changable quantity of mini pixels.
void pixelFill(int startPixel, int endPixel, int red, int green, int blue) {
    for(int i = startPixel; i <= endPixel; i++) {
        pixel.setPixelColor(i, red, green ,blue);
        pixel.show();
    }
}