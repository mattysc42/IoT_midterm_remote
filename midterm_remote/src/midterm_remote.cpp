/* 
 * Project midterm_remote
 * Author: Matthew Call
 * Date: 7/9/24
 */

// Most of the header is in a seperate file to increase readability.
#include "header_includes.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

// SYSTEM_THREAD(ENABLED);

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected, 2500);

    // currently disabled because I'm not in range of this network.
    /* WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork");
    WiFi.connect();
    while(WiFi.connecting()) {
        Serial.printf(".");
    } */
    
    // encloder switch RGB
    pinMode(ENCODERSWITCHRED, OUTPUT);
    pinMode(ENCODERSWITCHGREEN, OUTPUT);
    pinMode(ENCODERSWITCHBLUE, OUTPUT);

    // ultrasonic sensor stuff
    pinMode(TRIGPIN, OUTPUT);  
	pinMode(ECHOPIN, INPUT);

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
    Serial.printf("Initializing MP3 Player. Please wait...\n");
    Serial2.begin(9600);
    mp3Player.begin(Serial2);
    delay(1000);

    // check that the MP3 is connected.
    if (!mp3Player.begin(Serial2)) {  //Use softwareSerial to communicate with mp3.
        Serial.printf("Unable to begin:\n");
        Serial.printf("1.Please recheck the connection!\n");
        Serial.printf("2.Please insert the SD card!\n");
        while(true);
    }

    // default startup output settings.
    mp3Player.loopFolder(1);
    mp3Player.enableLoopAll();
    mp3Player.pause();
    encoder.write(62);
    encoderInput = 62;
    mp3Player.volume(20);
    cycleHueBulbs(HueBlue, 255);
    Serial.printf("playing quiet playlist.");
}

void loop() {
    //start the timer
    currentTime = millis();

    // ultrasonic sensor stuff
    digitalWrite(TRIGPIN, LOW);  
	delayMicroseconds(2);  
	digitalWrite(TRIGPIN, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(TRIGPIN, LOW);
    duration = pulseIn(ECHOPIN, HIGH);
    distance = (duration * 0.0343) / 2;

    // Pause the playback and turn everything off if I'm away from my desk for more than 10 seconds. 
    if(distance > 70) {
        if((currentTime - previousTimeSensor) > 10000) {
            mp3Player.pause();
            toggleProximityStart = false;
            wemoToggleState = false;
            digitalWrite(ENCODERSWITCHRED, HIGH);
            digitalWrite(ENCODERSWITCHGREEN, HIGH);
            digitalWrite(ENCODERSWITCHBLUE, HIGH);
            pixel.setBrightness(0);
            pixel.show();
            myOLED.clearDisplay();
            myOLED.setCursor(0, 0);
            myOLED.display();
            cycleHueBulbs(blue, 0);
            wemoWrite(MYWEMO, LOW);
            wemoWrite(MYWEMO2, LOW);
            wemoWrite(MYWEMO3, LOW);
            previousTimeSensor = currentTime;
        }
    }

    // Auto start if sensor reads less than 70cm.
    else {

        // Reset the 10 second pause timer if I move back into range.
        previousTimeSensor = previousTime;
        
        // read the encoder input
        encoderInput = encoder.read();

        //start a cycle timer to prevent excessive inputs
        if ((currentTime - previousTime) > 50) {
            currentTrack = mp3Player.readCurrentFileNumber(Serial2);
            
            // change the encoder color depending on the playlist. This encoder turns on if the output is LOW.
            if(encoderSwitchToggle == true) {
                digitalWrite(ENCODERSWITCHRED, HIGH);
                digitalWrite(ENCODERSWITCHGREEN, LOW);
                digitalWrite(ENCODERSWITCHBLUE, HIGH);
            }
            else {
                digitalWrite(ENCODERSWITCHRED, HIGH);
                digitalWrite(ENCODERSWITCHGREEN, HIGH);
                digitalWrite(ENCODERSWITCHBLUE, LOW);
            }
            if(toggleStartStop == false) {
                digitalWrite(ENCODERSWITCHRED, LOW);
                digitalWrite(ENCODERSWITCHGREEN, HIGH);
                digitalWrite(ENCODERSWITCHBLUE, HIGH);
            }

            // prevent the encoder input from going out of range
            if(encoderInput <= 0) {
                encoderInput = 0;
                encoder.write(0);
            }
            else if(encoderInput >= 96) {
                encoderInput = 96;
                encoder.write(96);
            }

            // get temperature and converts it to farhenheight.
            tempCel = bmeSensor.readTemperature();
            tempFar = celToFar(tempCel);

            // turn on the wemos if temperature goes over 75 degrees farhenheight.
            if(tempFar > 75) {
                if(wemoToggleState == false) {
                    wemoWrite(MYWEMO, LOW);
                    wemoWrite(MYWEMO2, LOW);
                    wemoWrite(MYWEMO3, HIGH);
                    wemoToggleState = !wemoToggleState;
                }
            }

            // turn the wemos off if temperature goes below 75 degrees farhenheight.
            else {
                if(wemoToggleState == true) {
                    if(tempFar < 75) {
                        wemoWrite(MYWEMO, HIGH);
                        wemoWrite(MYWEMO2, HIGH);
                        wemoWrite(MYWEMO3, LOW);
                        wemoToggleState = !wemoToggleState;
                    }
                }
            }

            // get neopixel colors based on the MBE readings while keeping the values inside the analog output limits.
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

            // maps huebulb and neopixel brightness to encoder. Due to input lag issues with the huebulbs, this only works for the neopixel brightness currently.
            slopeHueBulb = findSlope(x1EncoderLow, y1BrightnessLow, x2EncoderHigh, y2BrightnessHigh);
            yInterceptHueBulb = findYInstercept(slopeHueBulb, x1EncoderLow, y1BrightnessLow);
            mappedEncoderToBrightness = findLinearConversion(slopeHueBulb, yInterceptHueBulb, encoderInput);
            pixel.setBrightness(mappedEncoderToBrightness * 0.5);
            
            // if the pause button is clicked, turn all the huebulbs red, pause the music, and turn off the wemos. Pressing again plays the music again and turns the huebulbs to the playlist color.
            if(buttonStartStop.isClicked()) {
                if(toggleStartStop == true) {
                    mp3Player.pause();
                    toggleStartStop = !toggleStartStop;
                    cycleHueBulbs(HueRed, 255.0);
                    wemoWrite(MYWEMO, LOW);
                    wemoWrite(MYWEMO2, LOW);
                    wemoWrite(MYWEMO3, LOW);
                }
                else {
                    if(toggleProximityStart == true) {
                        mp3Player.start();
                    }
                    toggleStartStop = !toggleStartStop;
                    if(!encoderSwitchToggle) {
                        Serial.printf("playing quiet playlist.");
                        mp3Player.volume(mappedEncoderToVolume);
                        cycleHueBulbs(HueBlue, mappedEncoderToBrightness);
                        
                    }
                    else {
                        Serial.printf("playing loud playlist.");
                        mp3Player.volume(mappedEncoderToVolume);
                        cycleHueBulbs(HueGreen, mappedEncoderToBrightness);
                        
                    }
                }
            }
               
            // switch playlists, encoder switch LED color, and huebulb color if the encoder switch is clicked.
            if(encoderButton.isClicked()) {
                if(toggleStartStop == true) {
                    encoderSwitchToggle = !encoderSwitchToggle;
                    if(!encoderSwitchToggle) {
                        mp3Player.start();
                        Serial.printf("playing quiet playlist.");
                        cycleHueBulbs(HueBlue, mappedEncoderToBrightness);
                    }
                    else {
                        mp3Player.start();
                        Serial.printf("playing loud playlist.");
                        cycleHueBulbs(HueGreen, mappedEncoderToBrightness);
                    } 
                } 
            }

            // choose the playlist depending on the encoder switch toggle. Prevents looping to different playlists after completing the current one.
            if(!encoderSwitchToggle) {
                if(currentTrack > 10) {
                    mp3Player.loopFolder(1);
                    Serial.printf("playing quiet playlist.");
                }
            }
            else {
                if(currentTrack < 11) {
                    mp3Player.loopFolder(2);
                    Serial.printf("playing loud playlist.");
                }
            }

            // if the encoder input changes, update the neopixel output.
            if(encoderInput != previousEncoderInput) {

                // sets color based on temperature. Pure blue at 32 degrees farenheight, pure red at 100 degrees farenheight.
                pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));

                if(tempFar < previousTempFar) {
                    pixel.clear();
                    pixel.show();
                    pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
                }

                // turns pixels off if the current pixelcount is less than the previous pixelcount then turns on the correct pixelcount
                if(pixelCount < previousInputPixel) {
                    pixel.clear();
                    pixel.show();
                    pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
                }
                
                // sets volume to match the encoder and pixelcount
                mp3Player.volume(mappedEncoderToVolume);
            }

            // check encoder input for 0.
            if(encoderInput == 0) {
                checkEncoderPositionZero();
                if(encoderInput != previousEncoderInput) {
                    mp3Player.volume(mappedEncoderToVolume);
                }
            }

            // prints data to the OLED display at 1 second intervals
            if((currentTime - previousTime2) > 1000) {

                // change the color based on temperature.
                if(tempFar != previousTempFar) {
                    pixel.clear();
                    pixel.show();
                    pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
                }

                // display the temperature on the OLED.
                myOLED.clearDisplay();
                myOLED.setCursor(0, 0);
                myOLED.printf("Temp: %0.2f F. \n---\nCurrent Track:\n \n", tempFar);
                
                // include the file with the playlist artists and song names and output them to the OLED
                #include "playlist.h"
                myOLED.display(); 
                Serial.printf("\nTemp in Fahrenheit is: %0.2f.\n", tempFar);
                previousTime2 = currentTime;

                // prints the Ultrasonic sensor distance to the serial monitor.
                Serial.printf("\nDistance: %0.4f\n", distance);

                // ensures that the music keeps looping even if it was paused. Without this, the music stops when the current song finishes.
                mp3Player.enableLoopAll();
            }

            // match previous inputs to current inputs.
            previousTempFar = tempFar;
            previousInputPixel = pixelCount;
            previousEncoderInput = encoderInput;
            previousEncoderToBrightness = mappedEncoderToBrightness;
            previousTime = currentTime;

            // Turn on the lights and music if the ultrasonic sensor detects someone in range.
            if(toggleProximityStart == false && toggleStartStop == true) {
                toggleProximityStart = !toggleProximityStart;
                pixelFill(0, pixelCount, (color * 2) - 64, 0, 200 - (color * 2));
                mp3Player.start();
            }
        }
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

void cycleHueBulbs(int playlistColor, float brightness) {
    for(int i = 1; i <= 6; i++) {
        setHue(i, true, playlistColor, 255, 255);
    }
}