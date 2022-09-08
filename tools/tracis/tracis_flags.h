/*

    TRACIS Processor: tools/tracis/tracis_flags.h

    Copyright (C) 2022  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Adapted from the SLIDEM processor source code

#ifndef _TRACIS_FLAGS_H
#define _TRACIS_FLAGS_H

// 32 bit flags
enum TRACIS_PRODUCT_FLAGS {
    TRACIS_FLAG_ESTIMATE_OK = 0,
    TRACIS_FLAG_CLASSIC_WING_ANOMALY = 1 << 0,
    TRACIS_FLAG_UPPER_ANGELS_WING_ANOMALY = 1 << 1,
    TRACIS_FLAG_LOWER_ANGELS_WING_ANOMALY = 1 << 2,
    TRACIS_FLAG_PERIPHERAL_ANOMALY = 1 << 3,
    TRACIS_FLAG_MEASLES_ANOMALY = 1 << 4,
    TRACIS_FLAG_BIFURCATION_ANOMALY = 1 << 5
};


#endif // _TRACIS_FLAGS_H
