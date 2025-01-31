// Copyright 2022 Stefan Kerkmann
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once


#define RP2040

#define UNICODE_CYCLE_PERSIST true
#define UNICODE_KEY_WINC      KC_LEFT_ALT

#define TAP_CODE_DELAY 4

/* key matrix size */
#define MATRIX_ROWS_PER_SIDE 2
#define MATRIX_ROWS 2
#define MATRIX_COLS 3

#define DIODE_DIRECTION COL2ROW
#define LAYOUT_TO_INDEX(row, col) ((row)*MATRIX_COLS+(col))

#define RGBLED_NUM       72
#define DRIVER_LED_TOTAL RGBLED_NUM
#define RGB_MATRIX_LED_COUNT RGBLED_NUM
#define RGB_MATRIX_SPLIT { 36, 36 }

#define NUM_SHIFT_REGISTERS 1

#define I2C_DRIVER I2CD0
#define I2C1_SCL_PIN GP5
#define I2C1_SDA_PIN GP4
#define I2C1_OPMODE OPMODE_I2C
#define I2C1_CLOCK_SPEED 400000

#define OLED_INIT

#define WS2812_DI_PIN GP25

#define NOP_FUDGE 0.4

// SPI interface to write to the selected display
#define SPI_DRIVER SPID0
#define SPI_SS_PIN GP9
#define SPI_DC_PIN GP11
#define HW_RST_PIN GP10
#define SPI_SCK_PIN GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP10

//This number can be calculated by dividing the MCU’s clock speed
//by the desired SPI clock speed. For example, an MCU running at 8 MHz
//wanting to talk to an SPI device at 4 MHz would set the divisor to 2
#define SPI_DIVISOR (CPU_CLOCK / 10000000) //rp1040 runs at 133Mhz, SPI at 10Mhz

// Shift register to select the display
//#define SR_NMR_PIN //NO_PIN if possible
#define SR_CLK_PIN GP6
#define SR_DATA_PIN GP8
#define SR_LATCH_PIN GP7