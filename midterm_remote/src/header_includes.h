#ifndef _HEADER_INCLUDES_H_
#define _HEADER_INCLUDES_H_

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
int playlistQuiet[10];
int playlistLoud[10];
int playlistIndexQuiet; // playlist indexes to cycle through with a for loop.
int playlistIndexLoud;
float currentTrackTime; 
float totalTrackTime; // If current track time == total track time, go to the next track.

// Hue Bulb and Wemo constants, variable, objects
const int BULB1 = 1;
const int MYWEMO = 3;
int color, previousColor; // match these to the mapped temperature.

// Encoder constants, variable, objects
const int ENCODERPINA = D9;
const int ENCODERPINB = D8;
const int ENCODERSWITCHPIN = D19;
float encoderInput;
float previousEncoderInput;
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
float y2VolumeHigh = 100.0;
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

#endif