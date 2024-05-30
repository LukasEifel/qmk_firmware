#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "polymorph.h"
#include "i2c_master.h"
//#include "helpers.h"
//#include "spi_helper.h"
//#include "shift_reg.h"
//#include "spi_master.h"
#include "disp_array.h"
#include "fonts/base_font.h"

const uint8_t SSD1306_MEMORYMODE = 0x20;           ///< See datasheet
const uint8_t SSD1306_COLUMNADDR = 0x21;           ///< See datasheet
const uint8_t SSD1306_PAGEADDR = 0x22;             ///< See datasheet
const uint8_t SSD1306_SETCONTRAST = 0x81;          ///< See datasheet
const uint8_t SSD1306_CHARGEPUMP = 0x8D;           ///< See datasheet
const uint8_t SSD1306_SEGREMAP = 0xA0;             ///< See datasheet
const uint8_t SSD1306_DISPLAYALLON_RESUME = 0xA4;  ///< See datasheet
const uint8_t SSD1306_DISPLAYALLON = 0xA5;         ///< Not currently used
const uint8_t SSD1306_NORMALDISPLAY = 0xA6;        ///< See datasheet
const uint8_t SSD1306_INVERTDISPLAY = 0xA7;        ///< See datasheet
const uint8_t SSD1306_SETMULTIPLEX = 0xA8;         ///< See datasheet
const uint8_t SSD1306_DISPLAYOFF = 0xAE;           ///< See datasheet
const uint8_t SSD1306_DISPLAYON = 0xAF;            ///< See datasheet
const uint8_t SSD1306_COMSCANINC = 0xC0;           ///< Not currently used
const uint8_t SSD1306_COMSCANDEC = 0xC8;           ///< See datasheet
const uint8_t SSD1306_SETDISPLAYOFFSET = 0xD3;     ///< See datasheet
const uint8_t SSD1306_SETDISPLAYCLOCKDIV = 0xD5;   ///< See datasheet
const uint8_t SSD1306_SETPRECHARGE = 0xD9;        ///< See datasheet
const uint8_t SSD1306_SETCOMPINS = 0xDA;           ///< See datasheet
const uint8_t SSD1306_SETVCOMDETECT = 0xDB;        ///< See datasheet

const uint8_t SSD1306_SETLOWCOLUMN = 0x00;   ///< Not currently used
const uint8_t SSD1306_SETHIGHCOLUMN = 0x10;  ///< Not currently used
const uint8_t SSD1306_SETSTARTLINE = 0x40;   ///< See datasheet

const uint8_t SSD1306_RIGHT_HORIZONTAL_SCROLL = 0x26;              ///< Init rt scroll
const uint8_t SSD1306_LEFT_HORIZONTAL_SCROLL = 0x27;               ///< Init left scroll
const uint8_t SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29; ///< Init diag scroll
const uint8_t SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A;  ///< Init diag scroll
const uint8_t SSD1306_DEACTIVATE_SCROLL = 0x2E;                    ///< Stop scroll
const uint8_t SSD1306_ACTIVATE_SCROLL = 0x2F;                      ///< Start scroll
const uint8_t SSD1306_SET_VERTICAL_SCROLL_AREA = 0xA3;             ///< Set scroll range

// display specific constants
#define SCREEN_WIDTH 72
#define SCREEN_HEIGHT 40
#define BUFFER_BYTE_VIS_HEIGHT 5
#define BUFFER_BYTE_HEIGHT 8
#define BUFFER_BYTE_VIS_WIDTH 72
#define BUFFER_BYTE_WIDTH 128
#define BUFFER_PIXEL_HEIGHT 40
#define BUFFER_PIXEL_WIDTH 128
//#define VISIBLE_PIXEL_X_FIRST ((BUFFER_BYTE_WIDTH - SCREEN_WIDTH) >> 1)
//#define VISIBLE_PIXEL_X_LAST_PLUS_ONE (VISIBLE_PIXEL_X_FIRST + SCREEN_WIDTH)

uint8_t i2c_address = 0x3C;

uint8_t scratch_buffer[BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT];

uint8_t* get_scratch_buffer(void) {
    return scratch_buffer;
}

int16_t get_scratch_buffer_size(void) {
    return BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT;
}

#define test_bmp_width 82
#define test_bmp_height 64

/*
const uint8_t PROGMEM test_bmp_data[] = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11111000, 0b01111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111110, 0b01111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b01111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111011, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111,
    0b11111111, 0b11111001, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11111001, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11110001, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111100, 0b01110011, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111110, 0b00111111, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b00011110, 0b00001111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11111110, 0b00011111, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111,
    0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11011111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b00011001, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b00111100, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b01111100, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11111110, 0b01111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111110, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b11101111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b11001111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b00000111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111100, 0b00000111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11110000, 0b00000011, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b10000000, 0b00000000, 0b11111110, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b10000000, 0b00000000, 0b11111100, 0b00000000, 0b00000000, 0b00000011, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b10000000, 0b00000001, 0b11111100, 0b00000000, 0b00000000, 0b00000011, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b10000000, 0b00000001, 0b11111100, 0b00000000, 0b00000000, 0b00000011, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b10000000, 0b00000001, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00011110, 0b00000000, 0b00000000,
    0b00000000, 0b00000111, 0b10000000, 0b00000001, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00011110, 0b00000000, 0b01111111, 0b11100011, 0b11110111, 0b10011111, 0b11111001, 0b11111101, 0b11100111, 0b01111000, 0b01111011, 0b11011111, 0b11000000, 0b11111111, 0b11110111, 0b11111111, 0b10111111, 0b11111101, 0b11111101, 0b11111111, 0b01111000, 0b01111011, 0b11011111, 0b11000000, 0b11111111, 0b11110111, 0b11111111, 0b10111111, 0b11111101, 0b11111101, 0b11111111, 0b01111000, 0b01111011, 0b11011111, 0b11000000, 0b11110000, 0b11110111, 0b10000111, 0b10111100, 0b00111101, 0b11100001, 0b11111111, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11110000, 0b11110111, 0b10000111, 0b10111100, 0b00111101, 0b11100001, 0b11110000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b00000000, 0b11110111, 0b10000111, 0b10000000, 0b00111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b01111111, 0b11110111, 0b10000111,
    0b10011111, 0b11111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11111111, 0b11110111, 0b10000111, 0b10111111, 0b11111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11110000, 0b11110111, 0b10000111, 0b10111100, 0b00111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11110000, 0b11110111, 0b10000111, 0b10111100, 0b00111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11110000, 0b11110111, 0b10000111, 0b10111100, 0b00111101, 0b11100001, 0b11100000, 0b01111000, 0b01111011, 0b11011110, 0b00000000, 0b11111111, 0b11110111, 0b11111111, 0b10111111, 0b11111101, 0b11100001, 0b11100000, 0b01111111, 0b11111011, 0b11011111, 0b11000000, 0b11111111, 0b11110111, 0b11111111, 0b10111111, 0b11111101, 0b11100001, 0b11100000, 0b01111111, 0b11111011, 0b11011111, 0b11000000, 0b01111100, 0b11110011, 0b11110011, 0b10011111, 0b00111101,
    0b11100001, 0b11100000, 0b00111110, 0b01111011, 0b11001111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b01101000, 0b11011011, 0b00010001, 0b00011010, 0b00110001, 0b11000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b00101011, 0b01011010, 0b11111011, 0b01101010, 0b11101111, 0b11000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b01001011, 0b01011011, 0b00111011, 0b00011010, 0b00110011, 0b11000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111101, 0b01101011, 0b01011011, 0b11011011, 0b01101010, 0b11111101, 0b11000000,
};
*/

#define GET_BUFFER_OFFSET(x, y) (((y) >> 3) * BUFFER_BYTE_WIDTH + (x))
#define WITHIN_BUFFER(x, y) ((x)>=0 && (y)>=0 && GET_BUFFER_OFFSET(x, y) < BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT)
#define SET_PIXEL(x, y) scratch_buffer[GET_BUFFER_OFFSET(x, y)] |= (1 << ((y)&0x7))
#define SET_PIXEL_CLIPPED(x, y) if(WITHIN_BUFFER(x, y)) { SET_PIXEL(x, y); }
#define COPY_TO_BUFFER_XY(unint16X, uint16Y, srcBuffer, numBytes) memcpy_P(&scratch_buffer[GET_BUFFER_OFFSET((unint16X), (uint16Y))], (srcBuffer), (numBytes))

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *font, uint16_t c) {
#ifdef __AVR__
    return &(((GFXglyph *)pgm_read_pointer(&font->glyph))[c]);
#else
    // expression in __AVR__ section may generate "dereferencing type-punned
    // pointer will break strict-aliasing rules" warning In fact, on other
    // platforms (such as STM32) there is no need to do this pointer magic as
    // program memory may be read in a usual way So expression may be simplified
    return font->glyph + c;
#endif  //__AVR__
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *font) {
#ifdef __AVR__
    return (uint8_t *)pgm_read_pointer(&font->bitmap);
#else
    // expression in __AVR__ section generates "dereferencing type-punned pointer
    // will break strict-aliasing rules" warning In fact, on other platforms (such
    // as STM32) there is no need to do this pointer magic as program memory may
    // be read in a usual way So expression may be simplified
    return font->bitmap;
#endif  //__AVR__
}

void kdisp_fill_rect(int8_t x_start, int8_t y_start, int8_t width, int8_t height) {
    for (int x = x_start; x < (x_start + width); ++x) {
        for (int y = y_start; y < (x_start + width); ++y) {
            SET_PIXEL_CLIPPED(x, y);
        }
    }
}

// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    ch  The 16-bit font-indexed character
*/
/**************************************************************************/
int8_t kdisp_write_gfx_char(const GFXfont **fonts, uint8_t num_fonts, int8_t x, int8_t y, uint16_t ch) {
    const GFXfont * currentFont = 0;
    uint16_t first = 0;
    uint16_t last = 0;
    for (uint8_t idx = 0; idx < num_fonts; ++idx) {
        currentFont = fonts[idx];
        first = pgm_read_word(&currentFont->first);
        last  = pgm_read_word(&currentFont->last);
        if (ch < first || ch > last) {
            if (idx == num_fonts - 1) {
                currentFont = fonts[0];
                first = pgm_read_word(&currentFont->first);
                last  = pgm_read_word(&currentFont->last);
                ch = u'!';
                break;
                //return 0; //no match at all
            }
        } else {
            break; // found character in range of the current font
        }
    }
    ch -= first;
    const GFXglyph *glyph  = pgm_read_glyph_ptr(currentFont, ch);
    const uint8_t  *bitmap = pgm_read_bitmap_ptr(currentFont);

    //adjust to the first font y-offset if the two fonts have different heights
    y += pgm_read_byte(&currentFont->yAdvance) - pgm_read_byte(&fonts[0]->yAdvance);

    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
    int8_t  w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
    int8_t   xo = pgm_read_byte(&glyph->xOffset), yo = pgm_read_byte(&glyph->yOffset);
    int8_t  xx, yy, bits = 0, bit = 0;

    // Todo: Add character clipping here
    for (yy = 0; yy < h; yy++) {
        for (xx = 0; xx < w; xx++) {
            if (!(bit++ & 7)) {
                bits = pgm_read_byte(&bitmap[bo++]);
            }
            if (bits & 0x80) {
                SET_PIXEL_CLIPPED(x + xo + xx, y + yo + yy);
            }
            bits <<= 1;
        }
    }

    return pgm_read_byte(&glyph->xAdvance);
}

void kdisp_write_gfx_text(const GFXfont **fonts, uint8_t num_fonts, int8_t x, int8_t y, const uint16_t *text) {
    int8_t x_cursor = x;
    int8_t y_cursor = y;
    while (*text != 0) {
        switch(*text) {
            case u'\b':
                x_cursor = x_cursor>1 ? x_cursor - 2 : 0;
                break;
            case u'\t':
                x_cursor += ((x_cursor-x)/40+1)*40;
                break;
            case u'\n':
                y_cursor += pgm_read_byte(&fonts[0]->yAdvance);
                x_cursor = x;
                break;
            case u'\v':
                y_cursor += ((y_cursor-y)/15+1)*15;
                break;
            case u'\r':
                x_cursor = x;
                break;
            default:
                x_cursor += kdisp_write_gfx_char(fonts, num_fonts, x_cursor, y_cursor, *text);
                break;
        }
        text++;
    }
}

void kdisp_write_base_char(int8_t x, int8_t y, const char ch) {
    int8_t font_index = (uint8_t)ch;  // font based on unsigned type for index
    if (font_index < BASIC_FONT_START || font_index > BASIC_FONT_END) {
        memset(&scratch_buffer[GET_BUFFER_OFFSET(x, y)], 0x00, BASIC_FONT_WIDTH);
    } else {
        const uint8_t *glyph = &font[(font_index - BASIC_FONT_START) * BASIC_FONT_WIDTH];
        COPY_TO_BUFFER_XY(x, y, glyph, BASIC_FONT_WIDTH);
    }
}

void kdisp_draw_bitmap(int8_t x, int8_t y, const uint8_t pgm_bmp[], int8_t bmp_width, int8_t bmp_height) {
    int8_t byte_width           = (bmp_width + 7) / 8;
    uint8_t vertical_pixel_row_8 = 0;

    for (int8_t bmp_y = 0; bmp_y < bmp_height; bmp_y++, y++) {
        for (int8_t bmp_x = 0; bmp_x < bmp_width; bmp_x++) {
            if (bmp_x & 0x07) {
                vertical_pixel_row_8 <<= 1;
            } else {
                vertical_pixel_row_8 = pgm_read_byte(&pgm_bmp[bmp_y * byte_width + (bmp_x >> 3)]);
            }
            if (vertical_pixel_row_8 & 0x80) {
                SET_PIXEL_CLIPPED(x + bmp_x, y);
            }
        }
    }
}

void kdisp_set_buffer(uint8_t vertical_pixel_row_of_8_pixels) { memset(scratch_buffer, vertical_pixel_row_of_8_pixels, BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT); }

void kdisp_send_buffer(void) {
    //spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);

    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);


    static const uint8_t dlist1[] = {SSD1306_PAGEADDR,
                                             0,                       // Page start address
                                             0xFF,                    // Page end (not really, but works here)
                                             SSD1306_COLUMNADDR, 0};  // Column start address
    i2c_transmit(i2c_address, dlist1, sizeof(dlist1), 50);

    static uint8_t bufferwiidth[] = {BUFFER_PIXEL_WIDTH - 1};
    i2c_transmit(i2c_address, bufferwiidth, sizeof(bufferwiidth), 50);

    //spi_prepare_data();

    static uint16_t buffer = BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT;
    //buffer[0] = BUFFER_BYTE_WIDTH * BUFFER_BYTE_HEIGHT;
    i2c_transmit(i2c_address, (uint8_t *)&buffer, 2, 50);

    //spi_stop();
}

void kdisp_invert(bool invert) {
    //spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);
    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static uint8_t inverted[1];
    if (invert) {
        inverted[0] = SSD1306_INVERTDISPLAY;
    } else {
        inverted[0] = SSD1306_NORMALDISPLAY;
    }
    i2c_transmit(i2c_address, inverted, sizeof(inverted), 50);
    //spi_stop();
}

void kdisp_scroll_vlines(uint8_t lines0to63) {
    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static const uint8_t vertical[] = {SSD1306_SET_VERTICAL_SCROLL_AREA};
    i2c_transmit(i2c_address, vertical, sizeof(vertical), 50);
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static uint8_t lines[1];
    lines[0] = lines0to63;
    i2c_transmit(i2c_address, lines, sizeof(lines), 50);
}

void kdisp_scroll(bool activate) {
    //spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);
    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static uint8_t scrolling[1];
    if (activate) {
        scrolling[0] = SSD1306_ACTIVATE_SCROLL;
    } else {
        scrolling[0] = SSD1306_DEACTIVATE_SCROLL;
    }
    i2c_transmit(i2c_address, scrolling, sizeof(scrolling), 50);
    //spi_stop();
}

//also setup the lines to scroll via kdisp_scroll_vlines
void kdisp_scroll_modeh(bool left, uint8_t hspeed0to7) {
    // static const uint8_t PROGMEM zero[] = {0x0};
    // i2c_transmit(0x3C, zero, sizeof(zero), 50);
//
    // if(left) {
    //     spi_write(SSD1306_LEFT_HORIZONTAL_SCROLL);
    // } else {
    //     spi_write(SSD1306_RIGHT_HORIZONTAL_SCROLL);
    // }
    // spi_write(0); //dummy
    // spi_write(0); //start page
    // switch(hspeed0to7) {
    //     case 0: spi_write(7); break; //2
    //     case 1: spi_write(4); break; //3
    //     case 2: spi_write(5); break; //4
    //     case 3: spi_write(0); break; //5
    //     case 4: spi_write(6); break; //25
    //     case 5: spi_write(1); break; //64
    //     case 6: spi_write(2); break; //128
    //     default: spi_write(3); break; //256
    // }
//
    // spi_write(0x05); //end page, maybe as param?
    // spi_write(0); //dummy
    // spi_write(0xff); //dummy
}

//also setup the lines to scroll via kdisp_scroll_vlines
void kdisp_scroll_modehv(bool left, uint8_t hspeed0to7, uint8_t voffset0to63) {
    // static const uint8_t PROGMEM zero[] = {0x0};
    // i2c_transmit(0x3C, zero, sizeof(zero), 50);
//
    // if(left) {
    //     spi_write(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
    // } else {
    //     spi_write(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
    // }
    // spi_write(0); //dummy
    // spi_write(0); //start page
    // switch(hspeed0to7) {
    //     case 0: spi_write(7); break; //2
    //     case 1: spi_write(4); break; //3
    //     case 2: spi_write(5); break; //4
    //     case 3: spi_write(0); break; //5
    //     case 4: spi_write(6); break; //25
    //     case 5: spi_write(1); break; //64
    //     case 6: spi_write(2); break; //128
    //     default: spi_write(3); break; //256
    // }
//
    // spi_write(0x05); //end page, maybe as param?
    // spi_write(voffset0to63&63);
}

void kdisp_set_contrast(uint8_t contrast) {
    //spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);
    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static const uint8_t contrastcmd[] = {SSD1306_SETCONTRAST};
    i2c_transmit(i2c_address, contrastcmd, sizeof(contrastcmd), 50);
    static uint8_t value[1];
    value[0] = contrast;
    i2c_transmit(i2c_address, value, sizeof(value), 50);
    //spi_stop();
}

void kdisp_enable(bool enable) {
    //spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);
    static const uint8_t PROGMEM zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static uint8_t sendenable[1];
    if (enable) {
        sendenable[0] = SSD1306_DISPLAYON;
    } else {
        sendenable[0] = SSD1306_DISPLAYOFF;
    }
    i2c_transmit(i2c_address, sendenable, sizeof(sendenable), 50);
    //spi_stop();
}

void kdisp_hw_setup(void) {
    // //make sure the pins are output pins and low
    // #if defined(KEY_DISPLAYS_VDD_PIN)
    //     setPinOutput(KEY_DISPLAYS_VDD_PIN);
    //     writePinLow(KEY_DISPLAYS_VDD_PIN);
    // #endif

    // #if defined(KEY_DISPLAYS_VBAT_PIN)
    //     setPinOutput(KEY_DISPLAYS_VBAT_PIN);
    //     writePinLow(KEY_DISPLAYS_VBAT_PIN);
    // #endif

    // sr_hw_setup();
}

void kdisp_init(const int8_t num_shift_regs) {
    // // first turn on logic power supply
    // #if defined(KEY_DISPLAYS_VDD_PIN)
    //     setPinOutput(KEY_DISPLAYS_VDD_PIN);
    //     writePinHigh(KEY_DISPLAYS_VDD_PIN);
    //     wait_ms(5);
    // #endif

    // // and then the power supply for the displays
    // #if defined(KEY_DISPLAYS_VBAT_PIN)
    //     setPinOutput(KEY_DISPLAYS_VBAT_PIN);
    //     writePinHigh(KEY_DISPLAYS_VBAT_PIN);
    // #endif

    // sr_init();

    // //make sure we are talking to all shift registers
    // uint8_t all[num_shift_regs];
    // for(int8_t i=0;i<num_shift_regs;++i) {
    //     all[i] = 0;
    // }
    // sr_shift_out_buffer_latch(all, num_shift_regs);


    // spi_init();
    // spi_start(SPI_SS_PIN, false, SPI_MODE, SPI_DIVISOR);

    // peripherals_reset();
}

const unsigned char oled72_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
      0xda,0x12,0x81,0xff,0xad,0x30,0xd9,0xf1,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
      0xaf,0x20,0x02};

void kdisp_setup(bool turn_on) {
    i2c_init();

    static const uint8_t zero[] = {0x0};
    i2c_transmit(i2c_address, zero, sizeof(zero), 50);

    static const uint8_t displayoff[] = {SSD1306_DISPLAYOFF};
    i2c_transmit(i2c_address, displayoff, sizeof(displayoff), 50);
    static const uint8_t clockDiv[] = {SSD1306_SETDISPLAYCLOCKDIV, 0x80};
    i2c_transmit(i2c_address, clockDiv, sizeof(clockDiv), 50);
    static const uint8_t dispOffset[] = {SSD1306_SETDISPLAYOFFSET, 0x00};
    i2c_transmit(i2c_address, dispOffset, sizeof(dispOffset), 50);
    static const uint8_t startLine[] = {SSD1306_SETSTARTLINE | 0x0};
    i2c_transmit(i2c_address, startLine, sizeof(startLine), 50);
    static const uint8_t displayallonresume[] = {SSD1306_DISPLAYALLON_RESUME};
    i2c_transmit(i2c_address, displayallonresume, sizeof(displayallonresume), 50);

    static const uint8_t normaldisplay[] = {SSD1306_NORMALDISPLAY};
    i2c_transmit(i2c_address, normaldisplay, sizeof(normaldisplay), 50);

    static const uint8_t chargePump[] = {SSD1306_CHARGEPUMP, 0x95};  // 0x14?
    i2c_transmit(i2c_address, chargePump, sizeof(chargePump), 50);
    static const uint8_t memMode[] = {SSD1306_MEMORYMODE, 0x00};
    i2c_transmit(i2c_address, memMode, sizeof(memMode), 50);
    static const uint8_t segremap[] = {SSD1306_SEGREMAP | 0x1};
    i2c_transmit(i2c_address, segremap, sizeof(segremap), 50);
    static const uint8_t comscandec[] = {SSD1306_COMSCANDEC};
    i2c_transmit(i2c_address, comscandec, sizeof(comscandec), 50);
    static const uint8_t contrast[] = {SSD1306_SETCONTRAST, 0x00};
    i2c_transmit(i2c_address, contrast, sizeof(contrast), 50);
    static const uint8_t precharge[] = {SSD1306_SETPRECHARGE, 0x22};
    i2c_transmit(i2c_address, precharge, sizeof(precharge), 50);
    static const uint8_t vCom[] = {SSD1306_SETVCOMDETECT, 0x20};
    i2c_transmit(i2c_address, vCom, sizeof(vCom), 50);
    static const uint8_t multiPlex[] = {SSD1306_SETMULTIPLEX, 40 - 1};
    i2c_transmit(i2c_address, multiPlex, sizeof(multiPlex), 50);
    static const uint8_t compins[] = {SSD1306_SETCOMPINS, 0x12};
    i2c_transmit(i2c_address, compins, sizeof(compins), 50);

    static const uint8_t fin[] = {0xad, 0x30};
    i2c_transmit(i2c_address, fin, sizeof(fin), 50);

    if(turn_on) {
        static const uint8_t displayon[] = {SSD1306_DISPLAYON};
        i2c_transmit(i2c_address, displayon, sizeof(displayon), 50);
    }

    kdisp_write_base_char(30, 30, 'A');
    kdisp_send_buffer();

    //spi_stop();
}
