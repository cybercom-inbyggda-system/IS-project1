/**
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 *
 * This is part of the Tinusaur/SSD1306xLED project.
 *
 * Copyright (c) 2016 Neven Boyanov, Tinusaur Team. All Rights Reserved.
 * Distributed as open source software under MIT License, see LICENSE.txt file.
 * Please, as a favor, retain the link http://tinusaur.org to The Tinusaur Project.
 *
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 *
 */

#ifndef SSD1306XLED_H
#define SSD1306XLED_H

// ============================================================================

#define CYBERCOM_PCB   1
#define BREADBOARD     2
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                ATtiny
//               25/45/85
//              +-(_)------+
//      (RST)---+ PB5  Vcc +---(+)-------
//           ---+ PB3  PB2 +---
//           ---+ PB4  PB1 +---SCL
// -------(-)---+ GND  PB0 +---SDA
//              +----------+
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if BOARD == CYBERCOM_PCB
#define SSD1306_SCL    PB1   // SCL on SSD1306 Board
#define SSD1306_SDA    PB0   // SDA on SSD1306 Board
#elif BOARD == BREADBOARD
#define SSD1306_SCL    PB3
#define SSD1306_SDA    PB4
#endif

#define SSD1306_SA     0x78  // Slave address

// ----------------------------------------------------------------------------

// Convenience definitions for PORTB

#define DIGITAL_WRITE_HIGH(PORT) PORTB |= (1 << PORT)
#define DIGITAL_WRITE_LOW(PORT) PORTB &= ~(1 << PORT)

// ----------------------------------------------------------------------------

/**
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 *
 * This is part of the Tinusaur/SSD1306xLED project.
 *
 * Copyright (c) 2016 Neven Boyanov, Tinusaur Team. All Rights Reserved.
 * Distributed as open source software under MIT License, see LICENSE.txt file.
 * Please, as a favor, retain the link http://tinusaur.org to The Tinusaur Project.
 *
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 *
 */

// ============================================================================

#include <stdlib.h>
#include <avr/io.h>

#include <stdint.h>
#include <avr/io.h>

#include <avr/pgmspace.h>

// ----------------------------------------------------------------------------

// Some code based on "IIC_wtihout_ACK" by http://www.14blog.com/archives/1358

const uint8_t ssd1306_init_sequence [] PROGMEM = {  // Initialization Sequence
  0xAE,       // Display OFF (sleep mode)
  0x20, 0b00, // Set Memory Addressing Mode
              // 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
              // 10=Page Addressing Mode (RESET); 11=Invalid
  0xB0,       // Set Page Start Address for Page Addressing Mode, 0-7
  0xC8,       // Set COM Output Scan Direction
  0x00,       // ---set low column address
  0x10,       // ---set high column address
  0x40,       // --set start line address
  0x81, 0x3F, // Set contrast control register
  0xA1,       // Set Segment Re-map. A0=address mapped; A1=address 127 mapped. 
  0xA6,       // Set display mode. A6=Normal; A7=Inverse
  0xA8, 0x3F, // Set multiplex ratio(1 to 64)
  0xA4,       // Output RAM to Display
              // 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
  0xD3, 0x00, // Set display offset. 00 = no offset
  0xD5,       // --set display clock divide ratio/oscillator frequency
  0xF0,       // --set divide ratio
  0xD9, 0x22, // Set pre-charge period
  0xDA, 0x12, // Set com pins hardware configuration    
  0xDB,       // --set vcomh
  0x20,       // 0x20,0.77xVcc
  0x8D, 0x14, // Set DC-DC enable
  0xAF        // Display ON in normal mode
  
};

// ----------------------------------------------------------------------------

void ssd1306_xfer_start(void)
{
  DIGITAL_WRITE_HIGH(SSD1306_SCL);  // Set to HIGH
  DIGITAL_WRITE_HIGH(SSD1306_SDA);  // Set to HIGH
  DIGITAL_WRITE_LOW(SSD1306_SDA);   // Set to LOW
  DIGITAL_WRITE_LOW(SSD1306_SCL);   // Set to LOW
}

void ssd1306_xfer_stop(void)
{
  DIGITAL_WRITE_LOW(SSD1306_SCL);   // Set to LOW
  DIGITAL_WRITE_LOW(SSD1306_SDA);   // Set to LOW
  DIGITAL_WRITE_HIGH(SSD1306_SCL);  // Set to HIGH
  DIGITAL_WRITE_HIGH(SSD1306_SDA);  // Set to HIGH
}

void ssd1306_send_byte(uint8_t byte)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if ((byte << i) & 0x80)
      DIGITAL_WRITE_HIGH(SSD1306_SDA);
    else
      DIGITAL_WRITE_LOW(SSD1306_SDA);
    
    DIGITAL_WRITE_HIGH(SSD1306_SCL);
    DIGITAL_WRITE_LOW(SSD1306_SCL);
  }
  DIGITAL_WRITE_HIGH(SSD1306_SDA);
  DIGITAL_WRITE_HIGH(SSD1306_SCL);
  DIGITAL_WRITE_LOW(SSD1306_SCL);
}

void ssd1306_send_command_start(void) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_SA);  // Slave address, SA0=0
  ssd1306_send_byte(0x00);  // write command
}

void ssd1306_send_command_stop(void) {
  ssd1306_xfer_stop();
}

void ssd1306_send_command(uint8_t command)
{
  ssd1306_send_command_start();
  ssd1306_send_byte(command);
  ssd1306_send_command_stop();
}

void ssd1306_send_data_start(void)
{
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_SA);
  ssd1306_send_byte(0x40);  //write data
}

void ssd1306_send_data_stop(void)
{
  ssd1306_xfer_stop();
}

void ssd1306_init(void)
{
  DDRB |= (1 << SSD1306_SDA); // Set port as output
  DDRB |= (1 << SSD1306_SCL); // Set port as output
  
  for (uint8_t i = 0; i < sizeof (ssd1306_init_sequence); i++) {
    ssd1306_send_command(pgm_read_byte(&ssd1306_init_sequence[i]));
  }
}

void ssd1306_setpos(uint8_t x, uint8_t y)
{
  ssd1306_send_command_start();
  ssd1306_send_byte(0xb0 + y);
  ssd1306_send_byte(((x & 0xf0) >> 4) | 0x10); // | 0x10
/* TODO: Verify correctness */  ssd1306_send_byte((x & 0x0f)); // | 0x01
  ssd1306_send_command_stop();
}


#endif
