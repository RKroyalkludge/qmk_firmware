// Copyright 2024 sdk66 (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN                    B1 // USB ENABLE pin
#define LED_POWER_EN_PIN                    A8 // LED ENABLE pin
#define HS_BAT_CABLE_PIN                    A7 // USB insertion detection pin
// #define HS_LED_BOOSTING_PIN                 A9 // LED BOOSTING

// #define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

// #define MM_BT_DEF_PIN                       C15
// #define MM_2G4_DEF_PIN                      C14
// #define MD_BT1_NAME                         "L1800-BT1"
// #define MD_BT2_NAME                         "L1800-BT2"
// #define MD_BT3_NAME                         "L1800-BT3"
// #define MD_DONGLE_PRODUCT                   "L1800-2.4G Dongle"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              7
#define HS_RGB_BLINK_INDEX_BT2              6
#define HS_RGB_BLINK_INDEX_BT3              5
#define HS_RGB_BLINK_INDEX_2G4              3
#define HS_RGB_BLINK_INDEX_USB              4

#define WIITE_B                             0X15

#define HS_LBACK_COLOR_BT1                  RGB_BLUE
#define HS_LBACK_COLOR_BT2                  RGB_BLUE
#define HS_LBACK_COLOR_BT3                  RGB_BLUE
#define HS_LBACK_COLOR_2G4                  RGB_GREEN
#define HS_LBACK_COLOR_USB                  RGB_WHITE

#define HS_PAIR_COLOR_BT1                   RGB_BLUE
#define HS_PAIR_COLOR_BT2                   RGB_BLUE
#define HS_PAIR_COLOR_BT3                   RGB_BLUE
#define HS_PAIR_COLOR_2G4                   RGB_GREEN
#define HS_PAIR_COLOR_USB                   RGB_WHITE

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {34, 35, 36, 37, 38, 39, 40, 41, 42, 43}

#define HS_DEBOUNCE
#define USB_POWER_DOWN_DELAY 7000
// #define GPIO_UART_ENABLE

/* Status Indicator Lamp */
#define RGB_MATRIX_BLINK_INDEX_ALL          0xFF 
#define RGB_MATRIX_BLINK_INDEX_BAT          250
#define RGB_MATRIX_BLINK_INDEX_IND          251
#define RGB_MATRIX_BLINK_COUNT              2

#define HS_RGB_INDEX_CAPS                   2
#define HS_RGB_INDEX_NUM                    0
#define HS_RGB_INDEX_SCR                    1
#define HS_RGB_INDEX_WIN_LOCK               3
#define HS_RGB_BLINK_INDEX_REC              100
#define HS_RGB_BLINK_INDEX_VAI              28
#define HS_RGB_BLINK_INDEX_VAD              7
#define HS_RGB_BLINK_INDEX_SPI              6
#define HS_RGB_BLINK_INDEX_SPD              8

/*  ENCODER */
#define HS_ENCODERS_PAD_A B6
#define HS_ENCODERS_PAD_B B7
// #define ENCODER_RESOLUTION  2

/*  IIC INIT */
#define I2C_DRIVER        I2CD1
#define I2C1_OPMODE       OPMODE_I2C
#define I2C1_CLOCK_SPEED  100000

/* UART */
#define SERIAL_DRIVER                       SD3
#define SD1_TX_PIN                          C10
#define SD1_RX_PIN                          C11

/* SPI */
#define SPI_DRIVER                          SPIDQ
#define SPI_SCK_PIN                         B3
#define SPI_MOSI_PIN                        B5
#define SPI_MISO_PIN                        B4

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE          (WEAR_LEVELING_BACKING_SIZE / 2)
#define VIA_EEPROM_ALLOW_RESET 

/* RGB Matrix */
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES
#define RGB_TRIGGER_ON_KEYDOWN

/* RGB Light */
#define RGBLED_NUM 10
#define RGBLIGHT_CUSTOM_DRIVER
#define RGBLIGHT_DISABLE_KEYCODES
#define RGBLIGHT_LIMIT_VAL      255
#define BREATHING_VAL           255
#define RAINBOW_MOOD            255
#define LED_TYPE                RGB
#define RGBLIGHT_LED_MAP {4, 3, 2, 1, 0, 5, 6, 7, 8, 9}

/* WS2812 */
#define WS2812_SPI_DRIVER       SPIDM2
#define WS2812_SPI_DIVISOR      32

#define RGB_MATRIX_LED_COUNT (130)

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 17)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)