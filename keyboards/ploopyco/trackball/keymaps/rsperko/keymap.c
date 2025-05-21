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

// Define a custom keycode for DPI toggling and drag scroll
enum custom_keycodes {
    DPI_TOGGLE = SAFE_RANGE,
    BTN3_SCROLL
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT( /* Base */
        KC_BTN1, DPI_TOGGLE, BTN3_SCROLL,
          KC_BTN2, LCTL(KC_DOWN)
    ),
};

// Track if drag scroll is active
static bool drag_scroll_active = false;
// Track when the key was pressed (for timing calculations)
static uint16_t btn3_timer;
// Track if button 3 is being held
static bool btn3_held = false;

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    // If drag scroll is active and any key is pressed, exit drag scroll mode
    if (drag_scroll_active && record->event.pressed) {
        toggle_drag_scroll();
        drag_scroll_active = false;
    }

    switch (keycode) {
        case DPI_TOGGLE:
            if (record->event.pressed) {
                // Toggle between the two DPI options (0 and 1)
                keyboard_config.dpi_config = keyboard_config.dpi_config == 0 ? 1 : 0;
                eeconfig_update_kb(keyboard_config.raw);
                pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
            }
            return false; // Skip all further processing of this key

        case BTN3_SCROLL:
            if (record->event.pressed) {
                // Key pressed - start timer and mark as held
                btn3_timer = timer_read();
                btn3_held = true;
            } else {
                // Key released
                btn3_held = false;

                // If released before TAPPING_TERM and drag scroll is not active,
                // treat it as a tap (send Ctrl+Up)
                if (!drag_scroll_active && timer_elapsed(btn3_timer) < TAPPING_TERM) {
                    tap_code16(LCTL(KC_UP));
                }

                btn3_timer = 0;
            }
            return false; // Skip all further processing of this key

        default:
            return true; // Process all other keycodes normally
    }
}

// This function runs frequently while keys are being processed
void matrix_scan_user(void) {
    // Check if BTN3_SCROLL is being held down
    if (btn3_held && !drag_scroll_active && timer_elapsed(btn3_timer) > TAPPING_TERM) {
        // If the key has been held longer than TAPPING_TERM, toggle drag scroll on
        toggle_drag_scroll();
        drag_scroll_active = true;
        // Don't set btn3_held to false so the user can release naturally
    }
}
