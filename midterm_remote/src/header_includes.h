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
const int BUTTONPINSTARTSTOP = D10;
bool toggleStartStop = 1;
bool togglePlaylist;
unsigned int currentFolder, currentTrack;
DFRobotDFPlayerMini mp3Player;
Button buttonStartStop(BUTTONPINSTARTSTOP);

// Hue Bulb and Wemo constants, variable, objects
const int BULB1 = 1;
const int BULBALL[] = {1, 2, 3, 4, 5, 6};
// lava lamp
const int MYWEMO = 0;
// desk outlet
const int MYWEMO2 = 1;
// fan
const int MYWEMO3 = 2;
bool wemoToggleState = true;
int color, previousColor, playlistColor;

// Encoder constants, variable, objects
const int ENCODERPINA = D8;
const int ENCODERPINB = D9;
const int ENCODERSWITCHPIN = D7;
const int ENCODERSWITCHRED = D16;
const int ENCODERSWITCHGREEN = D6;
const int ENCODERSWITCHBLUE = D14;
bool encoderSwitchToggle;
int encoderInput = 1;
float previousEncoderInput;
Encoder encoder(ENCODERPINA, ENCODERPINB); // used to control volume and neopixel brightness
Button encoderButton(ENCODERSWITCHPIN); // used to switch playlists.

// Ultrasonic Sensor constants, variable, objects
const int TRIGPIN = D18;
const int ECHOPIN = D11;
float duration, distance; 
bool toggleProximityStart;

// BME/OLED constants, variable, objects
const int SDAPIN = D0;
const int SCLPIN = D1;
const int OLED_RESET = -1;
byte sensorAddress = 0x76;
float tempCel, tempFar, previousTempFar;
Adafruit_SSD1306 myOLED(OLED_RESET);
Adafruit_BME280 bmeSensor;

// Neopixel constants, variable, objects
int pixelCount = 12;
Adafruit_NeoPixel pixel(pixelCount, SPI1, WS2812B);

// Linear conversion variables
// Linear conversion HueBulb and neopixel brightness
float x1EncoderLow = 0.0;
float y1BrightnessLow = 0.0;
float x2EncoderHigh = 96.0;
float y2BrightnessHigh = 255.0;
float slopeHueBulb, yInterceptHueBulb, mappedEncoderToBrightness, previousEncoderToBrightness;

// Linear conversion volume
float y1VolumeLow = 0.0;
float y2VolumeHigh = 30.0;
float slopeVolume, yInterceptVolume, mappedEncoderToVolume, previousInputVolume;

// Linear conversion pixels
float y1PixelLow = 0.0;
float y2PixelHigh = 12.0;
float slopePixel, yInterceptPixel, mappedEncoderToPixel, previousInputPixel;

// Timer variables and objects
int currentTime, previousTime, previousTime2, previousTimeSensor;

// Function Prototypes
void checkEncoderPositionZero();
void pixelFill(int startPixel, int endPixel, int red, int green, int blue);
float celToFar(float inputTempCel);
void cycleHueBulbs(int playlistColor, float brightness);

#endif