#include QMK_KEYBOARD_H

// #include "polymorph.h"
// #include "morph/keymaps/default/uni.h"

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

#include <hardware/flash.h>

#include "lang/lang_lut.h"

#define FW_VERSION "0.0.1"

#define FLASH_TARGET_OFFSET (4 * 1024 * 1024)
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static_assert(FLASH_PAGE_SIZE==256, "Flash page size changed");

#define FULL_BRIGHT 50
#define MIN_BRIGHT 1
#define DISP_OFF 0
#define BRIGHT_STEP 1

#define FADE_TRANSITION_TTIME 10000

#define FADE_OUT_TIME 120000

#define TURN_OFF_TIME 1200000


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

enum kb_layers {
    _BL = 0x00
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = LAYOUT(
        KC_RIGHT_ALT, KC_Q, KC_LSFT,
        KC_3, KC_4, KC_5
    )
};


struct display_info {
    uint8_t bitmask[NUM_SHIFT_REGISTERS];
};

#define BITMASK1(x) .bitmask = {(uint8_t)(~(1<<x))}

const struct display_info key_display[] = {
    {BITMASK1(2)}, {BITMASK1(2)}, {BITMASK1(2)},
    {BITMASK1(2)}, {BITMASK1(2)}, {BITMASK1(2)}
};

const struct display_info disp_row_0 = { BITMASK1(0) };

enum refresh_mode { START_FRIST_HALF, START_SECOND_HALF, DONE_ALL, ALL_AT_ONCE };
static enum refresh_mode g_refresh = DONE_ALL;

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

typedef struct _poly_sync_t {
    uint32_t crc32;
    uint8_t lang;
    uint8_t contrast;
    uint8_t flags;
    uint8_t overlay_flags;
    uint8_t unicode_mode;
} poly_sync_t;
static poly_sync_t l_state;


typedef struct _poly_last_t {
    uint32_t crc32;
    uint16_t latin_kc;
} poly_last_t;
// static poly_last_t l_last;

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


bool display_wakeup(keyrecord_t* record);
void update_displays(enum refresh_mode mode);
void set_displays(uint8_t contrast, bool idle);
void set_selected_displays(int8_t old_value, int8_t new_value);
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
    if (ee.brightness > FULL_BRIGHT) {
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
    if (l_state.contrast > (MIN_BRIGHT + BRIGHT_STEP)) {
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


// const uint16_t* keycode_to_disp_text(uint16_t keycode, led_t state) {
//     switch (keycode) {
//     case UM(0): return u" " PRIVATE_EMOJI_1F600;
//     case UM(1): return u" " PRIVATE_EMOJI_1F601;
//     case UM(2): return u" " PRIVATE_EMOJI_1F602;
//     case UM(3): return u" " PRIVATE_EMOJI_1F603;
//     case UM(4): return u" " PRIVATE_EMOJI_1F604;
//     case UM(5): return u" " PRIVATE_EMOJI_1F605;
//     case UM(6): return u" " PRIVATE_EMOJI_1F606;
//     case UM(7): return u" " PRIVATE_EMOJI_1F607;
//     case UM(8): return u" " PRIVATE_EMOJI_1F608;
//     case UM(9): return u" " PRIVATE_EMOJI_1F609;
//     case UM(10): return u" " PRIVATE_EMOJI_1F60A;
//     case UM(11): return u" " PRIVATE_EMOJI_1F60B;
//     case UM(12): return u" " PRIVATE_EMOJI_1F60C;
//     case UM(13): return u" " PRIVATE_EMOJI_1F60D;
//     case UM(14): return u" " PRIVATE_EMOJI_1F60E;
//     case UM(15): return u" " PRIVATE_EMOJI_1F60F;
//     case UM(16): return u" " PRIVATE_EMOJI_1F610;
//     case UM(17): return u" " PRIVATE_EMOJI_1F611;
//     case UM(18): return u" " PRIVATE_EMOJI_1F612;
//     case UM(19): return u" " PRIVATE_EMOJI_1F613;
//     case UM(20): return u" " PRIVATE_EMOJI_1F614;
//     case UM(21): return u" " PRIVATE_EMOJI_1F615;
//     case UM(22): return u" " PRIVATE_EMOJI_1F616;
//     case UM(23): return u" " PRIVATE_EMOJI_1F617;
//     case UM(24): return u" " PRIVATE_EMOJI_1F618;
//     case UM(25): return u" " PRIVATE_EMOJI_1F619;
//     case UM(26): return u" " PRIVATE_EMOJI_1F61A;
//     case UM(27): return u" " PRIVATE_EMOJI_1F61B;
//     case UM(28): return u" " PRIVATE_EMOJI_1F61C;
//     case UM(29): return u" " PRIVATE_EMOJI_1F61D;
//     case UM(30): return u" " PRIVATE_EMOJI_1F61E;
//     case UM(31): return u" " PRIVATE_EMOJI_1F61F;
//     case UM(32): return u" " PRIVATE_EMOJI_1F620;
//     case UM(33): return u" " PRIVATE_EMOJI_1F621;
//     case UM(34): return u" " PRIVATE_EMOJI_1F622;
//     case UM(35): return u" " PRIVATE_EMOJI_1F623;
//     case UM(36): return u" " PRIVATE_EMOJI_1F624;
//     case UM(37): return u" " PRIVATE_EMOJI_1F625;
//     case UM(38): return u" " PRIVATE_EMOJI_1F626;
//     case UM(39): return u" " PRIVATE_EMOJI_1F627;
//     case UM(40): return u" " PRIVATE_EMOJI_1F628;
//     case UM(41): return u" " PRIVATE_EMOJI_1F629;
//     case UM(42): return u" " PRIVATE_EMOJI_1F62A;
//     case UM(43): return u" " PRIVATE_EMOJI_1F62B;
//     case UM(44): return u" " PRIVATE_EMOJI_1F62C;
//     case UM(45): return u" " PRIVATE_EMOJI_1F62D;
//     case UM(46): return u" " PRIVATE_EMOJI_1F62E;
//     case UM(47): return u" " PRIVATE_EMOJI_1F62F;
//     case UM(48): return u" " PRIVATE_EMOJI_1F630;
//     case UM(49): return u" " PRIVATE_EMOJI_1F631;
//     case UM(50): return u" " PRIVATE_EMOJI_1F632;
//     case UM(51): return u" " PRIVATE_EMOJI_1F633;
//     case UM(52): return u" " PRIVATE_EMOJI_1F634;
//     case UM(53): return u" " PRIVATE_EMOJI_1F635;
//     case UM(54): return u" " PRIVATE_EMOJI_1F636;
//     case UM(55): return u" " PRIVATE_EMOJI_1F637;
//     case UM(56): return u" " PRIVATE_EMOJI_1F638;
//     case UM(57): return u" " PRIVATE_EMOJI_1F639;
//     case UM(58): return u" " PRIVATE_EMOJI_1F644;
//     case UM(59): return u" " PRIVATE_EMOJI_1F645;
//     case UM(60): return u" " PRIVATE_EMOJI_1F646;
//     case UM(61): return u" " PRIVATE_EMOJI_1F647;
//     case UM(62): return u" " PRIVATE_EMOJI_1F648;
//     case UM(63): return u" " PRIVATE_EMOJI_1F649;
//     case UM(64): return u" " PRIVATE_EMOJI_1F64A;
//     case UM(65): return u" " PRIVATE_EMOJI_1F64B;
//     case UM(66): return u" " PRIVATE_EMOJI_1F64C;
//     case UM(67): return u" " PRIVATE_EMOJI_1F64D;
//     case UM(68): return u" " PRIVATE_EMOJI_1F64E;
//     case UM(69): return u" " PRIVATE_EMOJI_1F64F;
//     case UM(70): return u" " PRIVATE_EMOJI_1F440;
//     case UM(71): return u" " PRIVATE_EMOJI_1F441;
//     case UM(72): return u" " PRIVATE_EMOJI_1F442;
//     case UM(73): return u" " PRIVATE_EMOJI_1F443;
//     case UM(74): return u" " PRIVATE_EMOJI_1F444;
//     case UM(75): return u" " PRIVATE_EMOJI_1F445;
//     case UM(76): return u" " PRIVATE_EMOJI_1F446;
//     case UM(77): return u" " PRIVATE_EMOJI_1F447;
//     case UM(78): return u" " PRIVATE_EMOJI_1F448;
//     case UM(79): return u" " PRIVATE_EMOJI_1F449;
//     case UM(80): return u" " PRIVATE_EMOJI_1F44A;
//     case UM(81): return u" " PRIVATE_EMOJI_1F44B;
//     case UM(82): return u" " PRIVATE_EMOJI_1F44C;
//     case UM(83): return u" " PRIVATE_EMOJI_1F44D;
//     case UM(84): return u" " PRIVATE_EMOJI_1F44E;
//     case UM(85): return u" " PRIVATE_EMOJI_1F44F;
//     case UM(86): return u" " PRIVATE_EMOJI_1F450;
//     case UM(87): return u" " PRIVATE_EMOJI_1F451;
//     case UM(88): return u" " PRIVATE_EMOJI_1F452;
//     case UM(89): return u" " PRIVATE_EMOJI_1F453;
//     case UM(90): return u" " PRIVATE_EMOJI_1F47B;
//     case UM(91): return u" " PRIVATE_EMOJI_1F47C;
//     case UM(92): return u" " PRIVATE_EMOJI_1F47D;
//     case UM(93): return u" " PRIVATE_EMOJI_1F47E;
//     case UM(94): return u" " PRIVATE_EMOJI_1F47F;
//     case UM(95): return u" " PRIVATE_EMOJI_1F480;
//     case UM(96): return u" " PRIVATE_EMOJI_1F481;
//     case UM(97): return u" " PRIVATE_EMOJI_1F482;
//     case UM(98): return u" " PRIVATE_EMOJI_1F483;
//     case UM(99): return u" " PRIVATE_EMOJI_1F484;
//     case UM(100): return u" " PRIVATE_EMOJI_1F485;
//     case UM(101): return u" " PRIVATE_EMOJI_1F489;
//     case UM(102): return u" " PRIVATE_EMOJI_1F48A;
//     case UM(103): return u" " PRIVATE_EMOJI_1F48B;
//     case UM(104): return u" " PRIVATE_EMOJI_1F48C;
//     case UM(105): return u" " PRIVATE_EMOJI_1F48D;
//     case UM(106): return u" " PRIVATE_EMOJI_1F48E;
//     case UM(107): return u" " PRIVATE_EMOJI_1F48F;
//     case UM(108): return u" " PRIVATE_EMOJI_1F490;
//     case UM(109): return u" " PRIVATE_EMOJI_1F491;
//     case UM(110): return u" " PRIVATE_EMOJI_1F492;
//     case UM(111): return u" " PRIVATE_EMOJI_1F493;
//     case UM(112): return u" " PRIVATE_EMOJI_1F494;
//     case UM(113): return u" " PRIVATE_EMOJI_1F495;
//     case UM(114): return u" " PRIVATE_EMOJI_1F496;
//     case UM(115): return u" " PRIVATE_EMOJI_1F4A1;
//     case UM(116): return u" " PRIVATE_EMOJI_1F4A2;
//     case UM(117): return u" " PRIVATE_EMOJI_1F4A3;
//     case UM(118): return u" " PRIVATE_EMOJI_1F4A4;
//     case UM(119): return u" " PRIVATE_EMOJI_1F4A5;
//     case UM(120): return u" " PRIVATE_EMOJI_1F4A6;
//     case UM(121): return u" " PRIVATE_EMOJI_1F4A7;
//     case UM(122): return u" " PRIVATE_EMOJI_1F4A8;
//     case UM(123): return u" " PRIVATE_EMOJI_1F4A9;
//     case UM(124): return u" " PRIVATE_EMOJI_1F4AA;
//     case UM(125): return u" " PRIVATE_EMOJI_1F4AB;
//     case UM(126): return u" " PRIVATE_EMOJI_1F4AC;
//     case UM(127): return u" " PRIVATE_EMOJI_1F4AD;
//     case UM(128): return u" " PRIVATE_EMOJI_1F4AE;
//     case UM(129): return u" " PRIVATE_EMOJI_1F4AF;
//     case UM(130): return u" " PRIVATE_EMOJI_1F4B0;
//     case UM(131): return u" " PRIVATE_EMOJI_1F4B1;
//     case UM(132): return u" " PRIVATE_EMOJI_1F912;
//     case UM(133): return u" " PRIVATE_EMOJI_1F913;
//     case UM(134): return u" " PRIVATE_EMOJI_1F914;
//     case UM(135): return u" " PRIVATE_EMOJI_1F915;
//     case UM(136): return u" " PRIVATE_EMOJI_1F916;
//     case UM(137): return u" " PRIVATE_EMOJI_1F917;
//     case UM(138): return u" " PRIVATE_EMOJI_1F918;
//     case UM(139): return u" " PRIVATE_EMOJI_1F919;
//     case TO(_EMJ0): return u" " PRIVATE_EMOJI_1F600 u"\v" ICON_LAYER;
//     case TO(_EMJ1): return u" " PRIVATE_EMOJI_1F440 u"\v" ICON_LAYER;
//     case KC_DEADKEY: return (l_state.flags&DEAD_KEY_ON_WAKEUP)==0 ? u"WakeX\r\v" ICON_SWITCH_OFF : u"WakeX\r\v" ICON_SWITCH_ON;
//     case LBL_TEXT: return u"Text:";
//     case KC_TOGMODS: return (l_state.flags&MODS_AS_TEXT)==0 ? u"Mods\r\v" ICON_SWITCH_OFF : u"Mods\r\v" ICON_SWITCH_ON;
//     case KC_TOGTEXT: return (l_state.flags&MORE_TEXT)==0 ? u"Cmds\r\v" ICON_SWITCH_OFF : u"Cmds\r\v" ICON_SWITCH_ON;
//     case QK_UNICODE_MODE_MACOS:
//         return l_state.unicode_mode==UNICODE_MODE_MACOS ? u"Mac\r\v" ICON_SWITCH_ON : u"Mac\r\v" ICON_SWITCH_OFF;
//     case QK_UNICODE_MODE_LINUX:
//         return l_state.unicode_mode==UNICODE_MODE_LINUX ? u"Lnx\r\v" ICON_SWITCH_ON : u"Lnx\r\v" ICON_SWITCH_OFF;
//     case QK_UNICODE_MODE_WINDOWS:
//         return l_state.unicode_mode==UNICODE_MODE_WINDOWS ? u"Win\r\v" ICON_SWITCH_ON : u"Win\r\v" ICON_SWITCH_OFF;
//     case QK_UNICODE_MODE_BSD:
//         return l_state.unicode_mode==UNICODE_MODE_BSD ? u"BSD\r\v" ICON_SWITCH_ON : u"BSD\r\v" ICON_SWITCH_OFF;
//     case QK_UNICODE_MODE_WINCOMPOSE:
//         return l_state.unicode_mode==UNICODE_MODE_WINCOMPOSE ? u"WinC\r\v" ICON_SWITCH_ON : u"WinC\r\v" ICON_SWITCH_OFF;
//     case QK_UNICODE_MODE_EMACS:
//         return l_state.unicode_mode==UNICODE_MODE_EMACS ? u"Emcs\r\v" ICON_SWITCH_ON : u"Emcs\r\v" ICON_SWITCH_OFF;
//     case QK_LEAD:
//         return u"Lead";
//     case KC_HYPR:
//         return (l_state.flags&MORE_TEXT)!=0 ? u"Hypr" : u" " PRIVATE_HYPER;
//     case KC_MEH:
//         return (l_state.flags&MORE_TEXT)!=0 ? u"Meh" : u" " PRIVATE_MEH;
//     case KC_EXEC:
//         return u"Exec";
//     case KC_NUM_LOCK:
//         return !state.num_lock ? u"Nm" ICON_NUMLOCK_OFF : u"Nm" ICON_NUMLOCK_ON;
//     case KC_KP_SLASH:
//         return u"/";
//     case KC_KP_ASTERISK:
//     case KC_KP_MINUS:
//         return u"-";
//     case KC_KP_7:
//         return !state.num_lock ? ARROWS_LEFTSTOP : u"7";
//     case KC_KP_8:
//         return !state.num_lock ? u"   " ICON_UP : u"8";
//     case KC_KP_9:
//         return !state.num_lock ? ARROWS_UPSTOP : u"9";
//     case KC_KP_PLUS:
//         return u"+";
//     case KC_KP_4:
//         return !state.num_lock ? u"  " ICON_LEFT : u"4";
//     case KC_KP_5:
//         return !state.num_lock ? u"" : u"5";
//     case KC_KP_6:
//         return !state.num_lock ? u"  " ICON_RIGHT : u"6";
//     case KC_KP_EQUAL:
//         return u"=";
//     case KC_KP_1:
//         return !state.num_lock ? ARROWS_RIGHTSTOP : u"1";
//     case KC_KP_2:
//         return !state.num_lock ? u"   " ICON_DOWN : u"2";
//     case KC_KP_3:
//         return !state.num_lock ? ARROWS_DOWNSTOP : u"3";
//     case KC_CALCULATOR:
//         return u"   " PRIVATE_CALC;
//     case KC_KP_0:
//         return !state.num_lock ? u"Ins" : u"0";
//     case KC_KP_DOT:
//         return !state.num_lock ? u"Del" : u".";
//     case KC_KP_ENTER:
//         return u"Enter";
//     case QK_BOOTLOADER:
//         return u"Boot";
//     case QK_DEBUG_TOGGLE:
//         return ((l_state.flags & DBG_ON) == 0) ? u"Dbg\r\v" ICON_SWITCH_OFF : u"Dbg\r\v" ICON_SWITCH_ON;
//     case RGB_RMOD:
//         return u" " ICON_LEFT PRIVATE_LIGHT;
//     case KC_RGB_TOG:
//         return u"  I/O";
//     case RGB_MOD:
//         return PRIVATE_LIGHT ICON_RIGHT;
//     case RGB_HUI:
//         return u"Hue+";
//     case RGB_HUD:
//         return u"Hue-";
//     case RGB_SAI:
//         return u"Sat+";
//     case RGB_SAD:
//         return u"Sat-";
//     case RGB_VAI:
//         return u"Bri+";
//     case RGB_VAD:
//         return u"Bri-";
//     case RGB_SPI:
//         return u"Spd+";
//     case RGB_SPD:
//         return u"Spd-";
//     case RGB_MODE_PLAIN:
//         return u"Plan";
//     case RGB_MODE_BREATHE:
//         return u"Brth";
//     case RGB_MODE_SWIRL:
//         return u"Swrl";
//     case RGB_MODE_RAINBOW:
//         return u"Rnbw";
//     case KC_MEDIA_NEXT_TRACK:
//         return ICON_RIGHT ICON_RIGHT;
//     case KC_MEDIA_PLAY_PAUSE:
//         return u"  " ICON_RIGHT;
//     case KC_MEDIA_STOP:
//         return u"Stop";
//     case KC_MEDIA_PREV_TRACK:
//         return ICON_LEFT ICON_LEFT;
//     case KC_MS_ACCEL0:
//         return u">>";
//     case KC_MS_ACCEL1:
//         return u">>>";
//     case KC_MS_ACCEL2:
//         return u">>>>";
//     case KC_BTN1:
//         return u"  " ICON_LMB;
//     case KC_BTN2:
//         return u"  " ICON_RMB;
//     case KC_BTN3:
//         return u"  " ICON_MMB;
//     case KC_MS_UP:
//         return u"  " ICON_UP;
//     case KC_MS_DOWN:
//         return u"  " ICON_DOWN;
//     case KC_MS_LEFT:
//         return u"  " ICON_LEFT;
//     case KC_MS_RIGHT:
//         return u"  " ICON_RIGHT;
//     case KC_AUDIO_MUTE:
//         return u"  " PRIVATE_MUTE;
//     case KC_AUDIO_VOL_DOWN:
//         return u"  " PRIVATE_VOL_DOWN;
//     case KC_AUDIO_VOL_UP:
//         return u"  " PRIVATE_VOL_UP;
//     case KC_PRINT_SCREEN:
//         return u"  " PRIVATE_IMAGE;
//     case KC_SCROLL_LOCK:
//         return u"ScLk";
//     case KC_PAUSE:
//         return u"Paus";
//     case KC_INSERT:
//         return u"Ins";
//     case KC_HOME:
//         return ARROWS_LEFTSTOP;
//     case KC_END:
//         return u"   " ARROWS_RIGHTSTOP;
//     case KC_PAGE_UP:
//         return u"  " ARROWS_UPSTOP;
//     case KC_PAGE_DOWN:
//         return u"  " ARROWS_DOWNSTOP;
//     case KC_DELETE:
//         return (l_state.flags&MORE_TEXT)!=0 ? u"Del" : TECHNICAL_ERASERIGHT;
//     case KC_MYCM:
//         return u"  " PRIVATE_PC;
//     case TO(_SL):
//         return PRIVATE_SETTINGS u"\v" ICON_LAYER;
//     case MO(_FL0):
//     case MO(_FL1):
//         return u"Fn\r\v\t" ICON_LAYER;
//     case TO(_NL):
//         return u"Nm\r\v\t" ICON_LAYER;
//     case MO(_NL):
//         return u"Nm!\r\v\t" ICON_LAYER;
//     case KC_BASE:
//         return u"Base\r\v\t" ICON_LAYER;
//     case OSL(_UL):
//         return u"Util*\r\v\t" ICON_LAYER;
//     case TO(_UL):
//         return u"Util\r\v\t" ICON_LAYER;
//     case MO(_ADDLANG1):
//         return u"Intl";
//     case KC_F1: return u" F1";
//     case KC_F2: return u" F2";
//     case KC_F3: return u" F3";
//     case KC_F4: return u" F4";
//     case KC_F5: return u" F5";
//     case KC_F6: return u" F6";
//     case KC_F7: return u" F7";
//     case KC_F8: return u" F8";
//     case KC_F9: return u" F9";
//     case KC_F10:return u" F10";
//     case KC_F11:return u" F11";
//     case KC_F12:return u" F12";
//     case KC_F13:return u" F13";
//     case KC_F14:return u" F14";
//     case KC_F15:return u" F15";
//     case KC_F16:return u" F16";
//     case KC_F17:return u" F17";
//     case KC_F18:return u" F18";
//     case KC_F19:return u" F19";
//     case KC_F20:return u" F20";
//     case KC_F21:return u" F21";
//     case KC_F22:return u" F22";
//     case KC_F23:return u" F23";
//     case KC_F24:return u" F24";
//     case KC_LEFT_CTRL:
//     case KC_RIGHT_CTRL:
//         return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Ctrl" : TECHNICAL_CONTROL;
//     case KC_LEFT_ALT:
//         return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Alt" : TECHNICAL_OPTION;
//     case KC_RIGHT_ALT:
//         return (l_state.flags&MODS_AS_TEXT)!=0 ? ( l_state.lang == KC_LANG_EN ? u"Alt" : u"Alt\nGr") : TECHNICAL_OPTION;
//     case KC_LGUI:
//     case KC_RGUI:
//         return (l_state.flags&MODS_AS_TEXT)!=0 ? u"GUI" : DINGBAT_BLACK_DIA_X;
//     case KC_LEFT:
//         return u"  " ICON_LEFT;
//     case KC_RIGHT:
//         return u"  " ICON_RIGHT;
//     case KC_UP:
//         return u"  " ICON_UP;
//     case KC_DOWN:
//         return u"  " ICON_DOWN;
//     case KC_CAPS_LOCK:
//         return state.caps_lock ? u"Cap" ICON_CAPSLOCK_ON : u"Cap" ICON_CAPSLOCK_OFF;
//     case KC_LSFT:
//     case KC_RSFT:
//         return (l_state.flags&MODS_AS_TEXT)!=0 ? u"Shft" : u" " ICON_SHIFT;
//     //The following entries will over-rule language specific enties in the follow langauge lookup table,
//     //however with this we can control them by flags and so far those wehere not lanuage specific anyway.
//     case KC_ENTER:      return (l_state.flags&MORE_TEXT)!=0 ? u"Enter"    : ARROWS_RETURN;
//     case KC_ESCAPE:	    return (l_state.flags&MORE_TEXT)!=0 ? u"Esc"      : TECHNICAL_ESCAPE;
//     case KC_BACKSPACE:  return (l_state.flags&MORE_TEXT)!=0 ? u"Bksp"     : TECHNICAL_ERASELEFT;
//     case KC_TAB:        return (l_state.flags&MORE_TEXT)!=0 ? u"Tab"      : ARROWS_TAB;
//     case KC_SPACE:      return (l_state.flags&MORE_TEXT)!=0 ? u"Space"    : ICON_SPACE;
//     default:
//     {
//         bool shift = ((l_layer.mods & MOD_MASK_SHIFT) != 0);
//         // bool add_lang = get_highest_layer(l_layer.layer)==_ADDLANG1;
//         // bool alt = ((l_layer.mods & MOD_MASK_ALT) != 0);
//         if(keycode>=KC_A ) {//&& keycode<=KC_Z && add_lang) {
//             //display the previously selected latin variation of the letter
//             const uint8_t offset = (shift || state.caps_lock) ? 0 : 26;
//             uint8_t variation = (shift || state.caps_lock) ? g_latin.ex[keycode-KC_A]>>4 : g_latin.ex[keycode-KC_A]&0xf;

//             const uint16_t* def_variation = latin_ex_map[offset+keycode-KC_A][0];
//             return (def_variation!=NULL) ? latin_ex_map[offset+keycode-KC_A][variation] : NULL;
//         }

//         // if(keycode>=KC_LAT0 && keycode<=KC_LAT9) {
//         //     if(add_lang && alt && l_last.latin_kc!=0) {
//         //         //show all available alternatives for selected latin letter
//         //         const uint8_t offset = (shift || state.caps_lock) ? 0 : 26;
//         //         return latin_ex_map[offset+l_last.latin_kc-KC_A][keycode-KC_LAT0];
//         //     } else {
//         //         return NULL; //show nothing
//         //     }
//         // }

//         const uint16_t* text = translate_keycode(l_state.lang, keycode, shift, state.caps_lock);
//         if (text != NULL) {
//             return text;
//         }
//     }
//     break;
//     }
//     return NULL;
// }

const uint16_t* keycode_to_disp_text(uint16_t keycode, led_t state, uint8_t mods, bool shift, bool altgr) {
    switch (keycode) {
        case KC_LEFT_CTRL:
        case KC_RIGHT_CTRL:
            return (l_state.flags & MODS_AS_TEXT) != 0 ? u"Ctrl" : TECHNICAL_CONTROL;
        case KC_LEFT_ALT:
            return (l_state.flags & MODS_AS_TEXT) != 0 ? u"Alt" : TECHNICAL_OPTION;
        case KC_RIGHT_ALT:
            return (l_state.flags & MODS_AS_TEXT) != 0 ? (l_state.lang == LANG_EN ? u"Alt" : u"Alt\nGr") : TECHNICAL_OPTION;
        case KC_LSFT:
        case KC_RSFT:
            return (l_state.flags & MODS_AS_TEXT) != 0 ? u"Shft" : u" " ICON_SHIFT;
        default:
        {
            //bool shift = ((mods & MOD_MASK_SHIFT) != 0);
            //bool altgr = ((mods & MOD_BIT_RALT) != 0);

            const uint16_t* text = translate_keycode_new(LANG_DE, keycode, shift, altgr, state.caps_lock);
            if (text != NULL) {
                return text;
            }
        }
        break;
    }

    return NULL;
}

size_t u16_strlen(const uint16_t *str) {
    size_t len = 0;
    while (str[len] != 0) len++;
    return len;
}

uint16_t* u16strcpy(uint16_t* dest, const uint16_t* src)
{
    uint16_t* d = dest;
    while ((*d++ = *src++) != 0);
    return dest;
}

uint16_t *u16_strcat(uint16_t *dest, const uint16_t *src) {
    uint16_t *d = dest + u16_strlen(dest);
    while ((*d++ = *src++) != 0);
    return dest;
}

void update_displays(enum refresh_mode mode) {
    if (l_state.contrast <= DISP_OFF || (l_state.flags & DISP_IDLE) != 0) {
        return;
    }

    const led_t state = l_layer.led_state;
    const uint8_t mods = l_layer.mods;
    bool shift = ((mods & MOD_MASK_SHIFT) != 0);
    bool altgr = ((mods & MOD_BIT_RALT) != 0);

    sr_shift_out_buffer_latch(disp_row_0.bitmask, sizeof(struct display_info));

    uint8_t skip = 0;
    for (uint8_t r = 0; r < MATRIX_ROWS; ++r) {
        for (uint8_t c = 0; c < MATRIX_COLS; ++c) {
            uint8_t  disp_idx = LAYOUT_TO_INDEX(r, c);

            //since MATRIX_COLS==8 we don't need to shift multiple times at the end of the row
            //except there was a leading and missing physical key (KC_NO on base layer)
            uint16_t keycode = keymaps[_BL][r][c - 1];
            if (keycode == KC_NO) {
                skip++;
            }
            else {
                if (disp_idx != 255) {
                    uint8_t layer = get_highest_layer(l_layer.layer);
                    uint16_t highest_kc = keymaps[layer][r][c - 1];
                    //if we encounter a transparent key go down one layer (but only one!)
                    keycode = (highest_kc == KC_TRNS) ? keymaps[get_highest_layer(l_layer.layer&~(1<<layer))][r][c - 1] : highest_kc;
                    kdisp_enable(true);//(l_state.flags&STATUS_DISP_ON) != 0);
                    kdisp_set_contrast(l_state.contrast - 1);
                    if(keycode != KC_TRNS) {
                        const uint16_t* text = keycode_to_disp_text(keycode, state, mods, shift, altgr);//keycode_to_disp_text(keycode, state);
                        kdisp_set_buffer(0x00);

                        uint16_t buffer[10];
                        u16strcpy(buffer, text);

                        if (!shift && !altgr) {
                            const uint16_t* op = keycode_to_disp_text(keycode, state, mods, true, false);

                            if (op != text) {
                                // u16_strcat(buffer, u"  ");
                                // uint16_t buffer2[5];
                                // u16strcpy(buffer2, op);
                                // u16_strcat(buffer, buffer2);

                                kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 50, 23, op);
                            }

                            const uint16_t* alt = keycode_to_disp_text(keycode, state, mods, false, true);
                            if (alt != text && alt != op) {
                                // u16_strcat(buffer, u"  ");
                                // uint16_t buffer2[5];
                                // u16strcpy(buffer2, op);
                                // u16_strcat(buffer, buffer2);

                                kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 75, 30, alt);
                            }
                        }

                        // if(!overlay_only) {
                            // if(text==NULL){ 
                            //     if((keycode&QK_UNICODEMAP_PAIR)!=0){
                            //         uint16_t chr = capital_case ? QK_UNICODEMAP_PAIR_GET_SHIFTED_INDEX(keycode) : QK_UNICODEMAP_PAIR_GET_UNSHIFTED_INDEX(keycode);
                            //         kdisp_write_gfx_char(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, unicode_map[chr]);
                            //     }
                            // } else {
                            //     kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, text);
                            // }
                        // }
                        // text = NULL;
                        // if(display_overlays) {
                        //     if(!copy_overlay_to_buffer(keycode, mods, combine)) {
                        //         text = keycode_to_disp_overlay(keycode, state); //fallback to hardcoded
                        //     }
                        // } else {
                        //     text = keycode_to_disp_overlay(keycode, state); //this should maybe go away - or setting?
                        // }
                        // if(text) {
                        
                        kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, buffer);
                            // kdisp_write_gfx_text(ALL_FONTS, sizeof(ALL_FONTS) / sizeof(GFXfont*), 28, 23, text);
                        // }
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

void update_performed(void) {
    last_update = timer_read32();
}


void request_disp_refresh(void) {
    g_refresh = ALL_AT_ONCE;
}


void set_displays(uint8_t contrast, bool idle) {
    if(idle) {
        //kdisp_idle(contrast);
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

void show_splash_screen(void) {
    clear_all_displays();
    display_message(2, 1, u"P", &FreeSansBold24pt7b);
}

void keyboard_post_init_user(void) {
    debug_enable=true;
    debug_matrix = false;
}

void keyboard_pre_init_user(void) {
    kdisp_hw_setup();
    kdisp_init(NUM_SHIFT_REGISTERS);
    kdisp_setup(true);

    select_all_displays();
    kdisp_scroll_vlines(47);
    kdisp_scroll_modehv(true, 3, 1);
    kdisp_scroll(false);

    set_displays(FULL_BRIGHT, false);

    // show_splash_screen();
}

void sync_and_refresh_displays(void) {
    l_layer.led_state = host_keyboard_led_state();
    l_layer.mods = get_mods();

    update_displays(g_refresh);
    g_refresh = DONE_ALL;
}

void housekeeping_task_user(void) {
    sync_and_refresh_displays();
}

void suspend_power_down_kb(void) {
    poly_suspend();
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

    update_performed();
    housekeeping_task_user();
    suspend_wakeup_init_user();
}

void poly_suspend(void) {
    l_state.flags &= ~((uint8_t)STATUS_DISP_ON) & ~((uint8_t)DISP_IDLE);
    l_state.contrast = DISP_OFF;
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    housekeeping_task_user();

    #ifndef VIA_ENABLE
        raw_hid_send(data, length);
    #endif
}

#ifndef VIA_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length) {
    via_custom_value_command_kb(data, length);
}
#endif