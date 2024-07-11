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
    if (!mp3Player.begin(Serial2)) {  //Use softwareSerial to communicate with mp3.
        Serial.printf("Unable to begin:\n");
        Serial.printf("1.Please recheck the connection!\n");
        Serial.printf("2.Please insert the SD card!\n");
        while(true);
    }
    mp3Player.loopFolder(1);
    mp3Player.volume(10);
    setHue(BULB1, true, HueBlue, 75, 255);
    Serial.printf("playing quiet playlist.");
}

void loop() {
    myOLED.clearDisplay();
    encoderInput = encoder.read();
    currentTrack = mp3Player.readCurrentFileNumber();
    currentTime = millis();
    if ((currentTime - previousTime) > 5) {

        if(mp3NextButton.isClicked()) {
            if(mp3Player.readCurrentFileNumber() == 10) {
                if(!encoderSwitchToggle) {
                    mp3Player.loopFolder(1);
                }
                else {
                    mp3Player.loopFolder(2);
                }
            }
            else{
                mp3Player.next();
            }
        }
        if(mp3BackButton.isClicked()) {
            mp3Player.previous();
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
        
        // switch playlists, encoder switch LED color, and huebulb color if the encoder switch is clicked.
        if(encoderButton.isClicked()) {
            encoderSwitchToggle = !encoderSwitchToggle;
            if(!encoderSwitchToggle) {
                setHue(BULB1, true, HueBlue, mappedEncoderToBrightness, 255);
                mp3Player.loopFolder(1);
                Serial.printf("playing quiet playlist.");
                mp3Player.volume(mappedEncoderToVolume);
            }
            else {
                setHue(BULB1, true, HueRed, mappedEncoderToBrightness, 255);
                mp3Player.loopFolder(2);
                Serial.printf("playing loud playlist.");
                mp3Player.volume(mappedEncoderToVolume);
            } 
        } 

        if(encoderInput != previousEncoderInput) {
            if(encoderSwitchToggle == 0) {
                setHue(BULB1, true, HueBlue, mappedEncoderToBrightness, 255);
                mp3Player.volume(mappedEncoderToVolume);
            }
            else {
                setHue(BULB1, true, HueRed, mappedEncoderToBrightness, 255);
                mp3Player.volume(mappedEncoderToVolume);
            } 
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
        }

        // check encoder input and work from there.
        if(encoderInput == 0) {
            checkEncoderPositionZero();
            if(encoderInput != previousEncoderInput) {
                setHue(BULB1, false, HueBlue, mappedEncoderToBrightness, 255);
                // Set volume
                mp3Player.volume(mappedEncoderToVolume);
            }
        }

        // gets temperature and converts it to farenheight.
        // 
        tempCel = bmeSensor.readTemperature();
        tempFar = celToFar(tempCel);

        // prints data to the OLED display at 1 second intervals
        myOLED.setCursor(0, 0);
        myOLED.printf("---\nTemp: %0.2f F. \n---\n", tempFar);
        myOLED.display(); 
        //
        Serial.printf("\nTemp in Fahrenheit is: %0.2f.\n", tempFar);
        
        // match previous inputs to current inputs.
        previousTempFar = tempFar;
        previousInputPixel = pixelCount;
        previousEncoderInput = encoderInput;
        previousEncoderToBrightness = mappedEncoderToBrightness;
        previousTime = currentTime;
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
    checkEncoderPositionZero();

}

// checks if the encoder output is 0 and turns off the all minipixels if true.
void checkEncoderPositionZero() {
    if(encoderInput == 0) {
        pixel.clear();
        pixel.show();
    }
}