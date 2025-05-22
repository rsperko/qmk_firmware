/* Copyright 2020 Colin Lam (Ploopy Corporation)
 * Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
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

#pragma once

// Custom DPI options (400 and 1000)
#define PLOOPY_DPI_OPTIONS { 400, 1000 }
#define PLOOPY_DPI_DEFAULT 1  // Default to 1000 DPI

// Tapping term for tap-hold functionality (in milliseconds)
#define TAPPING_TERM 250  // Adjust this value to change how long you need to hold for tap-hold key functions

// Drag Scroll Configuration (Ploopy Specific)
// To make drag scroll slower, increase these divisor values (e.g., 12.0, 16.0). Default is 8.0.
#define PLOOPY_DRAGSCROLL_DIVISOR_H 8.0
#define PLOOPY_DRAGSCROLL_DIVISOR_V 8.0
// Uncomment to invert drag scroll direction (like macOS natural scroll for vertical)
// #define PLOOPY_DRAGSCROLL_INVERT
