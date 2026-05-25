// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "rgb_record/rgb_record.h"

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _DEFA, 
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BL] = LAYOUT( /* Base */
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                       
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_BSLS,  KC_ENTER,          
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,            
        KC_LCTL,  KC_LGUI,  KC_LALT,                      KC_SPC,                                 KC_RALT,  KC_APP,             KC_RCTL,  MO(_FL)),

    [_FL] = LAYOUT( /* Base */
        KC_GRV,   KC_F1,    KC_F2,    KC_F3,    KC_F4,     KC_F5,   KC_F6,    KC_F7,   KC_F8,     KC_F9,     KC_F10,   KC_F11,   KC_F12,  KC_DEL,   
        _______,  KC_BT1,   KC_BT2,   KC_BT3,   _______,  _______,  KC_PSCR,  KC_SCRL, KC_PAUSE,  MO(_DEFA), KC_2G4,   RGB_VAD,  RGB_VAI, 
        _______,  KC_NO,    TO(_MBL), _______,  _______,  _______,  KC_INS,   KC_HOME, KC_PGUP,   MO(_DEFA), RGB_SPD,  RGB_SPI,  RGB_MOD, HS_BATQ,
        _______,  _______,  _______,  _______,  _______,  _______,  NK_TOGG,  KC_DEL,  KC_END,    KC_PGDN,   RGB_HUI,  _______,           MM_DIR,
        MM_FC,    GU_TOGG,  _______,                       EE_CLR,                                KC_RALT,   _______,            _______, _______),

    [_MBL] = LAYOUT( /* Base */
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_BSLS,  KC_ENTER,          
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,            
        KC_LCTL,  KC_LALT,  KC_LGUI,                      KC_SPC,                                 KC_RGUI,  KC_APP,             KC_RCTL,  MO(_MFL)),

    [_MFL] = LAYOUT( /* Base */
        KC_GRV,   KC_F1,    KC_F2,    KC_F3,    KC_F4,     KC_F5,   KC_F6,   KC_F7,    KC_F8,     KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_DEL, 
        _______,  KC_BT1,   KC_BT2,   KC_BT3,   _______,  _______,  KC_PSCR, KC_SCRL,  KC_PAUSE,  MO(_DEFA),KC_2G4,   RGB_VAD,  RGB_VAI,  
        _______,  TO(_BL),  KC_NO,    _______,  _______,  _______,  KC_INS,  KC_HOME,  KC_PGUP,   MO(_DEFA),RGB_SPD,  RGB_SPI,  RGB_MOD,  HS_BATQ,
        _______,  _______,  _______,  _______,  _______,  _______,  NK_TOGG, KC_DEL,   KC_END,    KC_PGDN,  RGB_HUI,  _______,            MM_DIR,
        MM_FC,    _______,  _______,                       EE_CLR,                                KC_RGUI,  _______,            _______,  _______),
    [_DEFA] = LAYOUT( /* Base */ 
        QK_BOOT,  _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, 
        _______,  _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_TEST,  _______,  _______,  _______,  
        _______,  _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_TEST,  _______,  _______,  _______,  _______,  
        _______,  _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,           
        _______,  _______,   _______,                      _______,                                _______,  _______,            _______,  _______)

};

const uint16_t PROGMEM rgbrec_default_effects[RGBREC_CHANNEL_NUM][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,        
       ________, _______,  ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,           HS_GREEN,
       ________, ________, ________,                     ________,                               ________, ________,           HS_GREEN, HS_GREEN),

    [1] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,        
       ________, _______,  ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,           HS_GREEN,
       ________, ________, ________,                     ________,                               ________, ________,            HS_GREEN,HS_GREEN),

    [2] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, 
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,        
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,           HS_GREEN,
       ________, ________, ________,                     ________,                               ________, ________,           HS_GREEN, HS_GREEN)
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [1] = {ENCODER_CCW_CW(_______, _______)},
    [2] = {ENCODER_CCW_CW(_______, _______)},
    [3] = {ENCODER_CCW_CW(_______, _______)},
    [4] = {ENCODER_CCW_CW(_______, _______)}
};
#endif
// clang-format on
