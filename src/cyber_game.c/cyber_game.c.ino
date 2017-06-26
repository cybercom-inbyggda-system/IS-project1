/*
 * Copyright (c) 2017 Cybercom Sweden AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <avr/io.h>

#include <stdint.h>
#include <avr/io.h>

#include <avr/pgmspace.h>

#define BOARD BREADBOARD
//#define BOARD CYBERCOM_PCB
#include "ssd1306xled.h"


/***************************************
 *
 * Generic Helper functions
 *
 ***************************************/

/*!
 * \brief Clear an area on the screen
 */
void ssd1306_clear(int x_pos, int y_pos, unsigned int width, unsigned int height)
{
  int x, y;
  int x_start = x_pos;
  int x_stop = x_pos + width;
  int y_start = y_pos / 8;
  int y_stop = (y_pos + width + 7) / 8;

  if (x_start < 0)
    x_start = 0;
  if (x_stop > 128)
    x_stop = 128;
  if (y_start < 0)
    y_start = 0;
  if (y_stop > 8)
    y_stop = 8;

  for (int y = y_start; y < y_stop; y++)
  {
    ssd1306_setpos(x_start, y);
    ssd1306_send_data_start();
    for (int x = x_start; x < x_stop; x++)
      ssd1306_send_byte(0);
    ssd1306_send_data_stop();
  }
}


/*!
 * \brief Clear entire screen
 */
void ssd1306_clear() {
  ssd1306_clear(0, 0, 128, 64);
}


/*!
 * \brief Draw an image at some co-rdinate
 * 
 * Images may be located at any co-ordinate, both inside and outside the screen.
 * This enables scrolling an image into the screen.
 * NOTE that, althoug this function supports setting any position on an image, the ssd1306 
 * hardware only allows handling y-position in 8-pixle chunks. Hence, when writing of an image 
 * starts or stops at a y-position that is not a multiple of 8, more pixles outside this area 
 * will be written to.
 * 
 * \param x_pos x-position meassured in pixles
 * \param y_pos y-position meassured in pixles
 * \param width width of image in pixles
 * \param height height of image in pixles
 * \param bitmap raw pixle data
 */
void ssd1306_draw_bmp(int x_pos, int y_pos, uint8_t width, uint8_t height, const uint8_t bitmap[])
{
  uint8_t x_start = x_pos >= 0 ? x_pos : 0;
  uint8_t x_stop = x_pos + (int)width >= 0 ? x_pos + width : 0;

  uint8_t y_start = y_pos >= 0 ? y_pos >> 3 : 0;    // round downwards to multiple of 8
  uint8_t y_stop = y_pos + (int)height + 7 >= 0 ? (y_pos + height + 7) >> 3 : 0;
  uint8_t shift = y_pos & 0x7;
  uint8_t bmp_height = (height + 7) >> 3;
  y_pos = y_pos >= 0 ? y_pos / 8 : (y_pos - 7)/8;

  if (x_stop >= 128)
    x_stop = 128;
  if (y_stop >= 8)
    y_stop = 8;

  for (uint8_t y = y_start; y < y_stop; y++)
  {
    ssd1306_setpos(x_start, y);
    ssd1306_send_data_start();
    unsigned int i = (y - y_pos) * width + x_start - x_pos;
    for (uint8_t x = x_start; x < x_stop; x++)
    {
      uint8_t bits = 0x00;
      if (y - y_pos < bmp_height)
        bits = pgm_read_byte(&bitmap[i]) << shift;
      if (shift > 0 && y - y_pos > 0)
        bits |= pgm_read_byte(&bitmap[i - width]) >> (8 - shift);
      ssd1306_send_byte(bits);
      i++;
    }
    ssd1306_send_data_stop();
  }

}


#include "font8x16.h"
/*!
 * \brief Draw a text on the screen
 * 
 * To draw fast, we will avoid costly repositioning we will not 
 * draw each character separately. Instead, we will drawi top of 
 * all characters in one go, then repositioning and draw all 
 * bottom part in the next go.
 * 
 * \param x_pos x-position of upper left corner (in pixles)
 * \param y_pos y-position of upper left corner (in pixles)
 * \param str zero-terminated string
 */
void ssd1306_draw_text(int x_pos, int y_pos, char *str)
{
  const unsigned int bytes_per_char = 16;
  const unsigned int char_width = 8;
  const unsigned int height = 16;

  const unsigned width = char_width * strlen(str);

  uint8_t x_start = x_pos >= 0 ? x_pos : 0;
  uint8_t x_stop = x_pos + (int)width >= 0 ? x_pos + width : 0;

  uint8_t y_start = y_pos >= 0 ? y_pos >> 3 : 0;    // round downwards to multiple of 8
  uint8_t y_stop = y_pos + (int)height + 7 >= 0 ? (y_pos + height + 7) >> 3 : 0;
  uint8_t shift = y_pos & 0x7;
  uint8_t bmp_height = (height + 7) >> 3;
  y_pos = y_pos >= 0 ? y_pos / 8 : (y_pos - 7)/8;

  if (x_stop >= 128)
    x_stop = 128;
  if (y_stop >= 8)
    y_stop = 8;

  for (uint8_t y = y_start; y < y_stop; y++)
  {
    ssd1306_setpos(x_start, y);
    ssd1306_send_data_start();
    unsigned int i = x_start - x_pos;
    unsigned int j = y - y_pos;
    unsigned int char_idx;
    for (uint8_t x = x_start; x < x_stop; x++)
    {
      if (x == x_start || (i & 0x7) == 0)
      { 
        unsigned int ch = *(str + (i >> 3)); // i / char_width
        if (ch < 32 || ch > 126)
          ch = ' ';
        char_idx = (ch - 32) * bytes_per_char + j * char_width; 
      }

      uint8_t bits = 0;
      if (j < bmp_height)
        bits |= pgm_read_byte(&ssd1306xled_font8x16[char_idx + (i & 0x7)]) << shift;
      if (shift > 0 && j > 0)
        bits |= pgm_read_byte(&ssd1306xled_font8x16[char_idx - char_width + (i & 0x7)]) >> (8 - shift);
      ssd1306_send_byte(bits);

      i++;
    }
    ssd1306_send_data_stop();
    j++;
  }
}



/***************************************
 *
 * Specific Drawing Functions
 *
 ***************************************/

#include "numbers.h"
/*!
 * \brief Draw a right-aligned decimal number
 * 
 * \param x_pos x-position of upper _right_ corner (in pixles)
 * \param y_pos y-position of upper _right_ corner (in pixles)
 * \param count number to print
 */
void draw_number(unsigned int count, unsigned int x_pos, unsigned int y_pos)
{
  const unsigned int image_width = 11; // pixles
  const unsigned int image_height = 16; // pixles
  const unsigned int margin_pixles = 3;
  unsigned int count_tmp = count;
  static unsigned int prev_count = 0;
  const uint8_t *image_vect[] = {
    number_0, number_1, number_2, number_3, number_4,
    number_5, number_6, number_7, number_8, number_9
  };

  // write lowest number first
  for (unsigned int i = 0; i < 5; i++) {
    unsigned int rem = count_tmp % 10;
    const uint8_t *number_img;
    if (i > 0 && count_tmp == 0) {
      number_img = number_blank; // don't do leading 0s, do blank
    }
    else {
      number_img = image_vect[rem];
    }

    x_pos -= image_width - 1;
    ssd1306_draw_bmp(x_pos, y_pos, image_width, image_height, number_img);
    x_pos -= margin_pixles + 1;
    prev_count = prev_count / 10;
    count_tmp = count_tmp / 10;
    if (prev_count == count_tmp) break; // don't need to update rest
  }
  prev_count = count;
}


#include "cybercom.h"
/*!
 * \brief Draw Cybercom logo lookalike with 3 (original), 2, or 1 men
 * 
 * \param x_pos x-position of upper left corner (in pixles)
 * \param y_pos y-position of upper left corner (in pixles)
 * \param nbr_of_men display 3, 2, or 1 men
 */
void cybercom_men(unsigned int nbr_of_men, unsigned int x_pos, unsigned int y_pos)
{
  const unsigned int image_width[] = {18, 23, 28};
  const unsigned int image_height = 32;
  const uint8_t *image_vect[] = {
    cybercom_1, cybercom_2, cybercom_3, 
  };

  if (nbr_of_men >= 1 && nbr_of_men <= 3) {
    ssd1306_draw_bmp(x_pos, y_pos, image_width[nbr_of_men - 1], image_height, image_vect[nbr_of_men - 1]);
  }
}


#include "cybercom_running.h"
/*!
 * \brief Draw a running Cybercom man
 * 
 * \param x_pos x-position meassured in pixles
 * \param y_pos y-position meassured in pixles
 * \param running_step number of image in animation
 */
void cybercom_running(unsigned int running_step, int x_pos, int y_pos)
{
  const unsigned int image_width = 18;
  const unsigned int image_height = 32;
  const uint8_t *image_vect[] = {
    cybercom_running_0, cybercom_running_1, cybercom_running_2, cybercom_running_3, cybercom_running_4,
    cybercom_running_5, cybercom_running_6, cybercom_running_7, cybercom_running_8, cybercom_running_9,
  };

  if (running_step < 10) {
    ssd1306_draw_bmp(x_pos, y_pos, image_width, image_height, image_vect[running_step]);
  }
}


/***************************************
 *
 * Main Program
 *
 ***************************************/

// the setup function runs once when you press reset or power the board
void setup() {

  _delay_ms(500); // allow screen to power up

  ssd1306_init();

  // ---- Main Loop ----

  ssd1306_clear();

  cybercom_men(3, 100, 0);

  unsigned long count = 0;
  unsigned long prev_count = 0;
  int prev_man_y = 0;
  unsigned long prev_msec = millis();
  unsigned long interval = 1000; // milliseconds
  for (;;) {

    /* _delay_ms(200); */

    unsigned long msec = millis();
    if (msec - prev_msec >= interval) {
      draw_number((count - prev_count) * 1000 / interval, 95, 0); // print fps
      prev_msec = msec;
      prev_count = count;
    }

    int man_y = (count >> 2) % (2*96);
    if (man_y >= 96)
      man_y = man_y - 96 - 32;
    else
      man_y = 64 - man_y;
    cybercom_running((count / 2) % 10, 0, man_y);
//    cybercom_running((count / 2) % 10, 0, 5);  // for fps test

    ssd1306_draw_text(128 - count % 256, 48, "WATERFALL");
//    ssd1306_draw_text(count & 0x1 ? 0 : 32, 48, "WATERFALL"); // for fps test

    if ((man_y + 31)/8 < (prev_man_y+31)/8)
      ssd1306_clear(0, prev_man_y + 31, 18, 1);
    else if (man_y/8 > prev_man_y/8)
      ssd1306_clear(0, prev_man_y, 18, 1);

    prev_man_y = man_y;
    count++;
  }

}

