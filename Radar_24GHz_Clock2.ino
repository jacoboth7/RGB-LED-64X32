#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <PxMatrix.h>
#include "SPIFFS.h"
#include <ld2410.h>



// Pin definitions for ESP32
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_OE 21

#define ESP32

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool ChangedToIdle = false;

// Define display size
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define NO_FILES 20

unsigned long ms_current  = 0;
unsigned long ms_previous = 0;
unsigned long ms_animation_max_duration = 400000; // 10 seconds
unsigned long next_frame = 0;

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 50; //30-60 is usually fine

PxMATRIX display(MATRIX_WIDTH, MATRIX_HEIGHT, P_LAT, P_OE, P_A, P_B, P_C, P_D);

#include <FastLED.h> // Aurora needs fastled

#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Playlist.h"

#include "Patterns.h"
Patterns patterns;

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};


// Wi-Fi credentials
const char *ssid = "Botha-WiFi"; //"Schlebusch WiFi"; // //"Galaxy A54 5G EA55";
const char *password = "Botha12345"; //"Attiesch1997"; //

// NTP and time variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", 7200, 60000); // Adjusted to +2 hours

byte _Second = 0, _Minute = 0, _Hour = 0;
unsigned long _Epoch;

// SEGMENTS
#include "Digit.h"
Digit digit0(&display, 2, 64 - 0 - 9 * 1, -7, display.color565(255, 0, 0));
Digit digit1(&display, 2, 64 - 1 - 9 * 2, -7, display.color565(255, 0, 0));
Digit digit2(&display, 2, 62 - 2 - 9 * 3, -7, display.color565(255, 0, 0));
Digit digit3(&display, 2, 62 - 3 - 9 * 4, -7, display.color565(255, 0, 0));
Digit digit4(&display, 2, 60 - 4 - 9 * 5, -7, display.color565(255, 0, 0));
Digit digit5(&display, 2, 60 - 5 - 9 * 6, -7, display.color565(255, 0, 0));

// State machine definitions
enum State {
  IDLE,
  DETECT_MOVING_TARGET,
  DEMO
};

State currentState = IDLE;
unsigned long stateStartTime = 0;
unsigned long TimeInCloseRange = 0;
unsigned long StartTimeInCloseRange = 0;
unsigned long TimeInIDLE = 0;
bool targetDetected = false, latch = false;

uint32_t lastReading = 0;
uint32_t TimeSinceMovingTarget = 0;
bool radarConnected = false;
bool NewInIDLE = true;

ld2410 radar;

#if defined(ESP32)
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33
#endif

void IRAM_ATTR display_updater() {
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(50);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{

#ifdef ESP8266
  if (is_enable)
    display_ticker.attach(0.001, display_updater);
  else
    display_ticker.detach();
#endif

#ifdef ESP32
  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 2000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
#endif
}

void setup()
{
  //Set - up for the demo SPIFF file :
  //Mount the SPIFFS file
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //===============
  char  cLine[MAX_BUFFER_SIZE]  = {0};
  char  cNumber[16]             = {0}; // up to 16 characters
  int   iExitLoop               = 0;
  int   iRow                    = 0;
  int   iCol                    = 0;
  int   iCharIndex              = 0;

  FILE *file = fopen("/spiffs/Combined_Pattern_TX_big.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    //return;
  }

  Serial.println("After opening the file");

  if (file != NULL)
  {
    while (fgets(cLine, sizeof(cLine), file) != NULL) // Get string stream line by line
    {
      iCol          = 0;
      iExitLoop     = 0;
      iCharIndex    = 0;

      for ( int i = 0; (i < MAX_BUFFER_SIZE && iExitLoop == 0); i++)
      {
        if (cLine[i] == '\n') // End of line
        {
          cNumber[iCharIndex]   = '\n'; // Add null terminator
          iBuffer[iRow][iCol++] = atoi(cNumber);
          iExitLoop             = 1; // Exit the for loop
        }
        else
        {
          if (cLine[i] == ',') // Check the delimiter
          {
            cNumber[iCharIndex]   = '\n'; // Add null terminator
            iBuffer[iRow][iCol++] = atoi(cNumber);
            iCharIndex            = 0; // reset number index
          }
          else
          {
            cNumber[iCharIndex] = cLine[i];
            iCharIndex++;

          }
        }
      }
      iRow++;
    }
    fclose(file);
  }
  else
  {
    perror("Combined_Pattern_TX_big.txt");
  }

  // Wi-Fi setup
  WiFi.begin(ssid, password);
  Serial.print("Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");
  timeClient.begin();
  delay(100);

  // Display setup
  display.begin(16);
  display.setFastUpdate(true);
  display.clearDisplay();
  display.setTextColor(display.color565(255, 255, 255));
  display.setMuxPattern(BINARY);
  // Rotate display
  //display.setRotate(2);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &display_updater, true);
  timerAlarmWrite(timer, 2000, true);
  timerAlarmEnable(timer);

  // Radar setup
  MONITOR_SERIAL.begin(9600);
#if defined(ESP32)
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
#endif
  delay(500);
  if (radar.begin(RADAR_SERIAL)) {
    MONITOR_SERIAL.println(F("LD2410 radar sensor initialised"));
  } else {
    MONITOR_SERIAL.println(F("Radar sensor not connected"));
  }


  delay(2000);

  effects.Setup();
  delay(100);
  Serial.println("Effects being loaded: ");
  listPatterns();
  TimeInIDLE = millis();
}

void listPatterns() {
  patterns.listPatterns();
}


void DigitalClock()
{
  TimeSinceMovingTarget = millis();
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();
    if (unix_epoch != _Epoch) {
      //      Serial.print("ChangedToIdle");
      //      Serial.println(ChangedToIdle);
      int Second = second(unix_epoch);
      int Minute = minute(unix_epoch);
      int Hour = hour(unix_epoch);

      if ((ChangedToIdle == true) || (_Epoch == 0)) { // If we didn't have a previous time. Just draw it without morphing.

        Serial.println("Epoch = 0");
        digit0.Draw(Second % 10);
        digit1.Draw(Second / 10);
        digit2.Draw(Minute % 10);
        digit3.Draw(Minute / 10);
        digit4.Draw(Hour % 10);
        if (Hour >= 10) digit5.Draw(Hour / 10);
        ChangedToIdle = false;
        //delay(500);
      }
      else
      {
        // epoch changes every miliseconds, we only want to draw when digits actually change.
        if (Second != _Second) {
          digit1.DrawColon(display.color565(0, 0, 0));
          digit3.DrawColon(display.color565(0, 0, 0));
          int s0 = Second % 10;
          int s1 = Second / 10;
          if (s0 != digit0.Value()) digit0.Morph(s0);
          if (s1 != digit1.Value()) digit1.Morph(s1);
          digit1.DrawColon(display.color565(255, 255, 255));
          digit3.DrawColon(display.color565(255, 255, 255));
          _Second = Second;
        }

        if (Minute != _Minute) {
          int m0 = Minute % 10;
          int m1 = Minute / 10;
          if (m0 != digit2.Value()) digit2.Morph(m0);
          if (m1 != digit3.Value()) digit3.Morph(m1);
          _Minute = Minute;
        }

        if (Hour != _Hour) {
          int h0 = Hour % 10;
          int h1 = Hour / 10;
          if (h0 != digit4.Value()) digit4.Morph(h0);
          if (h1 > 0) {
            if (h1 != digit5.Value()) digit5.Morph(h1);
          }
          _Hour = Hour;
        }
      }
      _Epoch = unix_epoch;
    }
  }
}

void loop() {
  unsigned long currentTime = millis();
  radar.read();

  switch (currentState) {
    case IDLE:

      // Serial.println("State: IDLE");
      if (NewInIDLE == true)
      {
        NewInIDLE = false;
        TimeInIDLE = millis();
        display.clearDisplay();
      }
    
      DigitalClock();

      if (radar.movingTargetDetected() && (millis() - TimeInIDLE > 3000)) {
        currentState = DETECT_MOVING_TARGET;
        Serial.println("Change state to DETECT MOVING TARGET");
        stateStartTime = currentTime;
        delay(300);
      }


      break;

    case DETECT_MOVING_TARGET:
      //Serial.println("State: DETECT_MOVING_TARGET");

      if (radar.isConnected() && millis() - lastReading > 100) {
        lastReading = millis();
        if (radar.movingTargetDetected()) {
          stateStartTime = millis();
          targetDetected = true;
          //currentState = DEMO;
          display.clearDisplay();
          float MovingTargetRange = radar.movingTargetDistance() / 100.0;
          String number = String(MovingTargetRange, 1);
          int16_t x1, y1;
          uint16_t w, h;

          display.getTextBounds(number, 0, 0, &x1, &y1, &w, &h);
          display.setTextSize(1);
          display.setCursor((MATRIX_WIDTH - w + 2) / 2, (MATRIX_HEIGHT - h) / 2);
          display.print(number);
          display.showBuffer();

          uint16_t color = display.color565(255, 0, 0);
          int number_int = (int)MovingTargetRange;
          for (int i = 1; i < (7 - number_int); i++) {
            display.drawCircle(MATRIX_WIDTH / 2, MATRIX_HEIGHT / 2, 9 + i, color);
          }

          if (MovingTargetRange < 1.5)
          {
            if (latch == false)
            {
              StartTimeInCloseRange = millis();
              latch = true;
            }
            TimeInCloseRange = millis() - StartTimeInCloseRange;
          }
          else
            TimeInCloseRange = 0;

//          if ((MovingTargetRange < 1.5) && (TimeInCloseRange > 2000))
//          {
//            latch = false;
//            currentState = DEMO;
//            Serial.println("Change State to DEMO");
//            //Demo Init
//            display.setFastUpdate(true);
//            display.clearDisplay();
//            display.setTextColor(myGREEN); //CYAN);
//            display.setCursor(7, 5);
//            display.print("HENSOLDT");
//            display.setTextColor(myMAGENTA);  //myRED
//            display.setCursor(10, 19);
//            display.print("QUADOME");
//            display_update_enable(true);
//            delay(2000);
//            scroll_text(1, 25, "DYNAMIC ELECTRONIC PATTERN SCAN", 0, 255, 0);
//            scroll_text(9, 25, "SUPERIOR SITUATIONAL AWARENESS", 255, 0, 255);
//            scroll_text(17, 25, "MULTIPLE TARGET TRACKING", 0, 255, 0);
//            scroll_text(25, 25, "JAMMING RESISTANT", 255, 0, 255);
//            // Demo Init end
//            TimeInCloseRange = 0;
//          }

        } else if (millis() - stateStartTime > 5000) {
          currentState = IDLE;
          ChangedToIdle = true;
          display.clearDisplay();
          delay(100);
          Serial.println("Change State to IDLE");
          //TimeInIDLE = millis();
          NewInIDLE = true;
        }
      }

      break;

    case DEMO:
      //Serial.println("State: DEMO");

      Demo();

      //  if (targetDetected) {
      //    currentState = DETECT_MOVING_TARGET;
      //    ;
      //  } else {
      //    currentState = IDLE;
      //  }

      break;

      //        default:
      //            Serial.println("Error: Unknown state!");
      //            currentState = IDLE;
      //            break;
  }

  //delay(1000);
}  // End of loop

void Demo()
{
  ms_current = millis();
  Serial.println(ms_current);

  if ( (ms_current - ms_previous) > ms_animation_max_duration )
  {
    patterns.stop();
    patterns.move(1);
    patterns.start();

    if (patterns.getCurrentPatternName() == "Wave")
      scroll_text(5, 25, "RANDOM PATTERN DEMO", 0, 255, 0);
    else if (patterns.getCurrentPatternName() == "Plasma")
      scroll_text(15, 25, "QUADOME ANTENNA DEMO", 0, 255, 0);

    display.clearDisplay();
    Serial.print("Changing pattern to:  ");
    Serial.println(patterns.getCurrentPatternName());

    ms_previous = ms_current;
 /// Improve this change condition 
    // Change back to IDLE state
    currentState = IDLE;
    ChangedToIdle = true;
    display.clearDisplay();
    Serial.println("Change State to IDLE");
    NewInIDLE = true;
    //TimeInIDLE = millis();
  }

  if ( next_frame < ms_current)
  {
    next_frame = patterns.drawFrame() + ms_current;
    display.showBuffer();

  }
}
