# Custom Ploopy Trackball Configuration

This keymap implements several advanced features for the Ploopy trackball:

## 1. DPI TOGGLE (Button 2)

-   Toggles between 400 and 1200 DPI
-   1200 DPI is the default setting

## 2. TAP/HOLD FUNCTIONALITY FOR BUTTON 3

-   Tapping sends Ctrl+Up
-   Holding activates drag scrolling mode
-   Pressing any button while drag scrolling is active will deactivate it

## 3. TAP/HOLD FUNCTIONALITY FOR BOTTOM-RIGHT BUTTON

-   Tapping sends Ctrl+Down
-   Holding activates a navigation layer where:
    -   Left button (normally left-click) becomes back button (KC_BTN4)
    -   Button 3 becomes forward button (KC_BTN5)

## Implementation Details

Implementation uses QMK's layer system, custom keycodes, and timer-based
behaviors to differentiate between taps and holds, creating an
ergonomic multi-functional trackball setup.

The TAPPING_TERM is set to 200ms by default, meaning you need to hold a button
for more than 200ms to activate the hold function.
