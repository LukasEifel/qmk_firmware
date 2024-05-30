#OLED setup
OLED_ENABLE = yes
OLED_DRIVER = ssd1306

I2C_DRIVER_REQUIRED = yes

OPT_DEFS += -DHAL_USE_I2C=TRUE

#source files
QUANTUM_LIB_SRC += i2c_master.c
SRC += base/disp_array.c polymorph.c

RAW_ENABLE = yes
WPM_ENABLE = yes
