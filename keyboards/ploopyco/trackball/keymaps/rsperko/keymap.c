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

// Define custom keycodes
enum custom_keycodes {
    DPI_TOGGLE = SAFE_RANGE,
    BTN3_SCROLL,
    CTRL_DOWN_MOD // New keycode for the dual-function key
};

// Define layers
enum layers {
    _BASE = 0,   // Base layer
    _NAV_LAYER   // Navigation layer for back/forward
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT( /* Base */
        KC_BTN1, DPI_TOGGLE, BTN3_SCROLL,
          KC_BTN2, CTRL_DOWN_MOD
    ),

    [_NAV_LAYER] = LAYOUT( /* Navigation Layer - Hold Bottom-Right Button */
        KC_BTN4, LGUI(KC_GRV), KC_BTN5,    // Top-Left: Back, Top-Middle: Spotlight, Top-Right: Forward
          LGUI(KC_SPC), KC_TRNS             // Bottom-Left: Cycle App Windows, Bottom-Right: Transparent (Layer Activator)
    ),
};

// Track if drag scroll is active
static bool drag_scroll_active = false;
// Track when the key was pressed (for timing calculations)
static uint16_t btn3_timer;
// Track if button 3 is being held
static bool btn3_held = false;
// Track the ctrl_down button
static uint16_t ctrl_down_timer;
static bool ctrl_down_held = false;

// For BTN3_SCROLL specific drag scroll exit
static bool btn3_tapped_to_exit_drag_scroll = false;

// For DPI_TOGGLE tap/hold (sniper mode)
static bool dpi_toggle_held = false;
static uint16_t dpi_toggle_timer;
static bool sniper_mode_was_active = false; // True if the hold action (sniper) was engaged
static uint8_t dpi_before_hold;         // Stores keyboard_config.dpi_config at press time for DPI_TOGGLE

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    // If drag scroll is active and any key is pressed, exit drag scroll mode
    // This general check now excludes BTN3_SCROLL as it has specific handling.
    if (drag_scroll_active && record->event.pressed && keycode != BTN3_SCROLL) {
        toggle_drag_scroll();
        drag_scroll_active = false;
    }

    switch (keycode) {
        case DPI_TOGGLE:
            if (record->event.pressed) {
                dpi_toggle_timer = timer_read();
                dpi_toggle_held = true;
                dpi_before_hold = keyboard_config.dpi_config; // Store current DPI in case it's a hold
                sniper_mode_was_active = false; // Reset for this press event
            } else { // Key released
                dpi_toggle_held = false;
                if (sniper_mode_was_active) {
                    // Sniper mode was activated by holding, restore the original DPI
                    keyboard_config.dpi_config = dpi_before_hold;
                    eeconfig_update_kb(keyboard_config.raw);
                    pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
                    sniper_mode_was_active = false; // Clear the flag
                } else if (timer_elapsed(dpi_toggle_timer) < TAPPING_TERM) {
                    // It was a tap
                    // Toggle from the DPI state that was active when the key was pressed
                    keyboard_config.dpi_config = (dpi_before_hold == 0) ? 1 : 0;
                    eeconfig_update_kb(keyboard_config.raw);
                    pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
                }
                // If it was a hold but sniper mode condition wasn't met (e.g. already on lowest DPI),
                // and it wasn't a tap, nothing happens on release, which is correct.
                dpi_toggle_timer = 0;
            }
            return false; // Skip all further processing of this key

        case BTN3_SCROLL:
            if (record->event.pressed) {
                btn3_timer = timer_read();
                btn3_held = true;
                btn3_tapped_to_exit_drag_scroll = false; // Reset for this press

                if (drag_scroll_active) {
                    // If drag scroll is active and BTN3 is pressed,
                    // consume this press to ONLY exit drag scroll.
                    toggle_drag_scroll();
                    drag_scroll_active = false;
                    btn3_tapped_to_exit_drag_scroll = true;
                }
            } else { // Key released
                btn3_held = false;

                // If it wasn't a tap used to exit drag scroll, AND
                // drag scroll isn't currently active (meaning the hold didn't activate it during *this* press-hold-release cycle), AND
                // it was a short press (tap)
                if (!btn3_tapped_to_exit_drag_scroll && !drag_scroll_active && timer_elapsed(btn3_timer) < TAPPING_TERM) {
                    tap_code16(LCTL(KC_UP));
                }
                // Reset timer regardless
                btn3_timer = 0;
            }
            return false; // Skip all further processing of this key

        case CTRL_DOWN_MOD:
            if (record->event.pressed) {
                // Key pressed - start timer and mark as held
                ctrl_down_timer = timer_read();
                ctrl_down_held = true;
            } else {
                // Key released
                ctrl_down_held = false;

                // Turn off the layer if it was activated
                if (IS_LAYER_ON(_NAV_LAYER)) {
                    layer_off(_NAV_LAYER);
                } else if (timer_elapsed(ctrl_down_timer) < TAPPING_TERM) {
                    // If released quickly and layer wasn't activated, send Ctrl+Down
                    tap_code16(LCTL(KC_DOWN));
                }

                ctrl_down_timer = 0;
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
        // And it wasn't a press that was consumed just to exit drag scroll
        if (!btn3_tapped_to_exit_drag_scroll) {
            toggle_drag_scroll();
            drag_scroll_active = true;
        }
        // Don't set btn3_held to false here so the user can release naturally;
        // drag_scroll_active state handles behavior on next press or release.
    }

    // Check if CTRL_DOWN_MOD is being held down
    if (ctrl_down_held && !IS_LAYER_ON(_NAV_LAYER) && timer_elapsed(ctrl_down_timer) > TAPPING_TERM) {
        // If the key has been held longer than TAPPING_TERM, activate the navigation layer
        layer_on(_NAV_LAYER);
    }

    // Sniper DPI mode for DPI_TOGGLE
    if (dpi_toggle_held && !sniper_mode_was_active && timer_elapsed(dpi_toggle_timer) > TAPPING_TERM) {
        // dpi_before_hold was set on key press.
        // Activate sniper mode if current DPI (dpi_before_hold) is not the sniper DPI (index 0).
        if (dpi_before_hold != 0) { // Ploopy DPI option 0 is the lowest (400 DPI)
            keyboard_config.dpi_config = 0; // Set to sniper DPI (index 0)
            eeconfig_update_kb(keyboard_config.raw);
            pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
            sniper_mode_was_active = true; // Mark that sniper mode (hold action) has occurred
        }
    }
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (IS_LAYER_ON(_NAV_LAYER)) {
        // If the NAV_LAYER is active, we want to suppress scroll wheel events
        // to prevent accidental scrolling when trying to use layer functions
        // mapped to the scroll wheel button (DPI_TOGGLE).
        mouse_report.v = 0; // Zero out vertical scroll
        mouse_report.h = 0; // Zero out horizontal scroll
    }
    return mouse_report;
}

// Called by QMK when a physical encoder (scroll wheel) is turned.
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (drag_scroll_active) {
        // Physical scroll wheel was used while drag scroll mode was active.
        // Deactivate drag scroll mode.
        toggle_drag_scroll();      // Deactivate QMK's internal drag scroll state.
        drag_scroll_active = false; // Update our local tracking flag.

        // By returning true here, we allow the default QMK encoder action to proceed.
        // Since drag_scroll is now false, this will be a normal scroll event.
        // If the _NAV_LAYER is also active, pointing_device_task_user will then suppress this scroll.
        return true;
    }

    // If drag scroll is not active, let QMK handle the encoder event normally for standard scrolling.
    return true;
}
