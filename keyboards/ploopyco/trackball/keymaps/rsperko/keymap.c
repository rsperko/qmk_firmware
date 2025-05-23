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
#include "pointing_device.h" // For get_mouse_report() and report_mouse_t

// Provided by ploopyco.c/ploopyco.h
extern bool is_drag_scroll;
extern void        toggle_drag_scroll(void); // Ensure we can call Ploopy's function

// For Utility Layer scroll wheel Back/Forward
static int8_t utility_scroll_accumulator = 0;

// Define custom keycodes
enum custom_keycodes {
    BTN3_SCROLL = SAFE_RANGE,
    CTRL_DOWN_MOD
};

// Define layers
enum layers {
    _BASE = 0,   // Base layer
    _UTILITY_LAYER   // Utility layer (e.g., for copy/paste, app shortcuts)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT( /* Base */
        KC_BTN1, DRAG_SCROLL, BTN3_SCROLL,
          KC_BTN2, CTRL_DOWN_MOD
    ),

    [_UTILITY_LAYER] = LAYOUT( /* Utility Layer - Hold Bottom-Right Button */
        LGUI(KC_C),   LGUI(KC_GRV), LGUI(KC_V),
          LGUI(KC_X), KC_TRNS
    ),
};

// Track when the key was pressed (for timing calculations)
static uint16_t btn3_timer;
static bool     btn3_held = false;
// Track the ctrl_down button
static uint16_t ctrl_down_timer;
static bool     ctrl_down_held = false;

// For BTN3_SCROLL specific drag scroll exit
static bool btn3_tapped_to_exit_drag_scroll = false;

// For Sniper DPI mode
static bool sniper_dpi_active; // Initialized in keyboard_post_init_user

// keyboard_config.dpi_config: 0 for 400 DPI, 1 for 1000 DPI based on PLOOPY_DPI_OPTIONS
// PLOOPY_DPI_DEFAULT is 1 (1000 DPI)

// Function to toggle Sniper DPI mode
void toggle_sniper_mode(void) {
    if (sniper_dpi_active) {
        // Was sniper (400 DPI), switch to normal (1000 DPI)
        keyboard_config.dpi_config = 1; // Index 1 for 1000 DPI
        sniper_dpi_active          = false;
    } else {
        // Was normal (1000 DPI), switch to sniper (400 DPI)
        keyboard_config.dpi_config = 0; // Index 0 for 400 DPI
        sniper_dpi_active          = true;
    }
    eeconfig_update_kb(keyboard_config.raw);
    pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    // If drag scroll is active and any key (OTHER THAN DRAG_SCROLL key itself, or BTN3_SCROLL)
    // is pressed, ensure drag scroll is turned off.
    // The DRAG_SCROLL keycode handles its own toggling through ploopyco.c.
    // BTN3_SCROLL has specific logic below.
    if (is_drag_scroll && record->event.pressed) {
        if (keycode != DRAG_SCROLL && keycode != BTN3_SCROLL) {
            toggle_drag_scroll(); // Turn off drag scroll
        }
    }

    switch (keycode) {
        case BTN3_SCROLL: // Top-Right Thumb button
            if (record->event.pressed) {
                btn3_timer                          = timer_read();
                btn3_held                           = true;
                btn3_tapped_to_exit_drag_scroll = false; // Reset for this press

                if (is_drag_scroll) {
                    // If drag scroll is active and BTN3 is pressed,
                    // consume this press to ONLY exit drag scroll.
                    toggle_drag_scroll(); // This will set is_drag_scroll to false
                    btn3_tapped_to_exit_drag_scroll = true;
                }
            } else { // Key released
                btn3_held = false;
                if (!btn3_tapped_to_exit_drag_scroll && !is_drag_scroll && timer_elapsed(btn3_timer) < TAPPING_TERM) {
                    // Tap action: Ctrl+Down
                    tap_code16(LCTL(KC_DOWN));
                }
                // Hold action is handled in matrix_scan_user
                btn3_timer = 0;
            }
            return false;

        case CTRL_DOWN_MOD: // Bottom-Right Ring/Pinky button
            if (record->event.pressed) {
                ctrl_down_timer = timer_read();
                ctrl_down_held  = true;
            } else { // Key released
                ctrl_down_held = false;

                if (IS_LAYER_ON(_UTILITY_LAYER)) {
                    // If Utility Layer was active due to hold, turn it off.
                    layer_off(_UTILITY_LAYER);
                } else if (timer_elapsed(ctrl_down_timer) < TAPPING_TERM) {
                    // Tap action: Toggle Sniper DPI
                    toggle_sniper_mode();
                }
                // Hold action (activating Utility Layer) is handled in matrix_scan_user
                ctrl_down_timer = 0;
            }
            return false;

        default:
            return true;
    }
}

// This function runs frequently while keys are being processed
void matrix_scan_user(void) {
    // Hold action for BTN3_SCROLL (Top-Right Thumb)
    if (btn3_held && !is_drag_scroll && !btn3_tapped_to_exit_drag_scroll && timer_elapsed(btn3_timer) > TAPPING_TERM) {
        // Hold action: Ctrl+Up
        tap_code16(LCTL(KC_UP));
        btn3_held = false; // Consume the hold action to prevent repeats
    }

    // Hold action for CTRL_DOWN_MOD (Bottom-Right Ring/Pinky)
    if (ctrl_down_held && !IS_LAYER_ON(_UTILITY_LAYER) && timer_elapsed(ctrl_down_timer) > TAPPING_TERM) {
        // Activate the utility layer
        layer_on(_UTILITY_LAYER);
    }
}

// Called once at keyboard initialization.
void keyboard_post_init_user(void) {
    // keyboard_config.dpi_config is already loaded from EEPROM or set to PLOOPY_DPI_DEFAULT.
    // PLOOPY_DPI_DEFAULT is 1 (our 1000 DPI setting).
    // Index 0 is 400 DPI (sniper).
    // Index 1 is 1000 DPI (normal).

    if (keyboard_config.dpi_config == 0) {
        sniper_dpi_active = true;
    } else if (keyboard_config.dpi_config == 1) {
        sniper_dpi_active = false;
    } else {
        // Invalid dpi_config found in EEPROM, reset to default.
        keyboard_config.dpi_config = PLOOPY_DPI_DEFAULT; // Should be 1
        sniper_dpi_active = false; // Default is not sniper mode
        eeconfig_update_kb(keyboard_config.raw);
        // Ploopy's base init should handle setting the actual CPI from this corrected config.
    }
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (IS_LAYER_ON(_UTILITY_LAYER)) {
        // If the UTILITY_LAYER is active, suppress scroll wheel events
        mouse_report.v = 0; // Zero out vertical scroll
        mouse_report.h = 0; // Zero out horizontal scroll
    }
    return mouse_report;
}

// Called by QMK when a physical encoder (scroll wheel) is turned.
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (IS_LAYER_ON(_UTILITY_LAYER)) {
        if (clockwise) { // Typically scroll down / forward
            utility_scroll_accumulator++;
            if (utility_scroll_accumulator >= UTILITY_SCROLL_THRESHOLD) {
                tap_code(KC_BTN5); // Forward
                utility_scroll_accumulator = 0;
            }
        } else { // Counter-clockwise, typically scroll up / backward
            utility_scroll_accumulator--;
            if (utility_scroll_accumulator <= -UTILITY_SCROLL_THRESHOLD) {
                tap_code(KC_BTN4); // Back
                utility_scroll_accumulator = 0;
            }
        }
        // On the Utility Layer, we've consumed the scroll event for our custom Back/Forward.
        // Prevent the default scroll action.
        return false;
    } else if (is_drag_scroll) {
        // If not on Utility Layer, but drag scroll is active,
        // physical scroll wheel usage deactivates drag scroll.
        toggle_drag_scroll(); // This will set is_drag_scroll to false

        // Allow this specific scroll event that broke drag scroll to pass through as a normal scroll.
        return true;
    }

    // If not on Utility Layer and drag scroll is not active, allow normal scroll behavior.
    return true;
}
