# Custom Ploopy Trackball Configuration

This keymap implements several advanced features for the Ploopy trackball:

## 1. DPI_TOGGLE (Top-Middle Button / Scroll Wheel Button)

-   **Scroll Wheel:** Normal mouse scroll.
-   **Button Tap:** Toggles Drag Scroll mode on/off.
-   **Button Hold:** No action.

## 2. BTN3_SCROLL (Top-Right Thumb Button)

-   **Tap:** Sends Ctrl+Down.
-   **Hold:** Sends Ctrl+Up.
-   _If Drag Scroll is active, tapping this button will deactivate Drag Scroll mode (and not send Ctrl+Down)._

## 3. CTRL_DOWN_MOD (Bottom-Right Ring/Pinky Button)

-   **Tap:** Toggles Sniper DPI mode. Switches between 1000 DPI (normal) and 400 DPI (sniper).
-   **Hold:** Activates a "Utility Layer" where:
    -   Top-Left button (physical KC_BTN1 position) performs Copy (Cmd+C).
    -   Top-Middle button (physical DPI_TOGGLE position) cycles through windows of the current application (Cmd+`).
    -   Top-Right button (physical BTN3_SCROLL position) performs Paste (Cmd+V).
    -   Bottom-Left button (physical KC_BTN2 position) activates macOS Spotlight Search (Cmd+Space).

## 4. KC_BTN1 (Top-Left Thumb Button)

-   Left Click.

## 5. KC_BTN2 (Bottom-Left Ring/Pinky Button)

-   Right Click.

## Implementation Details

Implementation uses QMK's layer system, custom keycodes, and timer-based
behaviors to differentiate between taps and holds, creating an
ergonomic multi-functional trackball setup.

The TAPPING_TERM (for differentiating tap vs. hold for BTN3_SCROLL and CTRL_DOWN_MOD) is set in `config.h` (currently 250ms).
