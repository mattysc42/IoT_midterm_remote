/* 
 * Project midterm_remote
 * Author: Matthew Call
 * Date: 7/9/24
 */

#include "header_includes.h"

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

if (( currentTime - previousTime ) > 1000) {
        tempFar = bmeSensor.readTemperature();

        tempFar = celToFar(tempCel);
        Serial.printf("\nTemp in Fahrenheit is: %0.2f.\n", tempFar);

        // light minipixels based on the MBE readings while keeping the values inside the analog output limits.
        color = tempFar;
        if((color * 2) >= 254) {
            color = 127;
        }
        if((color * 2) <= 1) {
            color = 1;
        }

        // maps volume percentage and lights up an equal percentage of neopixels. (Still needs to be done)
        slopeVolume = findSlope(0, 0, 100, 12);
        yInterceptVolume = findYInstercept(slopeVolume, 0, 0);
        mappedEncoderToVolume = findLinearConversion(slopeVolume, yInterceptVolume, encoderInput);

        pixelCount = mappedEncoderToVolume;

        // sets color based on temperature. Pure blue at 32 degrees farenheight, pure red at 100 degrees farenheight.
        pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
        pixel.show();
        if(tempFar < previousTempFar) {
            pixel.clear();
            pixel.show();
            pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
        }

        // prints data to the OLED display
        myOLED.setCursor(0, 0);
        myOLED.printf("---\nTemp: %0.2f F. \n---\n", tempFar);
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