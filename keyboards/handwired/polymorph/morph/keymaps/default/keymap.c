#include QMK_KEYBOARD_H

#include "polymorph.h"

//#include "base/ss_oled.h"
#include "base/disp_array.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    LAYOUT_ortho_1x1(KC_A)
};

void keyboard_pre_init_user(void) {
    kdisp_setup(true);

    kdisp_write_base_char(20, 20, (char)64);

    kdisp_send_buffer();
}
