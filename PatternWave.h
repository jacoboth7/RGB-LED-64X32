/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PatternWave_H
#define PatternWave_H

//extern unsigned long ms_animation_max_duration

void scroll_text2(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = text.length();
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
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
   }
}

class PatternWave : public Drawable {
private:
    byte thetaUpdate = 0;
    byte thetaUpdateFrequency = 0;
    byte theta = 0;

    byte hueUpdate = 0;
    byte hueUpdateFrequency = 0;
    byte hue = 0;

    byte rotation = 0;

    uint8_t scale = 256 / MATRIX_WIDTH;

    uint8_t maxX = MATRIX_WIDTH - 1;
    uint8_t maxY = MATRIX_HEIGHT - 1;

    uint8_t waveCount = 1;

public:
    PatternWave() {
        name = (char *)"Wave";
    }

    void start() {
        //scroll_text2(15, 25, "RANDOM PATTERN DEMO", 0, 255, 0); //250 
        rotation = random(0, 4);
        waveCount = random(1, 3);
        

        ms_animation_max_duration = 40000;
      
       display.clearDisplay();
    }

    unsigned int drawFrame() {
        int n = 0;

        //Serial.println("Inside DrawFrame");
        switch (rotation) {
            case 0:
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    n = quadwave8(x * 2 + theta) / scale;
                    effects.drawBackgroundFastLEDPixelCRGB(x, n, effects.ColorFromCurrentPalette(x + hue));
                    if (waveCount == 2)
                        effects.drawBackgroundFastLEDPixelCRGB(x, maxY - n, effects.ColorFromCurrentPalette(x + hue));
                }
                break;

            case 1:
                for (int y = 0; y < MATRIX_HEIGHT; y++) {
                    n = quadwave8(y * 2 + theta) / scale;
                    effects.drawBackgroundFastLEDPixelCRGB(n, y, effects.ColorFromCurrentPalette(y + hue));
                    if (waveCount == 2)
                        effects.drawBackgroundFastLEDPixelCRGB(maxX - n, y, effects.ColorFromCurrentPalette(y + hue));
                }
                break;

            case 2:
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    n = quadwave8(x * 2 - theta) / scale;
                    effects.drawBackgroundFastLEDPixelCRGB(x, n, effects.ColorFromCurrentPalette(x + hue));
                    if (waveCount == 2)
                        effects.drawBackgroundFastLEDPixelCRGB(x, maxY - n, effects.ColorFromCurrentPalette(x + hue));
                }
                break;

            case 3:
                for (int y = 0; y < MATRIX_HEIGHT; y++) {
                    n = quadwave8(y * 2 - theta) / scale;
                    effects.drawBackgroundFastLEDPixelCRGB(n, y, effects.ColorFromCurrentPalette(y + hue));
                    if (waveCount == 2)
                        effects.drawBackgroundFastLEDPixelCRGB(maxX - n, y, effects.ColorFromCurrentPalette(y + hue));
                }
                break;
        }

        effects.DimAll(254);
		effects.ShowFrame();

        if (thetaUpdate >= thetaUpdateFrequency) {
            thetaUpdate = 0;
            theta++;
        }
        else {
            thetaUpdate++;
        }

        if (hueUpdate >= hueUpdateFrequency) {
            hueUpdate = 0;
            hue++;
        }
        else {
            hueUpdate++;
        }

        return 0;
    }
};

#endif
