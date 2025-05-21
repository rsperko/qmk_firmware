/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

// Define a custom keycode for DPI toggling
enum custom_keycodes {
    DPI_TOGGLE = SAFE_RANGE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT( /* Base */
        KC_BTN1, DPI_TOGGLE, LCTL(KC_UP),
          KC_BTN2, LCTL(KC_DOWN)
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case DPI_TOGGLE:
            if (record->event.pressed) {
                // Toggle between the two DPI options (0 and 1)
                keyboard_config.dpi_config = keyboard_config.dpi_config == 0 ? 1 : 0;
                eeconfig_update_kb(keyboard_config.raw);
                pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
            }
            return false; // Skip all further processing of this key
        default:
            return true; // Process all other keycodes normally
    }
}
