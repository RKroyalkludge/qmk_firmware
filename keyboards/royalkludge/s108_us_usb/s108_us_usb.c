// Copyright 2024 sdk66 (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "wls/wls.h"
#include "rgb_record/rgb_record.h"
#include "rgb_record/rgb_rgblight.h"

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#    include "usb_main.h"
#    include "lowpower.h"
#    include "rgb_matrix_blink.h"
#endif

enum layers {
    _BL = 0,
    _FL,
    _DEFA, 
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t DEB_flag: 1;
        uint8_t devs : 3;
        uint8_t record_channel : 2;
        uint8_t last_btdevs : 3;
        uint8_t sleep_timeout : 3;
        uint8_t rgb_ligth_mode : 4;

        uint8_t record_last_mode ;
        uint8_t record_cr_last_mode;
    };
} confinfo_t;
confinfo_t confinfo;

typedef struct {
    bool active;
    uint32_t timer;
    uint32_t interval;
    uint32_t times;
    uint8_t index;
    RGB rgb;
    void (*blink_cb)(uint8_t);
} hs_rgb_indicator_t;

void hs_reset_settings(void);
// void rgb_matrix_start_rec(void);
void matrix_init_user(void);
void hs_color_test(void);
uint8_t buff[] = {14,8,2,1,1,1,1,1,1,1,0}; 
static uint8_t hs_rl_mode = 0;

// #define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL))

uint32_t post_init_timer     = 0x00;
bool charging_state          = false;
bool bat_full_flag           = false;
static uint32_t ee_clr_timer = 0;
bool press_ee_clr_flag       = false;
bool test_white_light_flag   = false;
bool hs_reset_flag           = false;
static uint32_t hs_reset_time = 0;
uint8_t test_color_cnt       = 0;
static bool shifted          = false;
uint32_t hs_suspend_timer    = 0;
bool hs_frequency_test;
bool lower_sleep = false;
bool no_record_fg;
bool im_test_rate_flag;
uint32_t rec_time;
bool rec_filp;
uint16_t hs_deb              = DEBOUNCE;

bool hs_rgb_mode1 = false;
bool hs_rgb_mode2 = true;

void ind_indicators_hook(uint8_t index);
void nkr_indicators_hook(uint8_t index);
blink_rgb_t blink_rgbs[RGB_MATRIX_BLINK_COUNT] = {
    {.index = RGB_MATRIX_BLINK_INDEX_IND, .interval = 250, .times = 3, .color = {.r = 0x00, .g = 0x00, .b = 0x00}, .blink_cb = ind_indicators_hook},
    {.index = RGB_MATRIX_BLINK_INDEX_ALL, .interval = 250, .times = 3, .color = {.r = WIITE_B, .g = 0x00, .b = 0x00}, .blink_cb = nkr_indicators_hook},
};

void nkr_indicators_hook(uint8_t index) {
    if (index == RGB_MATRIX_BLINK_INDEX_ALL) {
        if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == WIITE_B) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == 0x00)) {
            if (rgb_matrix_blink_set_color(index, 0x00, 0x00, WIITE_B) != true) {
                dprintf("RGB_BLUE again\n");
            }
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
            dprintf("GREEM\r\n");
        } else if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == WIITE_B)) {
            if (rgb_matrix_blink_set_color(index, 0x00, WIITE_B, 0x00) != true) {
                dprintf("RGB_GREEN again\n");
            }
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
            dprintf("BLUE\r\n");
        }
    }
}

typedef enum {
    ind_state_none = 0,
    ind_state_exec,
    ind_state_blink3,
    ind_state_blinkon,
    ind_state_blinkoff,
} ind_status_t;

ind_status_t ind_status = ind_state_none;

void ind_indicators_hook(uint8_t index) {

    switch(ind_status) {
        case ind_state_none: {
            rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
            rgb_matrix_blink_set_interval_times(index, 50, 0xFF);
        } break;
        case ind_state_blink3: {
            rgb_matrix_blink_set_color(index, 0xFF, 0xFF, 0xFF);
            rgb_matrix_blink_set_interval_times(index, 250, 3);
            ind_status = ind_state_exec;
        } break;
        case ind_state_blinkon: {
            RGB rgb;
            HSV hsv = {.h = 0, .s = 0};
            hsv.v = rgb_matrix_get_val();
            rgb = hsv_to_rgb(hsv); 
            rgb_matrix_blink_set_color(index, rgb.r , rgb.g, rgb.b);
            rgb_matrix_blink_set_interval_times(index, 500, 1);
        } break;
        case ind_state_blinkoff: {
            ind_status = ind_state_none;
            ind_indicators_hook(index);
        } break;
        case ind_state_exec: {
            ind_status = ind_state_none;
            ind_indicators_hook(index);
        } break;
        default: break;
    }

    rgb_matrix_blink_set(index);
}

void ind_set_blink(ind_status_t status) {
    ind_status = status;
    rgb_matrix_blink_set_remain_time(RGB_MATRIX_BLINK_INDEX_IND, 0x00);
    ind_indicators_hook(RGB_MATRIX_BLINK_INDEX_IND);
}

bool rgb_matrix_blink_user(blink_rgb_t *blink_rgb) {

    switch (blink_rgb->index) {

        case RGB_MATRIX_BLINK_INDEX_IND: {
            
            switch(ind_status) {
                case ind_state_none: {
                    return false;
                } break;
                case ind_state_blinkon:
                case ind_state_blinkoff: {
                    if (!no_record_fg) return false;
                    
                    if (!blink_rgb->flip) {  
                        // rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                        rgb_matrix_set_color(HS_RGB_INDEX_CAPS, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                        rgb_matrix_set_color(HS_RGB_INDEX_NUM, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                        rgb_matrix_set_color(HS_RGB_INDEX_SCR, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                        rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                    } 
                } break;
                case ind_state_exec: {
                    
                    if (rgbrec_is_started()) {
                        return false;
                    }

                    
                    if (blink_rgb->flip) {
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_VAI, 0XFF, 0XFF, 0XFF);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_VAD, 0XFF, 0XFF, 0XFF);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_SPI, 0XFF, 0XFF, 0XFF);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_SPD, 0XFF, 0XFF, 0XFF);
                    } else {
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_VAI, 0x00, 0x00, 0x00);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_VAD, 0x00, 0x00, 0x00);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_SPI, 0x00, 0x00, 0x00);
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_SPD, 0x00, 0x00, 0x00);
                    }
                } break;
                default: break;
            }
            return false;
        } break;
#undef HS_RGB_BLINK_INDEX_REC
#ifdef HS_RGB_BLINK_INDEX_REC
        case HS_RGB_BLINK_INDEX_REC: {
        } break;
#endif
        default: { 
            
            if (test_white_light_flag) {
                return false;
            }
            
            if (rgbrec_is_started()) {
                return false;
            }
        } break;
    }

    return true;
}

void eeconfig_confinfo_update(void) {

    eeconfig_update_kb(confinfo.raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}

uint8_t hs_get_sleep_timeout(void){
    return confinfo.sleep_timeout;
}

void hs_set_sleep_timeout(uint8_t sleep_timeout){
    confinfo.sleep_timeout = sleep_timeout;
}


void eeconfig_confinfo_default(void) {

    confinfo.record_channel    = 0;
    confinfo.record_last_mode  = 0x00;
    confinfo.record_cr_last_mode = 0x00;
    confinfo.last_btdevs       = 1;
    confinfo.sleep_timeout     = 7;
    confinfo.rgb_ligth_mode    = 0;
    confinfo.DEB_flag          = true;

    confinfo.devs = wireless_get_current_devs();
    eeconfig_init_user_datablock();
    eeconfig_confinfo_update();
    all_color_reset();
    rgb_matrix_mode(13);
#if defined(NKRO_ENABLE) && defined(FORCE_NKRO)
    keymap_config.nkro = 1;
    eeconfig_update_keymap(keymap_config.raw);
#endif

#ifdef RGBLIGHT_ENABLE
    rgblight_mode(buff[0]);
#endif

}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
    }
}

void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif
    // keymap_config.no_gui = 0;
    eeconfig_confinfo_init();

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef MM_BT_DEF_PIN
    setPinInputHigh(MM_BT_DEF_PIN);
#endif

#ifdef MM_2G4_DEF_PIN
    setPinInputHigh(MM_2G4_DEF_PIN);
#endif

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

#ifdef HS_BAT_CABLE_PIN
    setPinInput(HS_BAT_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    setPinInputHigh(BAT_FULL_PIN);
#endif

#ifdef HS_ENCODERS_PAD_A
    setPinInputHigh(HS_ENCODERS_PAD_A);
#endif

#ifdef HS_ENCODERS_PAD_B
    setPinInputHigh(HS_ENCODERS_PAD_B);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_set_pin_output(HS_LED_BOOSTING_PIN);
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef WIRELESS_ENABLE
    wireless_init();
#if !(defined(MM_BT_DEF_PIN) && defined(MM_2G4_DEF_PIN))
    wireless_devs_change(!DEVS_USB, confinfo.devs, false);
#endif
    post_init_timer = timer_read32();
#endif

#ifdef RGB_MATRIX_BLINK_INDEX_BAT
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_BAT, 200, 0xFF);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_BAT);
#endif

#ifdef RGB_MATRIX_BLINK_INDEX_IND
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_IND, 200, 0xFF);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_IND);
#endif

#ifdef HS_RGB_BLINK_INDEX_REC
    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_REC, 200, 0xFF);
    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_REC);
#endif

    // matrix_init_user();
    keyboard_post_init_user();
    hs_rl_mode = confinfo.rgb_ligth_mode;
    rgbrec_init(confinfo.record_channel);

    if(confinfo.DEB_flag){
        hs_deb = 10;
    }else{
        hs_deb = 1; 
    }
}

#ifdef WIRELESS_ENABLE

void usb_power_connect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
#    endif
}

void usb_power_disconnect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_high(USB_POWER_EN_PIN);
#    endif
}

void suspend_power_down_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_low(LED_POWER_EN_PIN);
#    endif

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#    endif

    wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), false);
    suspend_wakeup_init_user();
    hs_rgb_blink_set_timer(timer_read32());
}

bool lpwr_is_allow_timeout_hook(void) {

    // if (wireless_get_current_devs() == DEVS_USB) {
    //     return false;
    // }

    return true;
}

void wireless_post_task(void) {

    // auto switching devs
    if (post_init_timer && timer_elapsed32(post_init_timer) >= 100) {

        md_send_devctrl(MD_SND_CMD_DEVCTRL_FW_VERSION);   // get the module fw version.
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_BT_EN);  // timeout 30min to sleep in bt mode, enable
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_2G4_EN); // timeout 30min to sleep in 2.4g mode, enable
        wireless_devs_change(!confinfo.devs, confinfo.devs, false);
        post_init_timer = 0x00;
    }

    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);
}

bool state = false;
uint32_t wls_process_long_press(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint16_t *)cb_arg);

    switch (keycode) {
        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }

        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case HS_FREQ: {
            hs_frequency_test = true;
        } break;
        default:
            break;
    }

    return 0;
}

bool process_record_wls(uint16_t keycode, keyrecord_t *record) {
    static uint16_t keycode_shadow                     = 0x00;
    static deferred_token wls_process_long_press_token = INVALID_DEFERRED_TOKEN;

    keycode_shadow = keycode;

#    ifndef WLS_KEYCODE_PAIR_TIME
#        define WLS_KEYCODE_PAIR_TIME 3000
#    endif

#    define WLS_KEYCODE_EXEC(wls_dev)                                                                                          \
        do {                                                                                                                   \
            if (record->event.pressed) {                                                                                       \
                if (wireless_get_current_devs() != wls_dev)                                                                    \
                    wireless_devs_change(wireless_get_current_devs(), wls_dev, false);                                         \
                if (wls_process_long_press_token == INVALID_DEFERRED_TOKEN) {                                                  \
                    wls_process_long_press_token = defer_exec(WLS_KEYCODE_PAIR_TIME, wls_process_long_press, &keycode_shadow); \
                }                                                                                                              \
            } else {                                                                                                           \
                cancel_deferred_exec(wls_process_long_press_token);                                                            \
                wls_process_long_press_token = INVALID_DEFERRED_TOKEN;                                                         \
            }                                                                                                                  \
        } while (false)

    switch (keycode) {
        case HS_FREQ:{
            WLS_KEYCODE_EXEC(wireless_get_current_devs());
        }break;

        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT1);
                
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT2);
     
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT3);
              
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_2G4);
           
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0xFF);
        } break;

        case KC_USB: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_USB);
                state = false;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        default:
            return true;
    }

    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (*md_getp_state() == MD_STATE_CONNECTED) {
        hs_rgb_blink_set_timer(timer_read32());
    }

    switch (keycode) {
        case MO(_FL): {
            if (!record->event.pressed && rgbrec_is_started()) {
                if (no_record_fg == true) {
                    no_record_fg = false;
                    rgbrec_register_record(keycode, record);
                }
                no_record_fg = true;
            }
            break;
        }
        case RP_END:
        case RP_P0:
        case RP_P1:
        case RP_P2:
        case RGB_MOD:
        case CR_MODE:
        case B_MODE:
        case CB_MODE:
            break;
        default: {
            if (rgbrec_is_started()) {
                if (!IS_QK_MOMENTARY(keycode) && record->event.pressed) {
                    rgbrec_register_record(keycode, record);

                    return false;
                }
            }
        } break;
    }

    if (rgbrec_is_started() && (!(keycode == RP_P0 || keycode == RP_P1 || keycode == RP_P2 || keycode == RP_END 
    || keycode == RGB_MOD || keycode == CB_MODE || keycode == B_MODE || keycode == CR_MODE || keycode == MO(_FL)))) {

        return false;
    }

    return true;
}

void im_rgblight_increase(void) {
    HSV rgb;
    uint8_t moude;
    moude = rgblight_get_mode();
    
    if (moude == 1) {
        rgb = rgblight_get_hsv();
        if (rgb.h == 0 && rgb.s != 0) hs_rl_mode = 3;
        else hs_rl_mode = 9;
        switch (rgb.h){
            case 40:hs_rl_mode = 4;break;
            case 80:hs_rl_mode = 5;break;
            case 120:hs_rl_mode = 6;break;
            case 160:hs_rl_mode = 7;break;
            case 200:hs_rl_mode = 8;break;
            default :break;
        }
    }
    hs_rl_mode++;
    if(hs_rl_mode == 11) hs_rl_mode=0;
    if (hs_rl_mode == 10) {rgb = rgblight_get_hsv();rgblight_sethsv(0,255,rgb.v);rgblight_disable();}
    else 
    {
        rgblight_enable();
        
        rgblight_mode(buff[hs_rl_mode]);
    }
    rgb = rgblight_get_hsv();
    switch (hs_rl_mode){
        case 3: rgblight_sethsv(0,255,rgb.v);break;
        case 4: rgblight_sethsv(40,255,rgb.v);break;
        case 5: rgblight_sethsv(80,255,rgb.v);break;
        case 6: rgblight_sethsv(120,255,rgb.v);break;
        case 7: rgblight_sethsv(160,255,rgb.v);break;
        case 8: rgblight_sethsv(200,255,rgb.v);break;
        case 9: rgblight_sethsv(0,0,rgb.v);break;
        case 0: rgblight_set_speed(255);break;
        default:rgblight_set_speed(200);break;
    } 
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (process_record_user(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif

    if (hs_frequency_test){
        if (!(keycode >= KC_1 && keycode <= KC_4))
            return false;
    } 

    if (test_white_light_flag && record->event.pressed && keycode != KC_SPC) {
        test_white_light_flag = false;
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    if ((confinfo.devs != DEVS_USB && *md_getp_state() == MD_STATE_DISCONNECTED)){
        if(IS_BASIC_KEYCODE(keycode) || IS_MODIFIER_KEYCODE(keycode)){
            return false;
        }
    }

    switch (keycode) {
        case DEB_TOG:{ 
            if (record->event.pressed) {
                confinfo.DEB_flag = !confinfo.DEB_flag;
                if(confinfo.DEB_flag){
                    hs_deb = 10; 
                }else{
                    hs_deb = 1;
                }
                eeconfig_confinfo_update();
            }
            return false;
        }break;
        case QK_GESC: {
            static bool grave_esc_was_shifted = false;
            if (record->event.pressed) {
                grave_esc_was_shifted = shifted;
                register_code16(shifted ? KC_GRAVE : KC_ESCAPE);
            } else {
                unregister_code16(grave_esc_was_shifted ? KC_GRAVE : KC_ESCAPE);
            }
            return false;   
        } break;
        case KC_LSFT:
        case KC_RSFT: {
            shifted = record->event.pressed;
            return true;
        } break;
        case NK_TOGG: {
            if (record->event.pressed) {
                rgb_matrix_blink_set_color(RGB_MATRIX_BLINK_INDEX_ALL, WIITE_B, 0x00, 0x00);
                rgb_matrix_blink_set_times(RGB_MATRIX_BLINK_INDEX_ALL, 1);
                rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
            }
        } break;
        case EX_RATE: {
            if (record->event.pressed) {
                im_test_rate_flag = true;
            } else {
                im_test_rate_flag = false;
            }
            return false;
        } break;
        case KC_CUT: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_X);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_X);
            }
            return false;
        } break;
        case KC_COPY: {
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_C);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_C);
            }
            return false;
        } break;
        case KC_PASTE: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_V);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_V);
            }
            return false;
        } break;
        case KC_FIND: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_F);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_F);
            }
            return false;
        } break;
        case BT_TEST: {
            if(record->event.pressed) {
                md_send_devctrl(0x62);
            }
        } break;
        case KC_1: { 
            if(hs_frequency_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(0, 5, 0x01);
                
                } 
                return false;
            }
        } break;
        case KC_2: { 
            if(hs_frequency_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(19, 5, 0x01);
                   
                } 
                return false;
            }
        } break;
        case KC_3: { 
            if(hs_frequency_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(39, 5, 0x01);
                   
                } 
                return false;
            }
        } break;
        case KC_4: { 
            if(hs_frequency_test){
                if (record->event.pressed) {
                    md_rf_send_stop();
                } 
                return false;
            } 
        } break;
        case QK_BOOT: {
            if (record->event.pressed) {
                rgb_matrix_disable_noeeprom();
                gpio_write_pin_low(LED_POWER_EN_PIN);
                hs_reset_flag = true;
                hs_reset_time = timer_read32();
                dprintf("into boot!!!\r\n");
                // eeconfig_disable();
                // bootloader_jump();
                return false;
            }
        } break;
        case KC_TEST: {
            if (record->event.pressed) {
                test_white_light_flag = true;
            }
            return false;
        } break;
        case KC_SPC: {
            if (test_white_light_flag) {
                if (record->event.pressed) {
                    if ((++test_color_cnt) > 3) {
                        test_color_cnt = 0;
                    }
                }
                return false;
            }
            return true;
        } break;
        case HS_RWCK: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LALT);
                register_code(KC_TAB);
            } else {
                unregister_code(KC_LALT);
                unregister_code(KC_TAB);
            }
            return true;
        } break;
        case HS_YYIN: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_H);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_H);
            }
            return true;
        } break;
        case HS_DESK: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_D);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_D);
            }
            return true;
        } break;
        case RGB_SPI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_speed() >= (55 + RGB_MATRIX_SPD_STEP * 4)) {
                    ind_set_blink(ind_state_blink3);
                }
            }
        } break;
        case RGB_SPD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_speed() <= (55 + RGB_MATRIX_SPD_STEP*1)) {
                    ind_set_blink(ind_state_blink3);
                    rgb_matrix_set_speed(55 + (RGB_MATRIX_SPD_STEP*0));
                    return false;
                }
            }
        } break;
        case RGB_VAI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP)) {
                    ind_set_blink(ind_state_blink3);
                } 
            }
        } break;
        case RGB_VAD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                    ind_set_blink(ind_state_blink3);
                    if (!keymap_config.no_gui && !host_keyboard_led_state().caps_lock) {
                        if (rgb_matrix_get_val() != 0) {
                            for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                                rgb_matrix_set_color(i, 0, 0, 0);
                            }
                        }
                    } 
                } 
            }
        } break;
        case HS_BATQ: {
            extern bool rk_bat_req_flag;
            rk_bat_req_flag = (confinfo.devs != DEVS_USB) && record->event.pressed;
            return false;
        } break;
        case EE_CLR: {
            if (record->event.pressed) {
                ee_clr_timer = timer_read32();
                press_ee_clr_flag = true;
            } else {
                ee_clr_timer = 0;
            }

            return false;
        } break;
        // case TO(_BL): {
        //     if (record->event.pressed) {
        //         if (keymap_is_mac_system()) {
        //             set_single_persistent_default_layer(_BL);
        //             layer_move(0);
        //         }
        //     }

        //     return false;
        // } break;
        // case TO(_MBL): {
        //     if (record->event.pressed) {
        //         if (!keymap_is_mac_system()) {
        //             set_single_persistent_default_layer(_MBL);
        //             layer_move(0);
        //         }
        //     }

        //     return false;
        // } break;       
        case RP_P0: {
            if (record->event.pressed) {
                hs_rgb_mode1 = true;
                hs_rgb_mode2 = true;
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 0;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
            }

            return false;
        } break;
        case RP_P1: {
            if (record->event.pressed) {
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 1;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
            }

            return false;
        } break;
        case RP_P2: {
            if (record->event.pressed) {
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 2;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
            }

            return false;
        } break;
        case RP_END: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {

                    return false;
                }
                if (!rgbrec_is_started()) {
                    rgbrec_start(confinfo.record_channel);
                    no_record_fg = false;
                    rec_time     = timer_read32();
                    rgbrec_set_close_all(HSV_BLACK);
                    ind_set_blink(ind_state_blinkon);
                } else {
                    rec_time = 0;
                    rgbrec_end(confinfo.record_channel,true);
                    ind_set_blink(ind_state_none);
                }
                eeconfig_confinfo_update();
            }

            return false;
        } break;
        case RL_MOD: {
            if (record->event.pressed) {
                im_rgblight_increase();
                confinfo.rgb_ligth_mode = hs_rl_mode;
                eeconfig_confinfo_update();
            }

            return false;
        } break;
        case CR_MODE: {  
            if (record->event.pressed) {
                hs_rgb_mode1 = true;
                ind_set_blink(ind_state_none);
                rgbrec_read_current_channel(confinfo.record_channel); 
                rgbrec_end(confinfo.record_channel,false); 
                no_record_fg            = false;

                if (hs_rgb_mode2) {
                    hs_rgb_mode2 = false;
                    if (confinfo.record_cr_last_mode == 0) {
                        confinfo.record_cr_last_mode = 0;
                    } else {
                        confinfo.record_cr_last_mode = confinfo.record_cr_last_mode -1;
                        if (confinfo.record_cr_last_mode < 0) confinfo.record_cr_last_mode = 0;
                    }
                    
                    record_interactive_mod(&(confinfo.record_cr_last_mode));
                } else {
                    record_interactive_mod(&(confinfo.record_cr_last_mode));
                }
            }
            eeconfig_confinfo_update();
            return false;
        } break;
        case B_MODE: { 
            if (record->event.pressed) {
                static uint8_t  B_MODE = 0;
                hs_rgb_mode1 = true;
                hs_rgb_mode2 = true;
                ind_set_blink(ind_state_none);
                rgbrec_read_current_channel(confinfo.record_channel); 
                rgbrec_end(confinfo.record_channel,false); 
                no_record_fg            = false;

                record_breathing_mod(&(B_MODE));
            }
            eeconfig_confinfo_update();
            return false;
        } break;
        case CB_MODE: {   
            if (record->event.pressed) {
                static uint8_t  CB_MODE = 0;
                hs_rgb_mode1 = true;
                hs_rgb_mode2 = true;
                ind_set_blink(ind_state_none);
                rgbrec_read_current_channel(confinfo.record_channel); 
                rgbrec_end(confinfo.record_channel,false); 
                no_record_fg            = false;

                record_cr_breathing_mod(&(CB_MODE));
            }
            eeconfig_confinfo_update();
            return false;
        } break;
        case RGB_MOD: { 
            if (record->event.pressed) {
                hs_rgb_mode2 = true;
                ind_set_blink(ind_state_none);
                rgbrec_read_current_channel(confinfo.record_channel); 
                rgbrec_end(confinfo.record_channel,false); 
                no_record_fg            = false;

                if (hs_rgb_mode1) {
                    hs_rgb_mode1 = false;
                    if (confinfo.record_last_mode == 0) {
                        confinfo.record_last_mode = 0;
                    } else {
                        confinfo.record_last_mode = confinfo.record_last_mode - 1;
                        if (confinfo.record_last_mode < 0) confinfo.record_last_mode = 0;
                    }
                    
                    record_rgbmatrix_increase(&(confinfo.record_last_mode));
                } else {
                    record_rgbmatrix_increase(&(confinfo.record_last_mode));
                }
                
                eeconfig_confinfo_update();
            }

            return false;
        } break;
        case RGB_HUD: {
            if (record->event.pressed) {
                record_color_hsv(false);
            }

            return false;
        } break;
        case RGB_HUI: {
            if (record->event.pressed) {
                record_color_hsv(true);
            }

            return false;
        } break;
        default:
            break;
    }

    return true;
}

#ifdef RGB_MATRIX_ENABLE

#    ifdef WIRELESS_ENABLE
bool wls_rgb_indicator_reset        = false;
uint32_t wls_rgb_indicator_timer    = 0x00;
uint32_t wls_rgb_indicator_interval = 0;
uint32_t wls_rgb_indicator_times    = 0;
uint32_t wls_rgb_indicator_index    = 0;
RGB wls_rgb_indicator_rgb           = {0};

void rgb_matrix_wls_indicator_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times) {

    wls_rgb_indicator_timer = timer_read32();

    wls_rgb_indicator_index    = index;
    wls_rgb_indicator_interval = interval;
    wls_rgb_indicator_times    = times * 2;
    wls_rgb_indicator_rgb      = rgb;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {

    wls_rgb_indicator_reset = reset;

    if (confinfo.devs != wireless_get_current_devs()) {
        confinfo.devs = wireless_get_current_devs();
        if (confinfo.devs > 0 && confinfo.devs < 4) confinfo.last_btdevs = confinfo.devs;
        eeconfig_confinfo_update();
    }

    switch (new_devs) {
        case DEVS_BT1: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT1, (RGB){HS_LBACK_COLOR_BT1}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT1, (RGB){HS_PAIR_COLOR_BT1}, 500, 1);
            }
        } break;
        case DEVS_BT2: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT2, (RGB){HS_LBACK_COLOR_BT2}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT2, (RGB){HS_PAIR_COLOR_BT2}, 500, 1);
            }
        } break;
        case DEVS_BT3: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT3, (RGB){HS_LBACK_COLOR_BT3}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT3, (RGB){HS_PAIR_COLOR_BT3}, 500, 1);
            }
        } break;
        case DEVS_2G4: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_2G4, (RGB){HS_LBACK_COLOR_2G4}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_2G4, (RGB){HS_PAIR_COLOR_2G4}, 500, 1);
            }
        } break;
        case DEVS_USB: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_LBACK_COLOR_USB}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_PAIR_COLOR_USB}, 500, 1);
            }
        } break;
        default:
            break;
    }
}

bool rgb_matrix_wls_indicator_cb(void) {

    if (*md_getp_state() != MD_STATE_CONNECTED) {
        if (!(wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_ACTIVE)) wireless_devs_change_kb(wireless_get_current_devs(), wireless_get_current_devs(), wls_rgb_indicator_reset);
        return true;
    }

    // refresh led
    led_wakeup();

    return false;
}

void rgb_matrix_wls_indicator(void) {

    if (wls_rgb_indicator_timer) {

        if (timer_elapsed32(wls_rgb_indicator_timer) >= wls_rgb_indicator_interval) {
            wls_rgb_indicator_timer = timer_read32();

            if (wls_rgb_indicator_times) {
                wls_rgb_indicator_times--;
            }

            if (wls_rgb_indicator_times <= 0) {
                wls_rgb_indicator_timer = 0x00;
                if (rgb_matrix_wls_indicator_cb() != true) {
                    return;
                }
            }
        }

        if (wls_rgb_indicator_times % 2) {
            rgb_matrix_set_color(wls_rgb_indicator_index, wls_rgb_indicator_rgb.g, wls_rgb_indicator_rgb.r, wls_rgb_indicator_rgb.b);
        } else {
            rgb_matrix_set_color(wls_rgb_indicator_index, 0x00, 0x00, 0x00);
        }
    }
}

#    endif

#endif

bool hs_reset_settings_user(void) {
    rgb_matrix_blink_set_color(RGB_MATRIX_BLINK_INDEX_ALL, WIITE_B, WIITE_B, WIITE_B);
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 3);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);

    return true;
}

void rgb_matrix_start_rec(void) {

    if (rgbrec_is_started()) {
        if (!rec_time || timer_elapsed32(rec_time) > 250) {
            rec_time = timer_read32();
            rec_filp = !rec_filp;
        }
        if (rec_filp) {
            #ifdef HS_RGB_BLINK_INDEX_REC
                rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, WIITE_B, WIITE_B, WIITE_B);
            #endif
        } else {
            #ifdef HS_RGB_BLINK_INDEX_REC
                rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, 0x00, 0x00, 0x00);
            #endif
        }
    } else {
        rec_time = 0;
        rec_filp = false;
    }
}

void hs_blink_rgb_matrix(uint8_t r, uint8_t g, uint8_t b) {
    int i;
    for(i = 0;i < RGB_MATRIX_LED_COUNT;i++){
        rgb_matrix_set_color(i, r,g,b);
    }
}

void housekeeping_task_user(void) { // loop

#ifdef HS_BAT_CABLE_PIN
    charging_state = readPin(HS_BAT_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    bat_full_flag = readPin(BAT_FULL_PIN);
#endif
    if (hs_reset_flag) {
        if (timer_elapsed32(hs_reset_time) > 100) {
            hs_reset_time = timer_read32();
            eeconfig_disable();
            bootloader_jump();
        }
    }
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {

#ifdef RGBLIGHT_ENABLE
    if (rgb_matrix_indicators_advanced_rgblight(led_min, led_max) != true) {

        return false;
    }

    extern LED_TYPE rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];
    const uint8_t mirror_map[][2] = {
        {104, 105}, {104, 118}, {104, 119}, 
    };
    const uint8_t mirror_map1[][2] = {
        {106, 107}, {106, 116}, {106, 117}, 
    };
    const uint8_t mirror_map2[][2] = {
        {108, 109}, {108, 114}, {108, 115}, 
    };
    const uint8_t mirror_map3[][2] = {
        {110, 111}, {110, 112}, {110, 113}, 
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map); i++) {
        uint8_t src = mirror_map[i][0];
        uint8_t dst = mirror_map[i][1];
        uint8_t r = rgb_matrix_ws2812_array[src].r;
        uint8_t g = rgb_matrix_ws2812_array[src].g;
        uint8_t b = rgb_matrix_ws2812_array[src].b;
        rgb_matrix_set_color(dst, r, g, b);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map1); i++) {
        uint8_t src = mirror_map1[i][0];
        uint8_t dst = mirror_map1[i][1];
        uint8_t r = rgb_matrix_ws2812_array[src].r;
        uint8_t g = rgb_matrix_ws2812_array[src].g;
        uint8_t b = rgb_matrix_ws2812_array[src].b;
        rgb_matrix_set_color(dst, r, g, b);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map2); i++) {
        uint8_t src = mirror_map2[i][0];
        uint8_t dst = mirror_map2[i][1];
        uint8_t r = rgb_matrix_ws2812_array[src].r;
        uint8_t g = rgb_matrix_ws2812_array[src].g;
        uint8_t b = rgb_matrix_ws2812_array[src].b;
        rgb_matrix_set_color(dst, r, g, b);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map3); i++) {
        uint8_t src = mirror_map3[i][0];
        uint8_t dst = mirror_map3[i][1];
        uint8_t r = rgb_matrix_ws2812_array[src].r;
        uint8_t g = rgb_matrix_ws2812_array[src].g;
        uint8_t b = rgb_matrix_ws2812_array[src].b;
        rgb_matrix_set_color(dst, r, g, b);
    }
#endif

    if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
        hs_reset_settings();
        ee_clr_timer = 0;
    }

    if (!rgbrec_is_started()) {
        if (host_keyboard_led_state().caps_lock){
            rgb_matrix_set_color(HS_RGB_INDEX_CAPS,0xff,0xff,0xff);
        }

        if (host_keyboard_led_state().num_lock){
            rgb_matrix_set_color(HS_RGB_INDEX_NUM,0xff,0xff,0xff);
        } 

        if (host_keyboard_led_state().scroll_lock){
            rgb_matrix_set_color(HS_RGB_INDEX_SCR,0xff,0xff,0xff);
        }

        if (keymap_config.no_gui){
            rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK,0xff,0xff,0xff);
        } 
    }

#ifdef WIRELESS_ENABLE 
    // rgb_matrix_wls_indicator();
#endif

    // rgb_matrix_start_rec();

    // query();
    rgb_matrix_blink_task(led_min, led_max);
    hs_color_test();
    return true;
}

void hs_reset_settings(void) {
    eeconfig_init();
    
#ifdef RGBLIGHT_ENABLE
    extern void rgblight_init(void);
    is_rgblight_initialized = false;
    rgblight_init();
    eeconfig_update_rgblight_default();
    rgblight_enable();
#endif

    eeconfig_update_rgb_matrix_default();
    keymap_config.raw = eeconfig_read_keymap();

    if (hs_reset_settings_user() != true) {
        return;
    }

    keyboard_post_init_kb();
}


void lpwr_wakeup_hook(void) { // awake
    if (wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_STOP) {
        usb_power_connect();
        restart_usb_driver(&USBD1);
        wireless_devs_change(!DEVS_USB, DEVS_USB, false);
    }
    
    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);
#ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef HS_ENCODERS_PAD_A
    setPinInputHigh(HS_ENCODERS_PAD_A);
#endif

#ifdef HS_ENCODERS_PAD_B
    setPinInputHigh(HS_ENCODERS_PAD_B);
#endif

}

void matrix_init_user(void) {
    
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (matrix_is_on(row, col)) {

                if (row == 0 && col == 0) {
                    hs_frequency_test = true; 
                }
            }
        }
    }
}

void hs_color_test(void) {
    if (test_white_light_flag) {
         RGB rgb_test_open;
        switch (test_color_cnt) {
            case 3: rgb_test_open = hsv_to_rgb((HSV){.h = 0, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 1: rgb_test_open = hsv_to_rgb((HSV){.h = 85, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 2: rgb_test_open = hsv_to_rgb((HSV){.h = 170, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 0: rgb_test_open = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            default:
                break;
        }
        rgb_matrix_set_color_all(rgb_test_open.r, rgb_test_open.g, rgb_test_open.b);
    }
}
