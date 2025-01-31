#OLED setup
OLED_ENABLE = yes
OLED_DRIVER = ssd1306

#RGB matrix lighting
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = ws2812

#source files
QUANTUM_LIB_SRC += spi_master.c
SRC += base/com.c base/crc32.c base/text_helper.c base/helpers.c base/disp_array.c base/shift_reg.c base/spi_helper.c polymorph.c

DEFAULT_FOLDER = handwired/polymorph/morph

# Build Options
WS2812_DRIVER = vendor

#Allow raw hid communication (for bi-directional data transfer)
RAW_ENABLE = yes

#collect words per minute data
WPM_ENABLE = yes

SEND_STRING_ENABLE = yes