/*
   Aurora: https://github.com/pixelmatix/aurora
   Copyright (c) 2014 Jason Coon

   Portions of this code are adapted from LedEffects Plasma by Robert Atkins: https://bitbucket.org/ratkins/ledeffects/src/26ed3c51912af6fac5f1304629c7b4ab7ac8ca4b/Plasma.cpp?at=default
   Copyright (c) 2013 Robert Atkins

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
   the Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PatternPlasma_H
#define PatternPlasma_H
#define CONFIG_SPIFFS_META_LENGTH 4
#define MAX_BUFFER_SIZE 1024
#define NO_FILES 20

char iBuffer[NO_FILES * MATRIX_HEIGHT][MATRIX_WIDTH];

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = text.length();
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(0.5);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));

  // Asuming 5 pixel average character width
  for (int xpos = MATRIX_WIDTH; xpos > -(MATRIX_WIDTH + text_length * 5); xpos--)
  {
    display.setTextColor(display.color565(colorR, colorG, colorB));
    display.clearDisplay();
    display.setCursor(xpos, ypos);
    display.println(text);
    delay(scroll_delay);
    yield();

    // This might smooth the transition a bit if we go slow
    // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
    // display.setCursor(xpos-1,ypos);
    // display.println(text);

    //delay(scroll_delay / 5);
    //yield();

  }
}

class PatternPlasma : public Drawable {
  private:
    int time   = 0;
    int cycles = 0;
    int frames = 0;
    char cPatternReadFlag = 0;

    //char iBuffer[NO_FILES*MATRIX_HEIGHT][MATRIX_WIDTH];
    //char iBuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
  public:
    //char iBuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
    PatternPlasma() {
      name = (char *)"Plasma";
    }

    void start() {
      //scroll_text(3, 25, "QUADOME ANTENNA DEMO", 0, 255, 0); //250
      //scroll_text(15, 25, "QUADOME ANTENNA DEMO", 0, 255, 0); //250
      display.clearDisplay();
      ms_animation_max_duration = 70000;
    }
    //
    //      if (cPatternReadFlag ==1)return;
    //      char  cLine[MAX_BUFFER_SIZE]  = {0};
    //      char  cNumber[16]             = {0}; // up to 16 characters
    //      int   iExitLoop               = 0;
    //      int   iRow                    = 0;
    //      int   iCol                    = 0;
    //      int   iCharIndex              = 0;
    //
    //      Serial.println("Start the Plasma pattern");
    //
    //      FILE *file = fopen("/spiffs/Combined_Pattern_TX.txt", "r");
    //      if (!file) {
    //        Serial.println("Failed to open file for reading");
    //        //return;
    //      }
    //
    //      Serial.println("After opening the file");
    //
    //      if (file != NULL)
    //      {
    //        while (fgets(cLine, sizeof(cLine), file) != NULL) // Get string stream line by line
    //        {
    //          iCol          = 0;
    //          iExitLoop     = 0;
    //          iCharIndex    = 0;
    //
    //          for ( int i = 0; (i < MAX_BUFFER_SIZE && iExitLoop == 0); i++)
    //          {
    //            if (cLine[i] == '\n') // End of line
    //            {
    //              cNumber[iCharIndex]   = '\n'; // Add null terminator
    //              iBuffer[iRow][iCol++] = atoi(cNumber);
    ////              Serial.print("iBuffer[i][j]: ");
    ////              Serial.print(iRow);
    ////              Serial.print(" ");
    ////              Serial.print(iCol - 1);
    ////              Serial.print(" ");
    ////              Serial.println(iBuffer[iRow][iCol - 1]);
    //              iExitLoop             = 1; // Exit the for loop
    //            }
    //            else
    //            {
    //              if (cLine[i] == ',') // Check the delimiter
    //              {
    //                cNumber[iCharIndex]   = '\n'; // Add null terminator
    ////                Serial.print("cNumber2: ");
    ////                Serial.println(cNumber);
    //                iBuffer[iRow][iCol++] = atoi(cNumber);
    ////                Serial.print("iBuffer[i][j]: ");
    ////                Serial.print(iRow);
    ////                Serial.print(" ");
    ////                Serial.print(iCol - 1);
    ////                Serial.print(" ");
    ////                Serial.println(iBuffer[iRow][iCol - 1]);
    //                iCharIndex            = 0; // reset number index
    //              }
    //              else
    //              {
    //                cNumber[iCharIndex] = cLine[i];
    //                iCharIndex++;
    //
    //              }
    //              //Serial.print("iCol: ");
    //              //Serial.println(iCol);
    //            }
    //          }
    //          iRow++;
    //          // Serial.print("iRow: ");
    //          // Serial.println(iRow);
    //          Serial.print("cNumber1: ");
    //          Serial.println(cNumber);
    //        }
    //        cPatternReadFlag = 1;
    //        fclose(file);
    //      }
    //      else
    //      {
    //        perror("Combined_Pattern_TX.txt");
    //      }
    //      //delay(100);
    //    }

    unsigned int drawFrame() {
      // display.clearDisplay();
      char iLocalBuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

      for (int x = 0; x < MATRIX_HEIGHT; x++) {
        for (int y = 0; y < MATRIX_WIDTH; y++) {
          if (iBuffer[x + (frames * MATRIX_HEIGHT)][y] <= 150) //&&((x<=3)||x>=(MATRIX_HEIGHT-3)||(y<=3)||(y>=(MATRIX_WIDTH-3)))) //'0')
            iLocalBuffer[x][y] = 150 - iBuffer[x + (frames * MATRIX_HEIGHT)][y]; //110; //5; //110;
          else if (iBuffer[x + (frames * MATRIX_HEIGHT)][y] <= 230)
            iLocalBuffer[x][y] = 0; //110; //5; //110;
          else
            iLocalBuffer[x][y] = iBuffer[x + (frames * MATRIX_HEIGHT)][y];
        }
      }

      for (int x = 0; x < MATRIX_HEIGHT; x++) {
        for (int y = 0; y < MATRIX_WIDTH; y++) {

          effects.Pixel(y, x, (int)iLocalBuffer[x][y]);
          //effects.Pixel(y, x, (int)iBuffer[x + (frames * MATRIX_HEIGHT)][y]);
          delay(1);
        }
        delay(4);
      }

      time += 1;
      cycles++;
      frames++;

      if (frames >= NO_FILES)
      {
        frames = 0;
      }
     // Serial.println(frames);

      if (cycles >= 2048) {
        time = 0;
        cycles = 0;
        cPatternReadFlag = 0;
      }
      delay(100);
      effects.ShowFrame();

      return 30;
    }
};

#endif
