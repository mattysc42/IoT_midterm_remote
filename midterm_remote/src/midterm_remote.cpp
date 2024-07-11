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
    waitFor(Serial.isConnected, 2500);
    WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork");
    WiFi.connect();
    while(WiFi.connecting()) {
        Serial.printf(".");
    }
    
    // encloder switch RGB
    pinMode(ENCODERSWITCHRED, OUTPUT);
    pinMode(ENCODERSWITCHGREEN, OUTPUT);
    pinMode(ENCODERSWITCHBLUE, OUTPUT);

    // pixel stuff
    pixel.begin();
    pixel.clear();
    pixel.show();

    // OLED stuff
    myOLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    myOLED.clearDisplay();
    myOLED.setTextSize(1);
    myOLED.setTextColor(1, 0);
    myOLED.setCursor(0, 0);
    myOLED.printf("BME Monitor\n------------\n");
    myOLED.display();
    delay(2000);

    // BME stuff.
    bool status = bmeSensor.begin(sensorAddress);
    if(status == false) {
        Serial.printf("BME280 at address %02X failed to start", sensorAddress);
    }

    // MP3 Player stuff.
    pinMode(MP3BUSYPIN, INPUT);

    Serial.printf("DFRobot DFPlayer Mini Demo\n");
    Serial.printf("Initializing DFPlayer ... (May take 3~5 seconds)\n");
    Serial2.begin(9600);
    mp3Player.begin(Serial2);
    delay(1000);
    //mp3Player.EQ(DFPLAYER_EQ_NORMAL);
    //mp3Player.outputDevice(DFPLAYER_DEVICE_SD);
    if (!mp3Player.begin(Serial2)) {  //Use softwareSerial to communicate with mp3.
        Serial.printf("Unable to begin:\n");
        Serial.printf("1.Please recheck the connection!\n");
        Serial.printf("2.Please insert the SD card!\n");
        while(true);
    }
    mp3Player.loopFolder(1);
    Serial.printf("playing quiet playlist.");
}

void loop() {
    myOLED.clearDisplay();
    encoderInput = encoder.read();
    
    // switch playlists, encoder switch LED color, and huebulb color if the encoder switch is clicked.
    if(encoderButton.isClicked()) {
        encoderSwitchToggle = !encoderSwitchToggle;
        if(!encoderSwitchToggle) {
            mp3Player.loopFolder(1);
            Serial.printf("playing quiet playlist.");
        }
        else {
            mp3Player.loopFolder(2);
            Serial.printf("playing loud playlist.");
        } 
    } 

    if(encoderSwitchToggle == true) {
        digitalWrite(ENCODERSWITCHRED, LOW);
        digitalWrite(ENCODERSWITCHGREEN, HIGH);
        digitalWrite(ENCODERSWITCHBLUE, HIGH);
    }
    else {
        digitalWrite(ENCODERSWITCHRED, HIGH);
        digitalWrite(ENCODERSWITCHGREEN, HIGH);
        digitalWrite(ENCODERSWITCHBLUE, LOW);
    }

    if(encoderInput <= 0) {
        encoderInput = 0;
        encoder.write(0);
    }
    else if(encoderInput >= 96) {
        encoderInput = 96;
        encoder.write(96);
    }

    // light minipixels based on the MBE readings while keeping the values inside the analog output limits.
    color = tempFar;
    if((color * 2) >= 254) {
        color = 127;
    }
    if((color * 2) <= 1) {
        color = 1;
    }
    

    // maps volume percentage to encoder.
    slopeVolume = findSlope(x1EncoderLow, y1VolumeLow, x2EncoderHigh, y2VolumeHigh);
    yInterceptVolume = findYInstercept(slopeVolume, x1EncoderLow, y1VolumeLow);
    mappedEncoderToVolume = findLinearConversion(slopeVolume, yInterceptVolume, encoderInput);

    // maps pixelcount to encoder
    slopePixel = findSlope(x1EncoderLow, y1PixelLow, x2EncoderHigh, y2PixelHigh);
    yInterceptPixel = findYInstercept(slopePixel, x1EncoderLow, y1PixelLow);
    mappedEncoderToPixel = findLinearConversion(slopePixel, yInterceptPixel, encoderInput);
    pixelCount = mappedEncoderToPixel;

    // maps huebulb brightness to encoder
    slopeHueBulb = findSlope(x1EncoderLow, y1BrightnessLow, x2EncoderHigh, y2BrightnessHigh);
    yInterceptHueBulb = findYInstercept(slopeHueBulb, x1EncoderLow, y1BrightnessLow);
    mappedEncoderToBrightness = findLinearConversion(slopeHueBulb, yInterceptHueBulb, encoderInput);
    pixel.setBrightness(mappedEncoderToBrightness * 0.5);

    if(encoderInput == 0) {
        checkEncoderPositionZero();
        if (( currentTime - previousTime ) > 500) {
            setHue(BULB1, false, HueBlue, mappedEncoderToBrightness, 255);
            mp3Player.volume(0);
            previousTime = currentTime;
        }
    }
    else {
        
        // sets color based on temperature. Pure blue at 32 degrees farenheight, pure red at 100 degrees farenheight.
        pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
        pixel.show();

        if(tempFar < previousTempFar) {
            pixel.clear();
            pixel.show();
            pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
        }

        // turns pixels off if the current pixelcount is less than the previous pixelcount
        if(pixelCount < previousInputPixel) {
            pixel.clear();
            pixel.show();
            pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
        }
        
        

        tempCel = bmeSensor.readTemperature();
        tempFar = celToFar(tempCel);

        if ((millis() - previousTime) > 1000) {
            previousTime = millis();

            Serial.printf("\nTemp in Fahrenheit is: %0.2f.\n", tempFar);

            // Update the huebulb
            if(encoderInput > 0) {
                if(mappedEncoderToVolume != previousInputVolume){
                    if (encoderSwitchToggle == true) {
                        setHue(BULB1, true, HueRed, mappedEncoderToBrightness, 255);
                    }
                    else {
                        setHue(BULB1, true, HueBlue, mappedEncoderToBrightness, 255);
                    }
                }
            }
            // prints data to the OLED display
            myOLED.setCursor(0, 0);
            myOLED.printf("---\nTemp: %0.2f F. \n---\n", tempFar);
            myOLED.display();        

            // MP3 player stuff
            // Set volume
            mp3Player.volume(mappedEncoderToVolume);
        }
    }
    // match previous inputs to current inputs.
    previousTempFar = tempFar;
    previousInputPixel = pixelCount;
    previousInputVolume = mappedEncoderToVolume;
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
    checkEncoderPositionZero();

}

// checks if the encoder output is 0 and turns off the all minipixels if true.
void checkEncoderPositionZero() {
    if(encoderInput == 0) {
        pixel.clear();
        pixel.show();
    }
}