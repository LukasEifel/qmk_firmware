#include QMK_KEYBOARD_H

#include "polymorph.h"
// #include "morph/morph.h"
#include "morph/keymaps/default/uni.h"

#include "base/com.h"
#include "base/disp_array.h"
#include "base/helpers.h"
#include "base/spi_helper.h"
#include "base/shift_reg.h"
#include "base/text_helper.h"
#include "base/fonts/NotoSans_Regular_Base_11pt.h"
#include "base/fonts/NotoSans_Medium_Base_8pt.h"
#include "base/fonts/gfx_used_fonts.h"

#include "base/crc32.h"

#include "quantum/quantum_keycodes.h"
#include "quantum/keymap_extras/keymap_german.h"
#include "quantum/via.h"

#include "raw_hid.h"
#include "oled_driver.h"
#include "version.h"
#include "print.h"
#include "debug.h"



// #include <transactions.h>
#include <hardware/flash.h>

#include "lang/lang_lut.h"

#define FW_VERSION "0.5.2"

#define FLASH_TARGET_OFFSET (4 * 1024 * 1024) //we start at 4MB and use the remaining 4MB for resource data
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static_assert(FLASH_PAGE_SIZE==256, "Flash page size changed");

#define FULL_BRIGHT 50
#define MIN_BRIGHT 1
#define DISP_OFF 0
#define BRIGHT_STEP 1

#define EMJ(x) UM(PRIVATE_EMOJI_##x)

//10 sec
#define FADE_TRANSITION_TIME 10000
//2 min
#define FADE_OUT_TIME 120000
//10 min
#define TURN_OFF_TIME 1200000

/*[[[cog
import cog
import os
from textwrap import wrap
from openpyxl import load_workbook
wb = load_workbook(filename = os.path.join(os.path.abspath(os.path.dirname(cog.inFile)), "lang", "lang_lut.xlsx"))
sheet = wb['key_lut']

languages = []
lang_index = 0
lang_key = sheet["B1"].value
while lang_key:
    languages.append(lang_key)
    if lang_index==0:
        cog.outl(f"static enum lang_layer g_lang_init = {lang_key};")
    lang_index = lang_index + 1
    lang_key = sheet.cell(row = 1, column = 2 + lang_index*3).value
]]]*/
static enum lang_layer g_lang_init = LANG_EN;
//[[[end]]]

enum kb_layers {
    _BL = 0x00,
    _L0 = _BL,
    _L1,
    _L2,
    _L3,
    _L4,
    _FL0,
    _FL1,
    _NL,
    _UL,
    _SL,
    _LL,
    _ADDLANG1,
    _EMJ0,
    _EMJ1 };

enum my_keycodes {
    KC_LANG = QK_KB_0,
    KC_DMIN,
    KC_DMAX,
    KC_DDIM,
    KC_DBRI,
    KC_DHLF,
    KC_D1Q,
    KC_D3Q,
    KC_BASE,
    KC_L0,
    KC_L1,
    KC_L2,
    KC_L3,
    KC_L4,
    KC_DEADKEY,
    KC_TOGMODS,
    KC_TOGTEXT,
    KC_RGB_TOG,
    /*[[[cog
        for lang in languages:
            if lang == "LANG_EN":
                cog.out(f"KC_LANG_EN = SAFE_RANGE, ")
            else:
                cog.out(f"KC_{lang}, ")
    ]]]*/
    KC_LANG_EN = SAFE_RANGE, KC_LANG_DE, KC_LANG_FR, KC_LANG_ES, KC_LANG_PT, KC_LANG_IT, KC_LANG_TR, KC_LANG_KO, KC_LANG_JA, KC_LANG_AR, KC_LANG_GR,
    //[[[end]]]
    /*[[[cog
      for idx in range(10):
          cog.out(f"KC_LAT{idx}, ")
    ]]]*/
    KC_LAT0, KC_LAT1, KC_LAT2, KC_LAT3, KC_LAT4, KC_LAT5, KC_LAT6, KC_LAT7, KC_LAT8, KC_LAT9,
    //[[[end]]]
    //Lables, no functionality:
    LBL_TEXT
};
static_assert((int)KC_RGB_TOG<=(int)QK_KB_31, "Too many custom QK key codes");
static_assert((int)LBL_TEXT<=(int)QK_USER_31, "Too many user custom key codes");

struct display_info {
    uint8_t bitmask[NUM_SHIFT_REGISTERS];
};

#define BITMASK1(x) .bitmask = {(uint8_t)(~(1<<x))}

const struct display_info key_display[] = {
        {BITMASK1(0)}, {BITMASK1(1)}, {BITMASK1(2)},
        {BITMASK1(3)}, {BITMASK1(4)}, {BITMASK1(5)}
};

const struct display_info disp_row_0 = { BITMASK1(0) };

enum refresh_mode { START_FIRST_HALF, START_SECOND_HALF, DONE_ALL, ALL_AT_ONCE };
static enum refresh_mode g_refresh = DONE_ALL;

enum com_state { NOT_INITIALIZED, USB_HOST, BRIDGE };
static enum com_state com = NOT_INITIALIZED;

bool is_usb_host_side(void) {
    return com == USB_HOST;
}

enum side_state { UNDECIDED, LEFT_SIDE, RIGHT_SIDE };
static enum side_state side = UNDECIDED;

bool is_left_side(void) {
    return side == LEFT_SIDE;
}

bool is_right_side(void) {
    return side == RIGHT_SIDE;
}
typedef struct _poly_eeconf_t {
    uint8_t lang;
    uint8_t brightness;
    uint16_t unused;
    uint8_t latin_ex[26];
} poly_eeconf_t;


static_assert(sizeof(poly_eeconf_t) == EECONFIG_USER_DATA_SIZE, "Mismatch in keyboard EECONFIG stored data");

typedef struct _poly_layer_t {
    uint32_t crc32;
    layer_state_t layer;
    layer_state_t def_layer;
    led_t led_state;
    uint8_t mods;
} poly_layer_t;
static poly_layer_t l_layer;
static poly_layer_t g_layer;

typedef struct _poly_sync_t {
    uint32_t crc32;
    uint8_t lang;
    uint8_t contrast;
    uint8_t flags;
    uint8_t overlay_flags;
    uint8_t unicode_mode;
} poly_sync_t;
static poly_sync_t l_state;
static poly_sync_t g_state;

typedef struct _poly_last_t {
    uint32_t crc32;
    uint16_t latin_kc;
} poly_last_t;
static poly_last_t l_last;
static poly_last_t g_last;

typedef struct _latin_sync_t {
    uint32_t crc32;
    uint8_t ex[26];
} latin_sync_t;

static latin_sync_t g_latin;

#define SYNC_ACK 0b11001010
typedef struct _poly_sync_reply_t {
    uint8_t ack;
} poly_sync_reply_t;


static int32_t last_update = 0;

#define NUM_SEGMENTS_PER_OVERLAY 15
#define BYTES_PER_SEGMENT 24
#define NUM_OVERLAYS 90
#define NUM_VARIATIONS 7 // NO_MOD(0), CTRL(1), SHIFT(2), CTRL_SHIFT(3), ALT(4), CTRL_ALT(5), ALT_SHIFT(6), Not supported: CTRL_ALT_SHIFT(7) GUI_KEY(8)
static uint8_t use_overlay[NUM_OVERLAYS*NUM_VARIATIONS];
static uint8_t overlays [NUM_OVERLAYS*NUM_VARIATIONS][72*40/8]; // ResX*ResY/PixelPerByte

void reset_overlay_buffers(void) {
    memset(&use_overlay, 0, sizeof(use_overlay));
}

typedef struct _overlay_sync_t {
    uint32_t crc32;
    uint16_t adj_idx;
    uint8_t segment;
    uint8_t overlay[BYTES_PER_SEGMENT];
} overlay_sync_t;


bool display_wakeup(keyrecord_t* record);
void update_displays(enum refresh_mode mode);
void set_displays(uint8_t contrast, bool idle);
void set_selected_displays(int8_t old_value, int8_t new_value);
void toggle_stagger(bool new_state);
void oled_update_buffer(void);
void poly_suspend(void);

void save_user_eeconf(void) {
    poly_eeconf_t ee;
    ee.lang = l_state.lang;
    ee.brightness = ~l_state.contrast;
    ee.unused = 0;
    memcpy(ee.latin_ex, g_latin.ex, sizeof(g_latin.ex));
    eeconfig_update_user_datablock(&ee);
}

poly_eeconf_t load_user_eeconf(void) {
    poly_eeconf_t ee;
    eeconfig_read_user_datablock(&ee);
    ee.brightness = ~ee.brightness;
    if(ee.brightness>FULL_BRIGHT) {
        ee.brightness = FULL_BRIGHT;
    }
    return ee;
}

void inc_brightness(void) {
    if (l_state.contrast < FULL_BRIGHT) {
        l_state.contrast += BRIGHT_STEP;
    }
    if (l_state.contrast > FULL_BRIGHT) {
        l_state.contrast = FULL_BRIGHT;
    }

    save_user_eeconf();
}

void dec_brightness(void) {
    if (l_state.contrast > (MIN_BRIGHT+BRIGHT_STEP)) {
        l_state.contrast -= BRIGHT_STEP;
    } else {
        l_state.contrast = MIN_BRIGHT;
    }

    save_user_eeconf();
}

void select_all_displays(void) {
    // make sure we are talking to all shift registers
    sr_shift_out_0_latch(NUM_SHIFT_REGISTERS);
}

void clear_all_displays(void) {
    select_all_displays();

    kdisp_set_buffer(0x00);
    kdisp_send_buffer();
}

void early_hardware_init_post(void) {
    spi_hw_setup();
}

void update_performed(void) {
    last_update = timer_read32();
}

layer_state_t persistent_default_layer_get(void) {
    return (layer_state_t)eeconfig_read_default_layer();
}

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}

void request_disp_refresh(void) {
    g_refresh = ALL_AT_ONCE;
    //use the following for partial update (during housekeeping)
    // g_refresh = START_FIRST_HALF;
}

//
// __/\\\\\\\\\\\\______________________________________________________________/\\\\\\\\\\\_____________________________________________
//  _\/\\\////////\\\__________________________________________________________/\\\/////////\\\___________________________________________
//   _\/\\\______\//\\\____________________/\\\________________________________\//\\\______\///_____/\\\__/\\\_____________________________
//    _\/\\\_______\/\\\__/\\\\\\\\\_____/\\\\\\\\\\\__/\\\\\\\\\________________\////\\\___________\//\\\/\\\___/\\/\\\\\\_______/\\\\\\\\_
//     _\/\\\_______\/\\\_\////////\\\___\////\\\////__\////////\\\__________________\////\\\_________\//\\\\\___\/\\\////\\\____/\\\//////__
//      _\/\\\_______\/\\\___/\\\\\\\\\\_____\/\\\________/\\\\\\\\\\____________________\////\\\_______\//\\\____\/\\\__\//\\\__/\\\_________
//       _\/\\\_______/\\\___/\\\/////\\\_____\/\\\_/\\___/\\\/////\\\_____________/\\\______\//\\\___/\\_/\\\_____\/\\\___\/\\\_\//\\\________
//        _\/\\\\\\\\\\\\/___\//\\\\\\\\/\\____\//\\\\\___\//\\\\\\\\/\\___________\///\\\\\\\\\\\/___\//\\\\/______\/\\\___\/\\\__\///\\\\\\\\_
//         _\////////////______\////////\//______\/////_____\////////\//______________\///////////______\////________\///____\///_____\////////__
//

void user_sync_poly_data_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    if (in_len == sizeof(poly_sync_t) && in_data != NULL && out_len == sizeof(poly_sync_reply_t) && out_data!= NULL) {
        uint32_t crc32 = crc32_1byte(&((uint8_t *)in_data)[4], in_len-4, 0);
        if(crc32 == ((const poly_sync_t *)in_data)->crc32) {
            memcpy(&l_state, in_data, sizeof(poly_sync_t));
            ((poly_sync_reply_t*)out_data)->ack = SYNC_ACK;
            //request_disp_refresh();
        }
    }
}

void user_sync_latin_ex_data_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    if (in_len == sizeof(latin_sync_t) && in_data != NULL && out_len == sizeof(poly_sync_reply_t) && out_data!= NULL) {
        uint32_t crc32 = crc32_1byte(&((uint8_t *)in_data)[4], in_len-4, 0);
        if(crc32 == ((const latin_sync_t *)in_data)->crc32) {
            memcpy(&g_latin, in_data, sizeof(latin_sync_t));
            ((poly_sync_reply_t*)out_data)->ack = SYNC_ACK;
            save_user_eeconf();
            request_disp_refresh();
        }
    }
}

void user_sync_lastkey_data_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    if (in_len == sizeof(poly_last_t) && in_data != NULL && out_len == sizeof(poly_sync_reply_t) && out_data!= NULL) {
        uint32_t crc32 = crc32_1byte(&((uint8_t *)in_data)[4], in_len-4, 0);
        if(crc32 == ((const poly_last_t *)in_data)->crc32) {
            memcpy(&l_last, in_data, sizeof(poly_last_t));
            memcpy(&g_last, in_data, sizeof(poly_last_t));
            ((poly_sync_reply_t*)out_data)->ack = SYNC_ACK;
            request_disp_refresh();
        }
    }
}

void user_sync_layer_data_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    if (in_len == sizeof(poly_layer_t) && in_data != NULL && out_len == sizeof(poly_sync_reply_t) && out_data!= NULL) {
        uint32_t crc32 = crc32_1byte(&((uint8_t *)in_data)[4], in_len-4, 0);
        if(crc32 == ((const poly_layer_t *)in_data)->crc32) {
            memcpy(&l_layer, in_data, sizeof(poly_layer_t));
            ((poly_sync_reply_t*)out_data)->ack = SYNC_ACK;
            //request_disp_refresh();
        }
    }
}

void user_sync_overlay_data_handler(uint8_t in_len, const void* in_data, uint8_t out_len, void* out_data) {
    if (in_len == sizeof(overlay_sync_t) && in_data != NULL && out_len == sizeof(poly_sync_reply_t) && out_data!= NULL) {
        uint32_t crc32 = crc32_1byte(&((uint8_t *)in_data)[4], in_len-4, 0);
        const overlay_sync_t* ov = ((const overlay_sync_t *)in_data);
        if(crc32 == ov->crc32) {
            memcpy(overlays[ov->adj_idx] + ov->segment*BYTES_PER_SEGMENT, ov->overlay, BYTES_PER_SEGMENT);
            if(ov->segment==NUM_SEGMENTS_PER_OVERLAY-1) {
                use_overlay[ov->adj_idx] = true;
                request_disp_refresh();
            }
            ((poly_sync_reply_t*)out_data)->ack = SYNC_ACK;
        }
    }
}

//void oled_on_off(bool on);

const char* tid_to_str(int8_t tid) {
    switch (tid)
    {
    // case USER_SYNC_LAYER_DATA: return "UserLayer";
    // case USER_SYNC_POLY_DATA:  return "UserPoly";
    // case USER_SYNC_LASTKEY_DATA: return "UserLastKey";
    // case USER_SYNC_LATIN_EX_DATA: return "UserLatinEx";

    default: return "";
    }
}

// bool send_to_bridge(int8_t tid, void* buffer_with4crc_bytes, const uint8_t num_bytes, const uint8_t retries) {
//     poly_sync_reply_t reply;
//     bool sync_success = false;
//     uint8_t retry = 0;
//     *((uint32_t *)buffer_with4crc_bytes) = crc32_1byte(&((uint8_t *)buffer_with4crc_bytes)[4], num_bytes-4, 0);
//     for(; retry<retries; ++retry) {
//         sync_success = transaction_rpc_exec(tid, num_bytes, buffer_with4crc_bytes, sizeof(poly_sync_reply_t), &reply);
//         if(sync_success && reply.ack == SYNC_ACK) {
//             break;
//         }
//         sync_success = false;
//         if(debug_enable) {
//             uprintf("Bridge sync retry %d (tid: %s, success: %d, ack: %d, bytes: %d)\n", retry, tid_to_str(tid), sync_success, reply.ack == SYNC_ACK, num_bytes);
//         }
//     }
//     if(debug_enable && retry>0) {
//         if(sync_success)
//             uprintf("Success on retry %d (tid: %s, success: %d, ack: %d, bytes: %d)\n", retry, tid_to_str(tid), sync_success, reply.ack == SYNC_ACK, num_bytes);
//         else
//             uprintf("Failed to sync %d bytes for transaction %s!\n", num_bytes, tid_to_str(tid));
//     }

//     return sync_success;
// }

bool differ(const void* b1, const void* b2, uint8_t byte_count) {
    return memcmp(&((const uint8_t *)b1)[4], &((const uint8_t *)b2)[4], byte_count - 4) != 0; // start after crc32
}

#define BYTE_TO_BINARY_PATTERN "|%s%s%s%s%s%s%s%s"
#define BYTE_TO_FLAGS(byte)  \
  ((byte) & 0x80 ? " RGB |" : " --- |"), \
  ((byte) & 0x40 ? "Txt2 |" : " --- |"), \
  ((byte) & 0x20 ? "Txt1 |" : " --- |"), \
  ((byte) & 0x10 ? " Dbg |" : " --- |"), \
  ((byte) & 0x08 ? "DeadK|" : " --- |"), \
  ((byte) & 0x04 ? "Idle |" : " --- |"), \
  ((byte) & 0x02 ? "Trans|" : " --- |"), \
  ((byte) & 0x01 ? "StatD|" : " --- |")

  #define BYTE_TO_OVERLAY_FLAGS(byte)  \
  ((byte) & 0x80 ? "  1  |" : " --- |"), \
  ((byte) & 0x40 ? "  1  |" : " --- |"), \
  ((byte) & 0x20 ? "Reset|" : " --- |"), \
  ((byte) & 0x10 ? "ClrRB|" : " --- |"), \
  ((byte) & 0x08 ? "ClrRT|" : " --- |"), \
  ((byte) & 0x04 ? "ClrLB|" : " --- |"), \
  ((byte) & 0x02 ? "ClrLT|" : " --- |"), \
  ((byte) & 0x01 ? "Disp |" : " --- |")

//helpers
static uint8_t flags = 0;
static uint8_t overlay_flags = 0;

void sync_and_refresh_displays(void) {
    bool layer_diff = false;
    bool state_diff = false;

    if (is_usb_host_side()) {
        const bool back_from_idle_transition = flag_turned_on(l_state.flags, g_state.flags, IDLE_TRANSITION);
        if (back_from_idle_transition) {
            poly_eeconf_t ee   = load_user_eeconf();
            l_state.contrast = ee.brightness;
        }

        if(flags!=l_state.flags) {
            uprintf("Poly State Flags: 0x%02x " BYTE_TO_BINARY_PATTERN "\n", l_state.flags, BYTE_TO_FLAGS(l_state.flags));
            flags=l_state.flags;
        }
        if(overlay_flags!=l_state.overlay_flags) {
            uprintf("Poly Ovrly Flags: 0x%02x " BYTE_TO_BINARY_PATTERN "\n", l_state.overlay_flags, BYTE_TO_OVERLAY_FLAGS(l_state.overlay_flags));
            overlay_flags=l_state.overlay_flags;
        }

        l_state.flags = set_flag(l_state.flags, DBG_ON, debug_enable);
        state_diff = differ(&l_state, &g_state, sizeof(poly_sync_t));
        if ( state_diff ) {
            // send_to_bridge(USER_SYNC_POLY_DATA, (void *)&l_state, sizeof(l_state), 10);
        }

        l_layer.led_state = host_keyboard_led_state();
        l_layer.mods = get_mods();
        layer_diff = differ(&l_layer, &g_layer, sizeof(poly_layer_t));
        if ( layer_diff ) {
            // send_to_bridge(USER_SYNC_LAYER_DATA, (void *)&l_layer, sizeof(l_layer), 10);
        }
        if ( differ(&l_last, &g_last, sizeof(poly_last_t)) ) {
            //  send_to_bridge(USER_SYNC_LASTKEY_DATA, (void *)&l_last, sizeof(l_last), 5);
             memcpy(&g_last, &l_last, sizeof(l_last));
        }
    } else {
        layer_diff = differ(&l_layer, &g_layer, sizeof(poly_layer_t));
        state_diff = differ(&l_state, &g_state, sizeof(poly_sync_t));
    }

    const bool in_idle_mode = (l_state.flags & DISP_IDLE) != 0;

    if(state_diff) {
        const bool idle_changed         = has_flag_changed(l_state.flags, g_state.flags, DISP_IDLE);
        const bool contrast_changed     = g_state.contrast != l_state.contrast;
        const bool status_disp_changed  = has_flag_changed(l_state.flags, g_state.flags, STATUS_DISP_ON);
        const bool status_disp_on       = test_flag(l_state.flags, STATUS_DISP_ON);
        const bool overlays_changed     = has_flag_changed(l_state.overlay_flags, g_state.overlay_flags, DISPLAY_OVERLAYS);
        const bool reset_overlays       = test_flag(l_state.overlay_flags, RESET_BUFFERS);
        const bool debug_changed        = has_flag_changed(l_state.flags, g_state.flags, DBG_ON);

        if(idle_changed) {
            if(in_idle_mode) {
                oled_set_brightness(0);
            }
        }

        //bool restored = false;
        if(status_disp_changed && status_disp_on) {
            // rgb_matrix_reload_from_eeprom();
            // if(rgb_matrix_is_enabled()) {
            //     l_state.flags = flag_on(l_state.flags, RGB_ON);
            //     restored = true;
            // }
            oled_set_brightness(OLED_BRIGHTNESS);
        }

        if(has_flag_changed(l_state.flags, g_state.flags, RGB_ON)) {
            if (test_flag(l_state.flags, RGB_ON)) {
                // if(restored) {
                //     rgb_matrix_enable_noeeprom();
                // } else {
                    rgb_matrix_enable();
                //}
            } else {
                //  if(!status_disp_on) {
                //     rgb_matrix_disable_noeeprom();
                // } else {
                    rgb_matrix_disable();
                //}
            }
        }

        if(reset_overlays) {
            reset_overlay_buffers();
            l_state.overlay_flags = flag_off(l_state.overlay_flags, RESET_BUFFERS);
        }

        if( debug_changed || overlays_changed || reset_overlays) {
            request_disp_refresh();
            update_performed();
        }

        if (contrast_changed || idle_changed) {
            set_displays(l_state.contrast, in_idle_mode);
        }

        memcpy(&g_state, &l_state, sizeof(l_state));
    }

    if(layer_diff) {
        memcpy(&g_layer, &l_layer, sizeof(l_layer));
        request_disp_refresh();
    }

    if (g_refresh == START_FIRST_HALF) {
        update_displays(START_FIRST_HALF);
        g_refresh = START_SECOND_HALF;
    }
    else if (g_refresh == START_SECOND_HALF || g_refresh == ALL_AT_ONCE) {
        update_displays(g_refresh);
        g_refresh = DONE_ALL;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    l_layer.layer = state;
    return state;
}

void housekeeping_task_user(void) {
    sync_and_refresh_displays();

    if(last_update>=0) {
        //turn off displays
        uint32_t elapsed_time_since_update = timer_elapsed32(last_update);

        if (is_usb_host_side()) {
            l_state.flags |= STATUS_DISP_ON;
            l_state.flags &= ~((uint8_t)IDLE_TRANSITION);

            if(elapsed_time_since_update > FADE_OUT_TIME && l_state.contrast >= MIN_BRIGHT && (l_state.flags & DISP_IDLE)==0) {
                poly_eeconf_t ee = load_user_eeconf();
                int32_t time_after = elapsed_time_since_update - FADE_OUT_TIME;
                int16_t brightness = ((FADE_TRANSITION_TIME - time_after) * ee.brightness) / FADE_TRANSITION_TIME;

                //transition to pulsing mode
                if(brightness<=MIN_BRIGHT) {
                    l_state.contrast = DISP_OFF;
                    l_state.flags |= DISP_IDLE;
                    uprint("Transition to pulsing\n");
                } else if(brightness>FULL_BRIGHT) {
                    l_state.contrast = FULL_BRIGHT;
                    uprint("Limiting brightness\n");
                } else{
                    l_state.contrast = brightness;
                }
                l_state.flags |= IDLE_TRANSITION;
            } else if(elapsed_time_since_update > TURN_OFF_TIME) {
                uprint("Turning off\n");
                poly_suspend();
                last_update=-1;
            } else if((l_state.flags & DISP_IDLE)!=0) {
                int32_t time_after = PK_MAX(elapsed_time_since_update - FADE_OUT_TIME - FADE_TRANSITION_TIME, 0)/300;
                l_state.contrast = time_after%50;
            } else {
                l_state.flags &= ~((uint8_t)DISP_IDLE);
            }
        }
    }
}

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT(
        KC_ESC, KC_1, KC_2,
        KC_3, KC_4, KC_5
    )
};


#define LX(x,y) ((x)/2),y
led_config_t g_led_config = { {// Key Matrix to LED Index
                              {NO_LED, NO_LED, NO_LED},
                              {NO_LED, NO_LED, NO_LED}
                             },
                             {
                                // LED Index to Physical Position
                                                {LX(144, 9)},   {LX(129, 9)},   {LX(104, 5)},
                                                {LX(144, 33)},  {LX(129, 33)},  {LX(104, 19)}
                             },
                             {
                                 // LED Index to Flag
                                 4, 4, 4,
                                 4, 4, 4
                             } };

const uint16_t* keycode_to_disp_text(uint16_t keycode, led_t state) {
    switch (keycode) {
    /*[[[cog
    wb = load_workbook(filename = os.path.join(os.path.abspath(os.path.dirname(cog.inFile)), "lang/lang_lut.xlsx"), data_only=True)
    sheet = wb['named_glyphs']

    idx = 0
    glyph_index = 1
    glyph_key = sheet[f"A{glyph_index}"].value
    while glyph_key and not glyph_key.startswith("PRIVATE_EMOJI"):
        glyph_key = sheet[f"A{glyph_index}"].value
        glyph_index = glyph_index + 1
    while glyph_key and glyph_key.startswith("PRIVATE_EMOJI"):
        cog.outl(f'case UM({idx}): return u" " {glyph_key};')
        glyph_key = sheet[f"A{glyph_index}"].value
        idx = idx + 1
        glyph_index = glyph_index + 1

    ]]]*/
    case UM(0): return u" " PRIVATE_EMOJI_1F600;
    case UM(1): return u" " PRIVATE_EMOJI_1F601;
    case UM(2): return u" " PRIVATE_EMOJI_1F602;
    case UM(3): return u" " PRIVATE_EMOJI_1F603;
    case UM(4): return u" " PRIVATE_EMOJI_1F604;
    case UM(5): return u" " PRIVATE_EMOJI_1F605;
    case UM(6): return u" " PRIVATE_EMOJI_1F606;
    case UM(7): return u" " PRIVATE_EMOJI_1F607;
    case UM(8): return u" " PRIVATE_EMOJI_1F608;
    case UM(9): return u" " PRIVATE_EMOJI_1F609;
    case UM(10): return u" " PRIVATE_EMOJI_1F60A;
    case UM(11): return u" " PRIVATE_EMOJI_1F60B;
    case UM(12): return u" " PRIVATE_EMOJI_1F60C;
    case UM(13): return u" " PRIVATE_EMOJI_1F60D;
    case UM(14): return u" " PRIVATE_EMOJI_1F60E;
    case UM(15): return u" " PRIVATE_EMOJI_1F60F;
    case UM(16): return u" " PRIVATE_EMOJI_1F610;
    case UM(17): return u" " PRIVATE_EMOJI_1F611;
    case UM(18): return u" " PRIVATE_EMOJI_1F612;
    case UM(19): return u" " PRIVATE_EMOJI_1F613;
    case UM(20): return u" " PRIVATE_EMOJI_1F614;
    case UM(21): return u" " PRIVATE_EMOJI_1F615;
    case UM(22): return u" " PRIVATE_EMOJI_1F616;
    case UM(23): return u" " PRIVATE_EMOJI_1F617;
    case UM(24): return u" " PRIVATE_EMOJI_1F618;
    case UM(25): return u" " PRIVATE_EMOJI_1F619;
    case UM(26): return u" " PRIVATE_EMOJI_1F61A;
    case UM(27): return u" " PRIVATE_EMOJI_1F61B;
    case UM(28): return u" " PRIVATE_EMOJI_1F61C;
    case UM(29): return u" " PRIVATE_EMOJI_1F61D;
    case UM(30): return u" " PRIVATE_EMOJI_1F61E;
    case UM(31): return u" " PRIVATE_EMOJI_1F61F;
    case UM(32): return u" " PRIVATE_EMOJI_1F620;
    case UM(33): return u" " PRIVATE_EMOJI_1F621;
    case UM(34): return u" " PRIVATE_EMOJI_1F622;
    case UM(35): return u" " PRIVATE_EMOJI_1F623;
    case UM(36): return u" " PRIVATE_EMOJI_1F624;
    case UM(37): return u" " PRIVATE_EMOJI_1F625;
    case UM(38): return u" " PRIVATE_EMOJI_1F626;
    case UM(39): return u" " PRIVATE_EMOJI_1F627;
    case UM(40): return u" " PRIVATE_EMOJI_1F628;
    case UM(41): return u" " PRIVATE_EMOJI_1F629;
    case UM(42): return u" " PRIVATE_EMOJI_1F62A;
    case UM(43): return u" " PRIVATE_EMOJI_1F62B;
    case UM(44): return u" " PRIVATE_EMOJI_1F62C;
    case UM(45): return u" " PRIVATE_EMOJI_1F62D;
    case UM(46): return u" " PRIVATE_EMOJI_1F62E;
    case UM(47): return u" " PRIVATE_EMOJI_1F62F;
    case UM(48): return u" " PRIVATE_EMOJI_1F630;
    case UM(49): return u" " PRIVATE_EMOJI_1F631;
    case UM(50): return u" " PRIVATE_EMOJI_1F632;
    case UM(51): return u" " PRIVATE_EMOJI_1F633;
    case UM(52): return u" " PRIVATE_EMOJI_1F634;
    case UM(53): return u" " PRIVATE_EMOJI_1F635;
    case UM(54): return u" " PRIVATE_EMOJI_1F636;
    case UM(55): return u" " PRIVATE_EMOJI_1F637;
    case UM(56): return u" " PRIVATE_EMOJI_1F638;
    case UM(57): return u" " PRIVATE_EMOJI_1F639;
    case UM(58): return u" " PRIVATE_EMOJI_1F644;
    case UM(59): return u" " PRIVATE_EMOJI_1F645;
    case UM(60): return u" " PRIVATE_EMOJI_1F646;
    case UM(61): return u" " PRIVATE_EMOJI_1F647;
    case UM(62): return u" " PRIVATE_EMOJI_1F648;
    case UM(63): return u" " PRIVATE_EMOJI_1F649;
    case UM(64): return u" " PRIVATE_EMOJI_1F64A;
    case UM(65): return u" " PRIVATE_EMOJI_1F64B;
    case UM(66): return u" " PRIVATE_EMOJI_1F64C;
    case UM(67): return u" " PRIVATE_EMOJI_1F64D;
    case UM(68): return u" " PRIVATE_EMOJI_1F64E;
    case UM(69): return u" " PRIVATE_EMOJI_1F64F;
    case UM(70): return u" " PRIVATE_EMOJI_1F440;
    case UM(71): return u" " PRIVATE_EMOJI_1F441;
    case UM(72): return u" " PRIVATE_EMOJI_1F442;
    case UM(73): return u" " PRIVATE_EMOJI_1F443;
    case UM(74): return u" " PRIVATE_EMOJI_1F444;
    case UM(75): return u" " PRIVATE_EMOJI_1F445;
    case UM(76): return u" " PRIVATE_EMOJI_1F446;
    case UM(77): return u" " PRIVATE_EMOJI_1F447;
    case UM(78): return u" " PRIVATE_EMOJI_1F448;
    case UM(79): return u" " PRIVATE_EMOJI_1F449;
    case UM(80): return u" " PRIVATE_EMOJI_1F44A;
    case UM(81): return u" " PRIVATE_EMOJI_1F44B;
    case UM(82): return u" " PRIVATE_EMOJI_1F44C;
    case UM(83): return u" " PRIVATE_EMOJI_1F44D;
    case UM(84): return u" " PRIVATE_EMOJI_1F44E;
    case UM(85): return u" " PRIVATE_EMOJI_1F44F;
    case UM(86): return u" " PRIVATE_EMOJI_1F450;
    case UM(87): return u" " PRIVATE_EMOJI_1F451;
    case UM(88): return u" " PRIVATE_EMOJI_1F452;
    case UM(89): return u" " PRIVATE_EMOJI_1F453;
    case UM(90): return u" " PRIVATE_EMOJI_1F47B;
    case UM(91): return u" " PRIVATE_EMOJI_1F47C;
    case UM(92): return u" " PRIVATE_EMOJI_1F47D;
    case UM(93): return u" " PRIVATE_EMOJI_1F47E;
    case UM(94): return u" " PRIVATE_EMOJI_1F47F;
    case UM(95): return u" " PRIVATE_EMOJI_1F480;
    case UM(96): return u" " PRIVATE_EMOJI_1F481;
    case UM(97): return u" " PRIVATE_EMOJI_1F482;
    case UM(98): return u" " PRIVATE_EMOJI_1F483;
    case UM(99): return u" " PRIVATE_EMOJI_1F484;
    case UM(100): return u" " PRIVATE_EMOJI_1F485;
    case UM(101): return u" " PRIVATE_EMOJI_1F489;
    case UM(102): return u" " PRIVATE_EMOJI_1F48A;
    case UM(103): return u" " PRIVATE_EMOJI_1F48B;
    case UM(104): return u" " PRIVATE_EMOJI_1F48C;
    case UM(105): return u" " PRIVATE_EMOJI_1F48D;
    case UM(106): return u" " PRIVATE_EMOJI_1F48E;
    case UM(107): return u" " PRIVATE_EMOJI_1F48F;
    case UM(108): return u" " PRIVATE_EMOJI_1F490;
    case UM(109): return u" " PRIVATE_EMOJI_1F491;
    case UM(110): return u" " PRIVATE_EMOJI_1F492;
    case UM(111): return u" " PRIVATE_EMOJI_1F493;
    case UM(112): return u" " PRIVATE_EMOJI_1F494;
    case UM(113): return u" " PRIVATE_EMOJI_1F495;
    case UM(114): return u" " PRIVATE_EMOJI_1F496;
    case UM(115): return u" " PRIVATE_EMOJI_1F4A1;
    case UM(116): return u" " PRIVATE_EMOJI_1F4A2;
    case UM(117): return u" " PRIVATE_EMOJI_1F4A3;
    case UM(118): return u" " PRIVATE_EMOJI_1F4A4;
    case UM(119): return u" " PRIVATE_EMOJI_1F4A5;
    case UM(120): return u" " PRIVATE_EMOJI_1F4A6;
    case UM(121): return u" " PRIVATE_EMOJI_1F4A7;
    case UM(122): return u" " PRIVATE_EMOJI_1F4A8;
    case UM(123): return u" " PRIVATE_EMOJI_1F4A9;
    case UM(124): return u" " PRIVATE_EMOJI_1F4AA;
    case UM(125): return u" " PRIVATE_EMOJI_1F4AB;
    case UM(126): return u" " PRIVATE_EMOJI_1F4AC;
    case UM(127): return u" " PRIVATE_EMOJI_1F4AD;
    case UM(128): return u" " PRIVATE_EMOJI_1F4AE;
    case UM(129): return u" " PRIVATE_EMOJI_1F4AF;
    case UM(130): return u" " PRIVATE_EMOJI_1F4B0;
    case UM(131): return u" " PRIVATE_EMOJI_1F4B1;
    case UM(132): return u" " PRIVATE_EMOJI_1F912;
    case UM(133): return u" " PRIVATE_EMOJI_1F913;
    case UM(134): return u" " PRIVATE_EMOJI_1F914;
    case UM(135): return u" " PRIVATE_EMOJI_1F915;
    case UM(136): return u" " PRIVATE_EMOJI_1F916;
    case UM(137): return u" " PRIVATE_EMOJI_1F917;
    case UM(138): return u" " PRIVATE_EMOJI_1F918;
    case UM(139): return u" " PRIVATE_EMOJI_1F919;
    //[[[end]]]
    case TO(_EMJ0): return u" " PRIVATE_EMOJI_1F600 u"\v" ICON_LAYER;
    case TO(_EMJ1): return u" " PRIVATE_EMOJI_1F440 u"\v" ICON_LAYER;
    case KC_DEADKEY: return (l_state.flags&DEAD_KEY_ON_WAKEUP)==0 ? u"WakeX\r\v" ICON_SWITCH_OFF : u"WakeX\r\v" ICON_SWITCH_ON;
    case LBL_TEXT: return u"Text:";
    case KC_TOGMODS: return (l_state.flags&MODS_AS_TEXT)==0 ? u"Mods\r\v" ICON_SWITCH_OFF : u"Mods\r\v" ICON_SWITCH_ON;
    case KC_TOGTEXT: return (l_state.flags&MORE_TEXT)==0 ? u"Cmds\r\v" ICON_SWITCH_OFF : u"Cmds\r\v" ICON_SWITCH_ON;
    case QK_UNICODE_MODE_MACOS:
        return l_state.unicode_mode==QK_UNICODE_MODE_MACOS ? u"Mac\r\v" ICON_SWITCH_ON : u"Mac\r\v" ICON_SWITCH_OFF;
    case QK_UNICODE_MODE_LINUX:
        return l_state.unicode_mode==QK_UNICODE_MODE_LINUX ? u"Lnx\r\v" ICON_SWITCH_ON : u"Lnx\r\v" ICON_SWITCH_OFF;
    case QK_UNICODE_MODE_WINDOWS:
        return l_state.unicode_mode==QK_UNICODE_MODE_WINDOWS ? u"Win\r\v" ICON_SWITCH_ON : u"Win\r\v" ICON_SWITCH_OFF;
    case QK_UNICODE_MODE_BSD:
        return l_state.unicode_mode==QK_UNICODE_MODE_BSD ? u"BSD\r\v" ICON_SWITCH_ON : u"BSD\r\v" ICON_SWITCH_OFF;
    case QK_UNICODE_MODE_WINCOMPOSE:
        return l_state.unicode_mode==QK_UNICODE_MODE_WINCOMPOSE ? u"WinC\r\v" ICON_SWITCH_ON : u"WinC\r\v" ICON_SWITCH_OFF;
    case QK_UNICODE_MODE_EMACS:
        return l_state.unicode_mode==QK_UNICODE_MODE_EMACS ? u"Emcs\r\v" ICON_SWITCH_ON : u"Emcs\r\v" ICON_SWITCH_OFF;
    case QK_LEAD:
        return u"Lead";
    case KC_HYPR:
        return (l_state.flags&MORE_TEXT)!=0 ? u"Hypr" : u" " PRIVATE_HYPER;
    case KC_MEH:
        return (l_state.flags&MORE_TEXT)!=0 ? u"Meh" : u" " PRIVATE_MEH;
    case KC_EXEC:
        return u"Exec";
    case KC_NUM_LOCK:
        return !state.num_lock ? u"Nm" ICON_NUMLOCK_OFF : u"Nm" ICON_NUMLOCK_ON;
    case KC_KP_SLASH:
        return u"/";
    case KC_KP_ASTERISK:
    case KC_KP_MINUS:
        return u"-";
    case KC_KP_7:
        return !state.num_lock ? ARROWS_LEFTSTOP : u"7";
    case KC_KP_8:
        return !state.num_lock ? u"   " ICON_UP : u"8";
    case KC_KP_9:
        return !state.num_lock ? ARROWS_UPSTOP : u"9";
    case KC_KP_PLUS:
        return u"+";
    case KC_KP_4:
        return !state.num_lock ? u"  " ICON_LEFT : u"4";
    case KC_KP_5:
        return !state.num_lock ? u"" : u"5";
    case KC_KP_6:
        return !state.num_lock ? u"  " ICON_RIGHT : u"6";
    case KC_KP_EQUAL:
        return u"=";
    case KC_KP_1:
        return !state.num_lock ? ARROWS_RIGHTSTOP : u"1";
    case KC_KP_2:
        return !state.num_lock ? u"   " ICON_DOWN : u"2";
    case KC_KP_3:
        return !state.num_lock ? ARROWS_DOWNSTOP : u"3";
    case KC_CALCULATOR:
        return u"   " PRIVATE_CALC;
    case KC_KP_0:
        return !state.num_lock ? u"Ins" : u"0";
    case KC_KP_DOT:
        return !state.num_lock ? u"Del" : u".";
    case KC_KP_ENTER:
        return u"Enter";
    case QK_BOOTLOADER:
        return u"Boot";
    case QK_DEBUG_TOGGLE:
        return ((l_state.flags & DBG_ON) == 0) ? u"Dbg\r\v" ICON_SWITCH_OFF : u"Dbg\r\v" ICON_SWITCH_ON;
    case RGB_RMOD:
        return u" " ICON_LEFT PRIVATE_LIGHT;
    case KC_RGB_TOG:
        return u"  I/O";
    case RGB_MOD:
        return PRIVATE_LIGHT ICON_RIGHT;
    case RGB_HUI:
        return u"Hue+";
    case RGB_HUD:
        return u"Hue-";
    case RGB_SAI:
        return u"Sat+";
    case RGB_SAD:
        return u"Sat-";
    case RGB_VAI:
        return u"Bri+";
    case RGB_VAD:
        return u"Bri-";
    case RGB_SPI:
        return u"Spd+";
    case RGB_SPD:
        return u"Spd-";
    case RGB_MODE_PLAIN:
        return u"Plan";
    case RGB_MODE_BREATHE:
        return u"Brth";
    case RGB_MODE_SWIRL:
        return u"Swrl";
    case RGB_MODE_RAINBOW:
        return u"Rnbw";
    case KC_MEDIA_NEXT_TRACK:
        return ICON_RIGHT ICON_RIGHT;
    case KC_MEDIA_PLAY_PAUSE:
        return u"  " ICON_RIGHT;
    case KC_MEDIA_STOP:
        return u"Stop";
    case KC_MEDIA_PREV_TRACK:
        return ICON_LEFT ICON_LEFT;
    case KC_MS_ACCEL0:
        return u">>";
    case KC_MS_ACCEL1:
        return u">>>";
    case KC_MS_ACCEL2:
        return u">>>>";
    case KC_BTN1:
        return u"  " ICON_LMB;
    case KC_BTN2:
        return u"  " ICON_RMB;
    case KC_BTN3:
        return u"  " ICON_MMB;
    case KC_MS_UP:
        return u"  " ICON_UP;
    case KC_MS_DOWN:
        return u"  " ICON_DOWN;
    case KC_MS_LEFT:
        return u"  " ICON_LEFT;
    case KC_MS_RIGHT:
        return u"  " ICON_RIGHT;
    case KC_AUDIO_MUTE:
        return u"  " PRIVATE_MUTE;
    case KC_AUDIO_VOL_DOWN:
        return u"  " PRIVATE_VOL_DOWN;
    case KC_AUDIO_VOL_UP:
        return u"  " PRIVATE_VOL_UP;
    case KC_PRINT_SCREEN:
        return u"  " PRIVATE_IMAGE;
    case KC_SCROLL_LOCK:
        return u"ScLk";
    case KC_PAUSE:
        return u"Paus";
    case KC_INSERT:
        return u"Ins";
    case KC_HOME:
        return ARROWS_LEFTSTOP;
    case KC_END:
        return u"   " ARROWS_RIGHTSTOP;
    case KC_PAGE_UP:
        return u"  " ARROWS_UPSTOP;
    case KC_PAGE_DOWN:
        return u"  " ARROWS_DOWNSTOP;
    case KC_DELETE:
        return (l_state.flags&MORE_TEXT)!=0 ? u"Del" : TECHNICAL_ERASERIGHT;
    case KC_MYCM:
        return u"  " PRIVATE_PC;
    case TO(_SL):
        return PRIVATE_SETTINGS u"\v" ICON_LAYER;
    case MO(_FL0):
    case MO(_FL1):
        return u"Fn\r\v\t" ICON_LAYER;
    case TO(_NL):
        return u"Nm\r\v\t" ICON_LAYER;
    case MO(_NL):
        return u"Nm!\r\v\t" ICON_LAYER;
    case KC_BASE:
        return u"Base\r\v\t" ICON_LAYER;
    case KC_L0:
        return l_layer.def_layer==_L0 ? u"Qwty\r\v" ICON_SWITCH_ON : u"Qwty\r\v" ICON_SWITCH_OFF;
    case KC_L1:
        return l_layer.def_layer==_L1 ? u"Qwty!\r\v" ICON_SWITCH_ON : u"Qwty!\r\v" ICON_SWITCH_OFF;
    case KC_L2:
        return l_layer.def_layer==_L2 ? u"Clmk\r\v" ICON_SWITCH_ON : u"Clmk\r\v" ICON_SWITCH_OFF;
    case KC_L3:
        return l_layer.def_layer==_L3 ? u"Neo\r\v" ICON_SWITCH_ON : u"Neo\r\v" ICON_SWITCH_OFF;
    case KC_L4:
        return l_layer.def_layer==_L4 ? u"Wkm\r\v" ICON_SWITCH_ON : u"Wkm\r\v" ICON_SWITCH_OFF;
    case OSL(_UL):
        return u"Util*\r\v\t" ICON_LAYER;
    case TO(_UL):
        return u"Util\r\v\t" ICON_LAYER;
    case MO(_ADDLANG1):
        return u"Intl";
    case KC_F1: return u" F1";
    case KC_F2: return u" F2";
    case KC_F3: return u" F3";
    case KC_F4: return u" F4";
    case KC_F5: return u" F5";
    case KC_F6: return u" F6";
    case KC_F7: return u" F7";
    case KC_F8: return u" F8";
    case KC_F9: return u" F9";
    case KC_F10:return u" F10";
    case KC_F11:return u" F11";
    case KC_F12:return u" F12";
    case KC_F13:return u" F13";
    case KC_F14:return u" F14";
    case KC_F15:return u" F15";
    case KC_F16:return u" F16";
    case KC_F17:return u" F17";
    case KC_F18:return u" F18";
    case KC_F19:return u" F19";
    case KC_F20:return u" F20";
    case KC_F21:return u" F21";
    case KC_F22:return u" F22";
    case KC_F23:return u" F23";
    case KC_F24:return u" F24";
    case KC_LEFT_CTRL:
    case KC_RIGHT_CTRL:
        return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Ctrl" : TECHNICAL_CONTROL;
    case KC_LEFT_ALT:
        return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Alt" : TECHNICAL_OPTION;
    case KC_RIGHT_ALT:
        return (l_state.flags&MODS_AS_TEXT)!=0 ? ( l_state.lang == KC_LANG_EN ? u"Alt" : u"Alt\nGr") : TECHNICAL_OPTION;
    case KC_LGUI:
    case KC_RGUI:
        return (l_state.flags&MODS_AS_TEXT)!=0 ? u"GUI" : DINGBAT_BLACK_DIA_X;
    case KC_LEFT:
        return u"  " ICON_LEFT;
    case KC_RIGHT:
        return u"  " ICON_RIGHT;
    case KC_UP:
        return u"  " ICON_UP;
    case KC_DOWN:
        return u"  " ICON_DOWN;
    case KC_CAPS_LOCK:
        return state.caps_lock ? u"Cap" ICON_CAPSLOCK_ON : u"Cap" ICON_CAPSLOCK_OFF;
    case KC_LSFT:
    case KC_RSFT:
        return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Shft" : u" " ICON_SHIFT;
    case KC_NO:
        return u"";
    case KC_DDIM:
        return u"  " ICON_LEFT;
    case KC_DBRI:
        return u"  " ICON_RIGHT;
    case KC_D1Q:
        return u"  " PRIVATE_DISP_DARKER;
    case KC_D3Q:
        return u"  " PRIVATE_DISP_BRIGHTER;
    case KC_DHLF:
        return u"  " PRIVATE_DISP_HALF;
    case KC_DMIN:
        return u"  " PRIVATE_DISP_DARK;
    case KC_DMAX:
        return u"  " PRIVATE_DISP_BRIGHT;
    case KC_LANG:
        return (l_state.flags&MORE_TEXT)!=0 ? u"Lang" : PRIVATE_WORLD;
    case SH_TOGG:
        return u"SwpH";
    case QK_MAKE:
        return u"Make";
    case EE_CLR:
        return u"ClrEE";
    case QK_REBOOT:
        return u" " ARROWS_CIRCLE;
    case KC_LNG1:
        return u"Han/Y";
    case KC_APP:
        return u" Ctx";
    case DE_GRV: //for Neo Layout
        return u"`";
    case KC_CUT:
        return u"Cut";
    case KC_COPY:
        return u"Copy";
    case KC_PASTE:
        return u"Paste";
    case KC_UNDO:
        return u"Undo";
    case KC_AGAIN:
        return u"Redo";
    case KC_FIND:
        return u"Find";
    case KC_SELECT:
        return u"Word\r\v   sel";
    case KC_EXSEL:
        return u"Line\r\v    sel";
    case KC_OPER:
        return u"Line\r\v    join";
    case KC_CRSEL:
        return u"Line\r\v    del";
    /*[[[cog
        for lang in languages:
            short = lang.split("_")[1]
            cog.outl(f'case KC_{lang}: return l_state.lang == {lang} ? u"[{short}]" : u" {short}";')
    ]]]*/
    case KC_LANG_EN: return l_state.lang == LANG_EN ? u"[EN]" : u" EN";
    case KC_LANG_DE: return l_state.lang == LANG_DE ? u"[DE]" : u" DE";
    case KC_LANG_FR: return l_state.lang == LANG_FR ? u"[FR]" : u" FR";
    case KC_LANG_ES: return l_state.lang == LANG_ES ? u"[ES]" : u" ES";
    case KC_LANG_PT: return l_state.lang == LANG_PT ? u"[PT]" : u" PT";
    case KC_LANG_IT: return l_state.lang == LANG_IT ? u"[IT]" : u" IT";
    case KC_LANG_TR: return l_state.lang == LANG_TR ? u"[TR]" : u" TR";
    case KC_LANG_KO: return l_state.lang == LANG_KO ? u"[KO]" : u" KO";
    case KC_LANG_JA: return l_state.lang == LANG_JA ? u"[JA]" : u" JA";
    case KC_LANG_AR: return l_state.lang == LANG_AR ? u"[AR]" : u" AR";
    case KC_LANG_GR: return l_state.lang == LANG_GR ? u"[GR]" : u" GR";
    //[[[end]]]
    //The following entries will over-rule language specific enties in the follow langauge lookup table,
    //however with this we can control them by flags and so far those wehere not lanuage specific anyway.
    case KC_ENTER:      return (l_state.flags&MORE_TEXT)!=0 ? u"Enter"    : ARROWS_RETURN;
    case KC_ESCAPE:	    return (l_state.flags&MORE_TEXT)!=0 ? u"Esc"      : TECHNICAL_ESCAPE;
    case KC_BACKSPACE:  return (l_state.flags&MORE_TEXT)!=0 ? u"Bksp"     : TECHNICAL_ERASELEFT;
    case KC_TAB:        return (l_state.flags&MORE_TEXT)!=0 ? u"Tab"      : ARROWS_TAB;
    case KC_SPACE:      return (l_state.flags&MORE_TEXT)!=0 ? u"Space"    : ICON_SPACE;
    default:
    {
        bool shift = ((l_layer.mods & MOD_MASK_SHIFT) != 0);
        // bool add_lang = get_highest_layer(l_layer.layer)==_ADDLANG1;
        // bool alt = ((l_layer.mods & MOD_MASK_ALT) != 0);
        // if(keycode>=KC_A && keycode<=KC_Z && add_lang) {
        //     //display the previously selected latin variation of the letter
        //     const uint8_t offset = (shift || state.caps_lock) ? 0 : 26;
        //     uint8_t variation = (shift || state.caps_lock) ? g_latin.ex[keycode-KC_A]>>4 : g_latin.ex[keycode-KC_A]&0xf;

        //     const uint16_t* def_variation = latin_ex_map[offset+keycode-KC_A][0];
        //     return (def_variation!=NULL) ? latin_ex_map[offset+keycode-KC_A][variation] : NULL;
        // }

        // if(keycode>=KC_LAT0 && keycode<=KC_LAT9) {
        //     if(add_lang && alt && l_last.latin_kc!=0) {
        //         //show all available alternatives for selected latin letter
        //         const uint8_t offset = (shift || state.caps_lock) ? 0 : 26;
        //         return latin_ex_map[offset+l_last.latin_kc-KC_A][keycode-KC_LAT0];
        //     } else {
        //         return NULL; //show nothing
        //     }
        // }

        const uint16_t* text = translate_keycode(l_state.lang, keycode, shift, state.caps_lock);
        if (text != NULL) {
            return text;
        }
    }
    break;
    }
    return NULL;
}

const uint16_t* keycode_to_disp_overlay(uint16_t keycode, led_t state) {
    switch (keycode)
    {
        case KC_F2: return u"      " PRIVATE_NOTE;
        case KC_F5: return u"     " ARROWS_CIRCLE;
        default: break;
    }

    if( (l_layer.mods & MOD_MASK_CTRL) != 0) {
        switch(keycode) {
            case KC_A: return u"      " BOX_WITH_CHECK_MARK;
            case KC_C: return u"     " CLIPBOARD_COPY;
            case KC_D: return u"\t " PRIVATE_DELETE;
            case KC_F: return u"    " PRIVATE_FIND;
            case KC_X: return u"\t\b\b\b\b" CLIPBOARD_CUT;
            case KC_V: return u"     " CLIPBOARD_PASTE;
            case KC_S: return u"\t" PRIVATE_FLOPPY;
            case KC_O: return u"\t" FILE_OPEN;
            case KC_P: return u"\t\b\b" PRIVATE_PRINTER;
            case KC_Z: return u"      " ARROWS_UNDO;
            case KC_Y: return u"      " ARROWS_REDO;
            default: break;
        }
    } else if((l_layer.mods & MOD_MASK_GUI) != 0) {
        switch(keycode) {
            case KC_D:      return u"    " PRIVATE_PC;
            case KC_L:      return u"    " PRIVATE_LOCK;
            case KC_P:      return u"    " PRIVATE_SCREEN;
            case KC_UP:     return u"     " PRIVATE_MAXIMIZE;
            case KC_DOWN:   return u"     " PRIVATE_WINDOW;
            default: break;
        }
    }
    return NULL;
}

// NO_MOD(0), CTRL(1), SHIFT(2), CTRL_SHIFT(3), ALT(4), CTRL_ALT(5),
// ALT_SHIFT(6), {Not supported: CTRL_ALT_SHIFT(7) and GUI_KEY(8)}
uint16_t adjust_overlay_idx_to_mod(uint16_t idx, uint8_t mods) {
    //no differencce between r&l mods:
    mods |= mods>>4;
    mods &= 0x7;
    // if(mods>7) {  //no CTRL_ALT_SHIFT and no combo with GUI, so 7 == GUI
    //     return idx + NUM_OVERLAYS*7;
    // }

    return mods == 7 ? idx : idx + NUM_OVERLAYS * mods;
}

bool copy_overlay_to_buffer(uint16_t keycode, uint8_t mods, bool combine) {
    if(keycode>KC_RGUI || (keycode>KC_NUM_LOCK && keycode<KC_NUBS) || (keycode>KC_APP && keycode<KC_LEFT_CTRL)) {
        return false;
    }
    uint16_t idx = (keycode>KC_APP) ? (keycode - KC_LEFT_CTRL + 82) : (keycode>KC_NUM_LOCK ? keycode - KC_NUBS + 80 : keycode - KC_A);
    if(idx>=90) {
        return false;
    }
    idx = adjust_overlay_idx_to_mod(idx, mods);
    if(!use_overlay[idx]) {
        return false;
    }
    if(combine) {
        combine_with_mask();
    }
    kdisp_draw_bitmap(28, 0, overlays[idx], 72, 40); //don't understnad why we start at offset 28... need to think about it
    return true;
}

void update_displays(enum refresh_mode mode) {
    if(l_state.contrast<=DISP_OFF || (l_state.flags&DISP_IDLE)!=0) {
        return;
    }

    //uint8_t layer = get_highest_layer(layer_state);

    const led_t state = l_layer.led_state;
    const uint8_t mods = l_layer.mods;
    const bool capital_case = ((mods & MOD_MASK_SHIFT) != 0) || state.caps_lock;
    const bool display_overlays = test_flag(l_state.overlay_flags, DISPLAY_OVERLAYS);
    //the left side has an offset of 0, the right side an offset of MATRIX_ROWS_PER_SIDE
    const uint8_t offset = is_left_side() ? 0 : MATRIX_ROWS_PER_SIDE;
    uint8_t start_row = 0;

    //select first display (and later on shift that 0 till the end)
    if (mode == START_SECOND_HALF) {
        sr_shift_out_buffer_latch(disp_row_0.bitmask, sizeof(struct display_info));
        start_row = 3;
    }
    else {
        sr_shift_out_buffer_latch(disp_row_0.bitmask, sizeof(struct display_info));
    }

    const uint8_t max_rows = mode == START_FIRST_HALF ? 3 : MATRIX_ROWS_PER_SIDE;

    const bool overlay_only = test_flag( l_state.overlay_flags, CLEAR_LEFT_TOP|CLEAR_LEFT_BOTTOM|CLEAR_RIGHT_TOP|CLEAR_RIGHT_BOTTOM);
    const bool combine = !overlay_only && (l_state.overlay_flags & (CLEAR_LEFT_TOP|CLEAR_LEFT_BOTTOM|CLEAR_RIGHT_TOP|CLEAR_RIGHT_BOTTOM))!=0;
    if(combine ) {
        prepare_mask_buffer(l_state.overlay_flags);
    }

    uint8_t skip = 0;
    for (uint8_t r = start_row; r < max_rows; ++r) {
        for (uint8_t c = 0; c < MATRIX_COLS; ++c) {
            uint8_t  disp_idx = LAYOUT_TO_INDEX(r, c);

            //since MATRIX_COLS==8 we don't need to shift multiple times at the end of the row
            //except there was a leading and missing physical key (KC_NO on base layer)
            uint16_t keycode = keymaps[_BL][r + offset][c];
            if (keycode == KC_NO) {
                skip++;
            }
            else {
                if (disp_idx != 255) {
                    uint8_t layer = get_highest_layer(l_layer.layer);
                    uint16_t highest_kc = keymaps[layer][r + offset][c];
                    //if we encounter a transparent key go down one layer (but only one!)
                    keycode = (highest_kc == KC_TRNS) ? keymaps[get_highest_layer(l_layer.layer&~(1<<layer))][r + offset][c] : highest_kc;
                    kdisp_enable(true);//(l_state.flags&STATUS_DISP_ON) != 0);
                    kdisp_set_contrast(l_state.contrast-1);
                    if(keycode!=KC_TRNS) {
                        const uint16_t* text = keycode_to_disp_text(keycode, state);
                        kdisp_set_buffer(0x00);
                        if(!overlay_only) {
                            if(text==NULL){
                                if((keycode&QK_UNICODEMAP_PAIR)!=0){
                                    uint16_t chr = capital_case ? QK_UNICODEMAP_PAIR_GET_SHIFTED_INDEX(keycode) : QK_UNICODEMAP_PAIR_GET_UNSHIFTED_INDEX(keycode);
                                    kdisp_write_gfx_char(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, unicode_map[chr]);
                                }
                            } else {
                                kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, text);
                            }
                        }
                        text = NULL;
                        if(display_overlays) {
                            if(!copy_overlay_to_buffer(keycode, mods, combine)) {
                                text = keycode_to_disp_overlay(keycode, state); //fallback to hardcoded
                            }
                        } else {
                            text = keycode_to_disp_overlay(keycode, state); //this should maybe go away - or setting?
                        }
                        if(text) {
                            kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, text);
                        }
                        kdisp_send_buffer();
                    }
                }
                sr_shift_once_latch();
            }

        }
        for (;skip > 0;skip--) {
            sr_shift_once_latch();
        }
    }
}

uint8_t to_brightness(uint8_t b) {
    switch(b) {
        case 23: case 24: case 25: case 26: case 27: return 7;
        case 22: case 28: return 6;
        case 21: case 29: return 5;
        case 20: case 30: return 4;
        case 19: case 31: return 3;
        case 18: case 32: return 2;
        case 1: case 7: return 1;
        case 2: case 6: return 3;
        case 3: case 4: case 5: return 5;
        default: return 0;
    }
}

void kdisp_idle(uint8_t contrast) {
    uint8_t offset = is_left_side() ? 0 : MATRIX_ROWS_PER_SIDE;
    uint8_t skip = 0;
    sr_shift_out_buffer_latch(disp_row_0.bitmask, sizeof(struct display_info));

    //uint8_t idx = 0;
    for (uint8_t r = 0; r < MATRIX_ROWS_PER_SIDE; ++r) {
        for (uint8_t c = 0; c < MATRIX_COLS; ++c) {
            uint8_t  disp_idx = LAYOUT_TO_INDEX(r, c);

            //since MATRIX_COLS==8 we don't need to shift multiple times at the end of the row
            //except there was a leading and missing physical key (KC_NO on base layer)
            uint16_t keycode = keymaps[_BL][r + offset][c];
            if (keycode == KC_NO) {
                skip++;
            } else {
                if (disp_idx != 255) {
                    uint8_t idle_brightness = to_brightness((contrast+(c%3+r)*keycode+offset+r)%50);
                    if(idle_brightness==0) {
                        kdisp_enable(false);
                    } else {
                        kdisp_enable(true);
                        kdisp_set_contrast(idle_brightness-1);
                    }
                }
                sr_shift_once_latch();
            }

        }
        for (;skip > 0;skip--) {
            sr_shift_once_latch();
        }
    }
}

void display_message(uint8_t row, uint8_t col, const uint16_t* message, const GFXfont* font) {

    const GFXfont* displayFont[] = { font };
    uint8_t index = 0;
    for (uint8_t c = 0; c < MATRIX_COLS; ++c) {

        uint8_t disp_idx = LAYOUT_TO_INDEX(row, c);

        if (disp_idx != 255) {

            sr_shift_out_buffer_latch(key_display[disp_idx].bitmask, sizeof(key_display->bitmask));
            kdisp_set_buffer(0x00);

            if (c >= col && message[index] != 0) {
                const uint16_t text[2] = { message[index], 0 };
                kdisp_write_gfx_text(displayFont, 1, 49, 38, text);
                index++;
            }
            kdisp_send_buffer();
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case KC_CRSEL:
            if (record->event.pressed) { SEND_STRING(SS_TAP(X_HOME) SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_END)) SS_TAP(X_BACKSPACE) SS_TAP(X_BACKSPACE) SS_TAP(X_DOWN)); }
            uprint("Delete Line.\n");
            return false;
        case KC_SELECT:
            if (record->event.pressed) { SEND_STRING(SS_LCTL(SS_TAP(X_LEFT) SS_LSFT(SS_TAP(X_RGHT)))); }
            uprint("Select Word.\n");
            return false;
        case KC_EXSEL:
            if (record->event.pressed) { SEND_STRING(SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_END))); }
            uprint("Select Line.\n");
            return false;
        case KC_OPER:
            if (record->event.pressed) {
                SEND_STRING( // Go to the end of the line and tap delete.
                    SS_TAP(X_END) SS_TAP(X_DEL)
                    SS_TAP(X_SPC) // In case this has joined two wormatrix toaend_string splhhhhds together, insert one space.
                    SS_LCTL(
                        // Go to the beginning of the next word.
                        SS_TAP(X_RGHT) SS_TAP(X_LEFT)
                        // Select back to the end of the previous word. This should select
                        // all spaces and tabs between the joined lines from indentation
                        // or trailing whitespace, including the space inserted earlier.
                        SS_LSFT(SS_TAP(X_LEFT) SS_TAP(X_RGHT))
                    )
                    SS_TAP(X_SPC) // Replace the selection with a single space.
                );
                uprint("Join Line.\n");
            }
            return false;
        default:
            break;
    }
    const bool addlang = get_highest_layer(l_layer.layer)==_ADDLANG1;
    if (record->event.pressed) {
        switch (keycode) {
            case QK_BOOTLOADER:
                uprintf("Bootloader entered. Please copy new Firmware.\n");
                clear_all_displays();
                display_message(1, 1, u"BOOT-", &FreeSansBold24pt7b);
                display_message(3, 0, u"LOADER!", &FreeSansBold24pt7b);
                return true;
            case KC_A ... KC_Z:
                l_last.latin_kc = keycode;
                if((get_mods() & MOD_MASK_ALT) == 0 && addlang) {
                    // const bool lshift = get_mods() == MOD_BIT(KC_LEFT_SHIFT);
                    // const bool rshift = get_mods() == MOD_BIT(KC_RIGHT_SHIFT);
                    // const bool upper_case = lshift || rshift || g_layer.led_state.caps_lock;
                    // const uint8_t offset = upper_case ? 0 : 26;
                    // if(latin_ex_map[offset+keycode-KC_A][0]) {
                        // uint8_t variation = upper_case ? g_latin.ex[keycode-KC_A]>>4 : g_latin.ex[keycode-KC_A]&0xf;

                        // //this is a work-around (at least for I-Bus on Linux we need to remove the shift, otherwise the Unicode sequence will not be recognized!)
                        // if(lshift) unregister_code16(KC_LEFT_SHIFT);
                        // if(rshift) unregister_code16(KC_RIGHT_SHIFT);
                        // register_code(latin_ex_map[offset+keycode-KC_A][variation][0]);
                        // if(lshift) register_code16(KC_LEFT_SHIFT);
                        // if(rshift) register_code16(KC_RIGHT_SHIFT);
                        // return false;
                    // }
                }
                break;
            default:
                break;
        }

        if((get_mods() & MOD_MASK_ALT) != 0 && addlang) {
            switch(keycode) {
                case KC_LAT0 ... KC_LAT9:
                    if( l_last.latin_kc!=0) {
                        uint8_t current = g_latin.ex[l_last.latin_kc-KC_A];
                        if((get_mods() & MOD_MASK_SHIFT) || g_layer.led_state.caps_lock) {
                            g_latin.ex[l_last.latin_kc-KC_A] = ((keycode-KC_LAT0)<<4) | (current&0xf);
                        } else {
                            g_latin.ex[l_last.latin_kc-KC_A] = (keycode-KC_LAT0) | (current&0xf0);
                        }
                        // send_to_bridge(USER_SYNC_LATIN_EX_DATA, (void*)&g_latin, sizeof(g_latin), 10);

                        save_user_eeconf();
                        request_disp_refresh();
                    }
                    break;
                case KC_A ... KC_Z:
                    request_disp_refresh();
                    break;
                default:
                    break;
            }

            return false;
        } else {
            l_last.latin_kc = 0;
        }
    }
    return display_wakeup(record);
}

void post_process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (keycode == KC_CAPS_LOCK) {
        request_disp_refresh();
    }

    if (!record->event.pressed) {
        switch (keycode) {
        case KC_RGB_TOG:
            l_state.flags = toggle_flag(l_state.flags, RGB_ON);
            break;
        case KC_DEADKEY:
            l_state.flags = toggle_flag(l_state.flags, DEAD_KEY_ON_WAKEUP);
            request_disp_refresh();
            break;
        case KC_TOGMODS:
            l_state.flags = toggle_flag(l_state.flags, MODS_AS_TEXT);
            request_disp_refresh();
            break;
        case KC_TOGTEXT:
            l_state.flags = toggle_flag(l_state.flags, MORE_TEXT);
            request_disp_refresh();
            break;
        case KC_L0:
            l_layer.def_layer = _L0;
            persistent_default_layer_set(l_layer.def_layer);
            request_disp_refresh();
            break;
        case KC_L1:
            l_layer.def_layer = _L1;
            persistent_default_layer_set(l_layer.def_layer);
            request_disp_refresh();
            break;
        case KC_L2:
            l_layer.def_layer = _L2;
            persistent_default_layer_set(l_layer.def_layer);
            request_disp_refresh();
            break;
        case KC_L3:
            l_layer.def_layer = _L3;
            persistent_default_layer_set(l_layer.def_layer);
            request_disp_refresh();
            break;
        case KC_L4:
            l_layer.def_layer = _L4;
            persistent_default_layer_set(l_layer.def_layer);
            request_disp_refresh();
            break;
        case KC_BASE:
            layer_clear();
            layer_on(l_layer.def_layer);
            break;
        case KC_RIGHT_SHIFT:
        case KC_LEFT_SHIFT:
            request_disp_refresh();
            break;
        case KC_D1Q:
            l_state.contrast = FULL_BRIGHT/4;
            save_user_eeconf();
            break;
        case KC_D3Q:
            l_state.contrast = (FULL_BRIGHT/4)*3;
            save_user_eeconf();
            break;
        case KC_DHLF:
            l_state.contrast = FULL_BRIGHT/2;
            save_user_eeconf();
            break;
        case KC_DMAX:
            l_state.contrast = FULL_BRIGHT;
            save_user_eeconf();
            break;
        case KC_DMIN:
            l_state.contrast = 2;
            save_user_eeconf();
            break;
        case KC_DDIM:
            dec_brightness();
            break;
        case KC_DBRI:
            inc_brightness();
            break;
        /*[[[cog
            for lang in languages:
                cog.outl(f'case KC_{lang}: l_state.lang = {lang}; save_user_eeconf(); layer_off(_LL); break;')
            ]]]*/
        case KC_LANG_EN: l_state.lang = LANG_EN; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_DE: l_state.lang = LANG_DE; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_FR: l_state.lang = LANG_FR; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_ES: l_state.lang = LANG_ES; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_PT: l_state.lang = LANG_PT; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_IT: l_state.lang = LANG_IT; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_TR: l_state.lang = LANG_TR; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_KO: l_state.lang = LANG_KO; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_JA: l_state.lang = LANG_JA; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_AR: l_state.lang = LANG_AR; save_user_eeconf(); layer_off(_LL); break;
        case KC_LANG_GR: l_state.lang = LANG_GR; save_user_eeconf(); layer_off(_LL); break;
        //[[[end]]]
        case KC_F1:case KC_F2:case KC_F3:case KC_F4:case KC_F5:case KC_F6:
        case KC_F7:case KC_F8:case KC_F9:case KC_F10:case KC_F11:case KC_F12:
            layer_off(_LL);
            break;
        default:
            break;
        }
    }
    else {
        switch (keycode)
        {
        case KC_RIGHT_SHIFT:
        case KC_LEFT_SHIFT:
            request_disp_refresh();
            break;
        case KC_LANG:
            if (IS_LAYER_ON(_LL)) {
                l_state.lang = (l_state.lang + 1) % NUM_LANG;
                save_user_eeconf();
                layer_off(_LL);
            }
            else {
                layer_on(_LL);
            }
            break;
        case RGB_MOD:
        case RGB_RMOD:
            request_disp_refresh();
            break;
        default:
            break;
        }
    }

    uprintf("Key 0x%04X, col/row: %u/%u, %s, time: %u, int: %d, cnt: %u\n",
        keycode, record->event.key.col, record->event.key.row, record->event.pressed ? "DN" : "UP",
        record->event.time, record->tap.interrupted ? 1 : 0, record->tap.count);

    update_performed();
};

void show_splash_screen(void) {
    clear_all_displays();
    display_message(1, 1, u"POLY", &FreeSansBold24pt7b);
    display_message(2, 1, u"KYBD", &FreeSansBold24pt7b);
}

void set_displays(uint8_t contrast, bool idle) {
    if(idle) {
        kdisp_idle(contrast);
    } else {
        select_all_displays();
        if(contrast==DISP_OFF) {
            kdisp_enable(false);
        } else {
            kdisp_enable(true);
            kdisp_set_contrast(contrast - 1);
        }
    }
}



//disable first keypress if the displays are turned off
bool display_wakeup(keyrecord_t* record) {
    bool accept_keypress = true;
    if ((l_state.contrast==DISP_OFF || (l_state.flags & DISP_IDLE)!=0) && record->event.pressed) {
        if(l_state.contrast==DISP_OFF && (l_state.flags&DEAD_KEY_ON_WAKEUP)!=0) {
            accept_keypress = timer_elapsed32(last_update) <= TURN_OFF_TIME;
        }
        poly_eeconf_t ee = load_user_eeconf();
        l_state.contrast = ee.brightness;
        l_state.flags &= ~((uint8_t)DISP_IDLE);
        l_state.flags |= STATUS_DISP_ON;
        update_performed();
        request_disp_refresh();
    }

    return accept_keypress;
}

void unicode_input_mode_set_user(uint8_t unicode_mode) {
    l_state.unicode_mode = unicode_mode;
    request_disp_refresh();
}

void keyboard_post_init_user(void) {
    // Customise these values to desired behaviour
    debug_enable = true;
    debug_matrix = false;
    debug_keyboard = false;
    debug_mouse = false;

    //pointing_device_set_cpi(20000);
    // pointing_device_set_cpi(650);
    //pimoroni_trackball_set_rgbw(0,0,255,100);
    l_layer.def_layer = persistent_default_layer_get();
    // l_state.unicode_mode = get_unicode_input_mode();
    layer_clear();
    layer_on(l_layer.def_layer);

    //set these values, they will never change
    com = is_keyboard_master() ? USB_HOST : BRIDGE;
    side = is_keyboard_left() ? LEFT_SIDE : RIGHT_SIDE;

    //encoder pins
    // setPinInputHigh(GP25);
    // setPinInputHigh(GP29);

    //srand(halGetCounterValue());

    memset(&g_state, 0, sizeof(g_state));
    memset(&use_overlay, 0, sizeof(use_overlay));

    // transaction_register_rpc(USER_SYNC_POLY_DATA,       user_sync_poly_data_handler);
    // transaction_register_rpc(USER_SYNC_LAYER_DATA,      user_sync_layer_data_handler);
    // transaction_register_rpc(USER_SYNC_LASTKEY_DATA,    user_sync_lastkey_data_handler);
    // transaction_register_rpc(USER_SYNC_LATIN_EX_DATA,   user_sync_latin_ex_data_handler);
    // transaction_register_rpc(USER_SYNC_OVERLAY_DATA,    user_sync_overlay_data_handler);

}

void keyboard_pre_init_user(void) {
    kdisp_hw_setup();
    kdisp_init(NUM_SHIFT_REGISTERS);
    peripherals_reset();
    kdisp_setup(true);

    select_all_displays();
    kdisp_scroll_vlines(47);
    kdisp_scroll_modehv(true, 3, 1);
    kdisp_scroll(false);

    poly_eeconf_t ee = load_user_eeconf();

    memset(&l_layer, 0, sizeof(l_layer));
    memset(&g_layer, 0, sizeof(g_layer));
    memset(&l_state, 0, sizeof(l_state));
    memset(&g_state, 0, sizeof(g_state));

    l_state.lang = ee.lang;
    l_state.contrast = ee.brightness;
    l_state.flags = set_flag(STATUS_DISP_ON, RGB_ON, rgb_matrix_is_enabled());

    memcpy(g_latin.ex, ee.latin_ex, sizeof(g_latin.ex));

    set_displays(l_state.contrast, false);
    l_last.latin_kc = 0;
    show_splash_screen();

    setPinInputHigh(I2C1_SDA_PIN);
}

void eeconfig_init_user(void) {
    uprint("Init EE config\n");
    poly_eeconf_t ee;
    ee.lang = g_lang_init;
    ee.brightness = ~FULL_BRIGHT;
    ee.unused = 0;
    memset(ee.latin_ex, 0, sizeof(ee.latin_ex));
    eeconfig_read_user_datablock(&ee);
}


void num_to_u16_string(char* buffer, uint8_t buffer_len, uint8_t value) {
    if(value<10) {
        snprintf((char*) buffer, buffer_len, "%d", value);
        buffer[1] = 0;
        buffer[2] = 0;
        buffer[3] = 0;
    } else if(value>99) {
        snprintf((char*) buffer, buffer_len, "%d %d %d", value/100, (value/10)%10, value%10);
        buffer[1] = 0;
        buffer[3] = 0;
        buffer[5] = 0;
        buffer[6] = 0;
        buffer[7] = 0;
    } else {
        snprintf((char*) buffer, buffer_len, "%d %d", value/10, value%10);
        buffer[1] = 0;
        buffer[3] = 0;
        buffer[4] = 0;
        buffer[5] = 0;
    }
}

void hex_to_u16_string(char* buffer, uint8_t buffer_len, uint8_t value) {
    if(value<16) {
        snprintf((char*) buffer, buffer_len, "%X", value);
        buffer[1] = 0;
        buffer[2] = 0;
        buffer[3] = 0;
    } else {
        snprintf((char*) buffer, buffer_len, "%X %X", value/16, value%16);
        buffer[1] = 0;
        buffer[3] = 0;
        buffer[4] = 0;
        buffer[5] = 0;
    }
}

void oled_update_buffer(void) {
    uint16_t buffer[32];

    kdisp_set_buffer(0);

    const GFXfont* displayFont[] = { &NotoSans_Regular11pt7b };
    const GFXfont* smallFont[] = { &NotoSans_Medium8pt7b };
    kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 0, 14, ICON_LAYER);
    hex_to_u16_string((char*) buffer, sizeof(buffer), get_highest_layer(g_layer.layer));
    kdisp_write_gfx_text(displayFont, 1, 20, 14, buffer);
    if(side==UNDECIDED) {
        kdisp_write_gfx_text(displayFont, 1, 50, 14, u"Uknw");
    } else {
        kdisp_write_gfx_text(displayFont, 1, 38, 14, is_left_side() ? u"LEFT" : u"RIGHT");
    }

    kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 108, 16, g_layer.led_state.num_lock ? ICON_NUMLOCK_ON : ICON_NUMLOCK_OFF);
    kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 108, 38, g_layer.led_state.caps_lock ? ICON_CAPSLOCK_ON : ICON_CAPSLOCK_OFF);
    if(g_layer.led_state.scroll_lock) {
        kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 112, 54, ARROWS_DOWNSTOP);
    } else {
        kdisp_write_gfx_text(smallFont, 1, 112, 56, is_usb_host_side() ? u"H" : u"B");
    }

    if(is_right_side()) {
        kdisp_write_gfx_text(smallFont, 1, 0, 30, u"RGB");

        if(!rgb_matrix_is_enabled()) {
            kdisp_write_gfx_text(smallFont, 1, 34, 30, u"Off");
        } else {
            num_to_u16_string((char*) buffer, sizeof(buffer), rgb_matrix_get_mode());
            kdisp_write_gfx_text(smallFont, 1, 34, 30, buffer);
            kdisp_write_gfx_text(smallFont, 1, 58, 30, get_led_matrix_text(rgb_matrix_get_mode()));
            kdisp_write_gfx_text(smallFont, 1, 0, 44, u"HSV");
            hex_to_u16_string((char*) buffer, sizeof(buffer), rgb_matrix_get_hue());
            kdisp_write_gfx_text(smallFont, 1, 38, 44, buffer);
            hex_to_u16_string((char*) buffer, sizeof(buffer), rgb_matrix_get_sat());
            kdisp_write_gfx_text(smallFont, 1, 60, 44, buffer);
            hex_to_u16_string((char*) buffer, sizeof(buffer), rgb_matrix_get_val());
            kdisp_write_gfx_text(smallFont, 1, 82, 44, buffer);
            kdisp_write_gfx_text(smallFont, 1, 0, 58, u"Speed");
            num_to_u16_string((char*) buffer, sizeof(buffer), rgb_matrix_get_speed());
            kdisp_write_gfx_text(smallFont, 1, 58, 58, buffer);
        }
    } else {
        switch(l_layer.def_layer) {
            case 0: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Qwerty"); break;
            case 1: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Qwerty Stag!"); break;
            case 2: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Colemak DH"); break;
            case 3: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Neo"); break;
            case 4: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Workman"); break;
            default: kdisp_write_gfx_text(smallFont, 1, 0, 30, u"Unknown"); break;
        }
        kdisp_write_gfx_text(smallFont, 1, 0, 44, u"Dsp*");
        num_to_u16_string((char*) buffer, sizeof(buffer), l_state.contrast);
        kdisp_write_gfx_text(smallFont, 1, 42, 44, buffer);
        uint8_t i=0;
        for(;i<(l_state.contrast/5);++i) {
            buffer[i] = 'l';
        }
        buffer[i] = 0;
        buffer[i+1] = 0;
        kdisp_write_gfx_text(smallFont, 1, 64, 44, buffer);

        kdisp_write_gfx_text(smallFont, 1, 0, 58, u"WPM");
        num_to_u16_string((char*) buffer, sizeof(buffer), get_current_wpm());
        kdisp_write_gfx_text(smallFont, 1, 44, 58, buffer);

        kdisp_write_gfx_text(smallFont, 1, 68, 58, u"L");
        num_to_u16_string((char*) buffer, sizeof(buffer), l_state.lang);
        kdisp_write_gfx_text(smallFont, 1, 84, 58, buffer);
    }
}

void oled_status_screen(void) {
     if( (l_state.flags&STATUS_DISP_ON) == 0) {
        oled_off();
        return;
    } else if( (l_state.flags&STATUS_DISP_ON) != 0) {
        oled_on();
    }

    oled_update_buffer();
    oled_clear();
    oled_write_raw((char *)get_scratch_buffer(), get_scratch_buffer_size());
}

void oled_render_logos(void) {
    if(is_left_side()) {
        // oled_draw_poly();
        oled_scroll_right();
    } else {
        // oled_draw_kybd();
        oled_scroll_left();
    }
}

bool oled_task_user(void) {
    if((l_state.flags&DISP_IDLE) != 0) {
        oled_render_logos();
    } else {
        oled_scroll_off();
        oled_status_screen();
    }
    return false;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (clockwise) {
      tap_code(KC_WH_U);
    } else {
      tap_code(KC_WH_D);
    }

    return false;
}

void invert_display(uint8_t r, uint8_t c, bool state) {
    uint16_t keycode = keymaps[_BL][r][0];
    if (keycode == KC_NO) {
        c--;
    }

    r = r % MATRIX_ROWS_PER_SIDE;
    uint8_t disp_idx = LAYOUT_TO_INDEX(r, c);
    const uint8_t* bitmask = key_display[disp_idx].bitmask;
    sr_shift_out_buffer_latch(bitmask, sizeof(key_display->bitmask));

    if (disp_idx != 255) {
        kdisp_invert(state);
    }
}

// invert displays directly when pressed (no need to do split sync)
extern matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t last_matrix[MATRIX_ROWS_PER_SIDE];
void matrix_scan_user(void) {
    uint8_t first   = is_left_side() ? 0 : MATRIX_ROWS_PER_SIDE;
    bool    changed = false;
    for (uint8_t r = first; r < first + MATRIX_ROWS_PER_SIDE; r++) {
        if (last_matrix[r - first] != matrix[r]) {
            changed = true;
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                bool old     = ((last_matrix[r - first] >> c) & 1) == 1;
                bool current = ((matrix[r] >> c) & 1) == 1;
                if (!old && current) {
                    invert_display(r,c,true);
                } else if (old && !current) {
                    invert_display(r,c,false);
                }
            }
        }
    }
    if (changed) {
        memcpy(last_matrix, &matrix[first], sizeof(last_matrix));
    }
}

void matrix_slave_scan_user(void) {
    matrix_scan_user();
}

oled_rotation_t oled_init_user(oled_rotation_t rotation){
    oled_off();
    oled_clear();
    oled_render();
    oled_scroll_set_speed(0);
    oled_render_logos();
    oled_on();
    return rotation;
}

void poly_suspend(void) {
    l_state.overlay_flags = flag_off(l_state.overlay_flags, DISPLAY_OVERLAYS);
    l_state.flags &= ~((uint8_t)STATUS_DISP_ON) & ~((uint8_t)DISP_IDLE);// & ~((uint8_t)RGB_ON);
    l_state.contrast = DISP_OFF;
}

void suspend_power_down_kb(void) {
    poly_suspend();
    rgb_matrix_disable_noeeprom();
    housekeeping_task_user();
    suspend_power_down_user();
    last_update = -1;
}


void suspend_wakeup_init_kb(void) {
    l_state.flags |= STATUS_DISP_ON;
    l_state.flags &= ~((uint8_t)DISP_IDLE);
    poly_eeconf_t ee = load_user_eeconf();
    l_state.contrast = ee.brightness;
    last_update = 0;

    //rgb_matrix_reload_from_eeprom();
    if(test_flag(l_state.flags, RGB_ON)) {
        rgb_matrix_enable_noeeprom();
    }

    update_performed();
    housekeeping_task_user();
    suspend_wakeup_init_user();
}

static uint16_t byte_counter = 0;
void fill_overlay_buffer(uint8_t keycode, uint8_t mods, uint8_t segment_0_to_14, uint8_t* buffer_24bytes) {
    if(keycode>KC_RGUI) {
        uprint("Warning: Supplied overlay keycode not supported.\n");
        return;
    }
    uint16_t idx = (keycode>KC_APP) ? (keycode - KC_LEFT_CTRL + 82) : (keycode>KC_NUM_LOCK ? keycode - KC_NUBS + 80 : keycode - KC_A);
    if(idx>=90) {
        uprint("Warning: Calculated index for overlay out of bounds. Dropping overlay.\n");
        return;
    }
    idx = adjust_overlay_idx_to_mod(idx, mods);
    memcpy(overlays[idx] + segment_0_to_14*BYTES_PER_SEGMENT, buffer_24bytes, BYTES_PER_SEGMENT);

    //send buffer to bridge
    // {
    //     overlay_sync_t transfer;
    //     transfer.segment = segment_0_to_14;
    //     transfer.adj_idx = idx;
    //     memcpy(&transfer.overlay, buffer_24bytes, BYTES_PER_SEGMENT);
    //     send_to_bridge(USER_SYNC_OVERLAY_DATA, (void*)&transfer, sizeof(transfer), 10);
    // }

    byte_counter += BYTES_PER_SEGMENT;
    if(segment_0_to_14==NUM_SEGMENTS_PER_OVERLAY-1) {
        use_overlay[idx] = true;
        uprintf("Received overlay for keycode 0x%x (modifiers: 0x%x): %d bytes, index %d.\n", keycode, mods, byte_counter, idx);
        byte_counter = 0;
    }
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    const char * name = "P\x06.Split72 " FW_VERSION " HW" STR(DEVICE_VER) " ";

    if(length>1 && (data[0] == /*via_command_id::*/id_custom_save || data[0] == 'P')) {
        switch(data[1]) {
            case id_custom_channel...id_qmk_led_matrix_channel: //unusable :(
                break;
            case 6: //id
                memset(data, 0, length);
                memcpy(data, name, strlen(name));
                break;
            case 7: //lang
                memset(data, 0, length);
                switch(l_state.lang) {
                    /*[[[cog
                    for lang in languages:
                        cog.outl(f'case {lang}: memcpy(data, "P\\x07.{lang[5:]}", 5); break;')
                    ]]]*/
                    case LANG_EN: memcpy(data, "P1.EN", 5); break;
                    case LANG_DE: memcpy(data, "P1.DE", 5); break;
                    case LANG_FR: memcpy(data, "P1.FR", 5); break;
                    case LANG_ES: memcpy(data, "P1.ES", 5); break;
                    case LANG_PT: memcpy(data, "P1.PT", 5); break;
                    case LANG_IT: memcpy(data, "P1.IT", 5); break;
                    case LANG_TR: memcpy(data, "P1.TR", 5); break;
                    case LANG_KO: memcpy(data, "P1.KO", 5); break;
                    case LANG_JA: memcpy(data, "P1.JA", 5); break;
                    case LANG_AR: memcpy(data, "P1.AR", 5); break;
                    case LANG_GR: memcpy(data, "P1.GR", 5); break;
                    //[[[end]]]
                    default:
                        memcpy(data, "P\x07!", 3);
                        break;
                }
                break;
            case 8: //lang list
                memset(data, 0, length);
                /*[[[cog
                lang_list = "P\\x08."
                for lang in languages:
                    lang_list += lang[5:]
                    if len(lang_list)>=(32-3-1):
                        cog.outl(f'memcpy(data, "{lang_list}", {len(lang_list)});')
                        cog.outl(f'raw_hid_send(data, length);')
                        cog.outl(f'memset(data, 0, length);')
                        lang_list = "P8."
                    elif lang != languages[-1]:
                        lang_list += ","
                cog.outl(f'memcpy(data, "{lang_list}", {len(lang_list)});')
                ]]]*/
                memcpy(data, "P\x08.EN,DE,FR,ES,PT,IT,TR,KO,JA", 29);
                raw_hid_send(data, length);
                memset(data, 0, length);
                memcpy(data, "P\x08.AR,GR", 8);
                //[[[end]]]
                break;
            case 9: //change language
                if(data[3]< NUM_LANG) {
                    l_state.lang = data[3];
                    uprintf("Setting lang to %u.\n", data[3]);
                    request_disp_refresh();
                    update_performed();
                    memcpy(data, "P\x09.", 3);
                } else {
                    uprintf("Invalid language index %u.\n", data[3]);
                    memset(data, 0, length);
                    memcpy(data, "P\x09!", 3);
                }
                break;
            case 10: //receive overlay e.g. P4:\4\4
                {
                    uint8_t keycode = data[3];
                    uint8_t segment = data[5];
                    if(keycode>=KC_A && keycode<=KC_RIGHT_GUI && segment<NUM_SEGMENTS_PER_OVERLAY) {
                        uint8_t mods = data[4];
                        if((mods & MOD_MASK_GUI)==0) { //for now we filter out the gui key
                            fill_overlay_buffer(keycode, mods,segment, &data[6]);
                            if(segment==NUM_SEGMENTS_PER_OVERLAY-1) {
                                update_performed();
                                request_disp_refresh();
                            }
                        }
                        memcpy(data, "P\x0a.", 3);
                    } else {
                        memset(data, 0, length);
                        memcpy(data, "P\x0a!", 3);
                    }
                }
                break;
            case 11: //overlays flags on
                {
                    const bool vis_changed = test_flag(data[3]^l_state.overlay_flags, DISPLAY_OVERLAYS);
                    l_state.overlay_flags = flag_on(l_state.overlay_flags, data[3]);
                    if(vis_changed) {
                        housekeeping_task_user();
                    }
                    memcpy(data, "P\x0b.", 3);
                    uprintf("Overlay flags 0x%x set.\n", data[3]);
                }
                break;

            case 12: //overlays flags off
                {
                    const bool vis_changed = test_flag(data[3]^l_state.overlay_flags, DISPLAY_OVERLAYS);
                    l_state.overlay_flags = flag_off(l_state.overlay_flags, data[3]);
                    if(vis_changed) {
                        housekeeping_task_user();
                    }
                    memcpy(data, "P\x0c.", 3);
                    uprintf("Overlay flags 0x%x cleared.\n", data[3]);
                }
                break;
            case 13: //set brightness
                if ( data[3] <= FULL_BRIGHT) {
                    l_state.contrast = data[3];
                    save_user_eeconf();
                    memcpy(data, "P\x0d.", 3);
                    uprintf("Set brightness to: %u.\n", data[3]);
                } else {
                    memcpy(data, "P\x0d!", 3);
                    uprintf("Refused to set brightness to: %u.\n", data[3]);
                }
                break;
            case 14: //key press
                {
                    uint16_t keycode = ((uint16_t)data[3])<<8 | data[4];
                    if(data[5]==0) {
                        register_code(keycode);
                        printf("Registered keycode: %u.\n", keycode);
                    } else {
                        unregister_code(keycode);
                        printf("Unregistered keycode: %u.\n", keycode);
                    }
                    memcpy(data, "P\x0e.", 3);
                }
                break;
            case 15: //start/stop idle
                if(data[3]==0) {
                    if((l_state.flags & (STATUS_DISP_ON|DISP_IDLE))==0) {
                        suspend_wakeup_init_kb();
                    } else {
                        l_state.flags &= ~((uint8_t)DISP_IDLE);
                        request_disp_refresh();
                        update_performed();
                    }
                    uprint("Stop idle.\n");
                } else {
                    last_update = timer_read32() - FADE_OUT_TIME;
                    if(last_update<0) {
                        uprintf("Starting idle in %ld msec .\n", -last_update);
                        last_update=0;
                    } else {
                        uprint("Start idle.\n");
                    }
                }
                memcpy(data, "P\x0f.", 3);
                break;
            default:
                printf("Unknown command: %u.\n", data[1]);
                break;
        }
        #ifndef VIA_ENABLE
            raw_hid_send(data, length);
        #endif
    }
}
#ifndef VIA_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length) {
    via_custom_value_command_kb(data, length);
}
#endif