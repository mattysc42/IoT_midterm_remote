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
const int MP3BUSYPIN = D15;
const int MP3TXPIN = D4;
const int MP3RXPIN = D5;
const int BUTTONPINSTARTSTOP = D3;
const int BUTTONPINPREVIOUSSONG = A1;
const int BUTTONPINNEXTSONG = A2;
unsigned int currentTrack = 1;
bool toggleStartStop;
bool togglePlaylist;
DFRobotDFPlayerMini mp3Player;

Button buttonStartStop(BUTTONPINSTARTSTOP);
Button mp3NextButton(BUTTONPINNEXTSONG);
Button mp3BackButton(BUTTONPINPREVIOUSSONG);

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
const int ENCODERSWITCHRED = D17;
const int ENCODERSWITCHGREEN = D18;
const int ENCODERSWITCHBLUE = D14;
int encoderInput;
float previousEncoderInput;
bool encoderSwitchToggle;
Encoder encoder(ENCODERPINA, ENCODERPINB);
Button encoderButton(ENCODERSWITCHPIN); // used to switch playlists.

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
// Linear conversion HueBulb brightness
float x1EncoderLow = 0.0;
float y1BrightnessLow = 0.0;
float x2EncoderHigh = 96.0;
float y2BrightnessHigh = 255.0;
float slopeHueBulb;
float yInterceptHueBulb;
float mappedEncoderToBrightness;
float previousInputHueBulb;

// Linear conversion volume
float y1VolumeLow = 0.0;
float y2VolumeHigh = 30.0;
float slopeVolume;
float yInterceptVolume;
float mappedEncoderToVolume;
float previousInputVolume;

// Linear conversion pixels
float y1PixelLow = 0.0;
float y2PixelHigh = 12.0;
float slopePixel;
float yInterceptPixel;
float mappedEncoderToPixel;
float previousInputPixel;

// Timer variables and objects
int currentTime;
int previousTime;

// Function Prototypes
void checkEncoderPositionZero();
void pixelFill(int startPixel, int endPixel, int red, int green, int blue);
float celToFar(float inputTempCel);

#endif