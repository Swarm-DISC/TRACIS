/*

    TRACIS: tools/tiiGcrDetector/indexing.h

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

#ifndef _TRACIS_INDEXING_H
#define _TRACIS_INDEXING_H

#define ADDR(n, m, d) (((float*)dataBuffers[(n)]+(d*timeIndex + m)))
#define MEAS(n, m, d) ((float)(*(ADDR(n, m, d))))
#define TIME_ADDR() (((double*)dataBuffers[0]+(timeIndex)))
#define TIME() ((double)*((double*)dataBuffers[0]+(timeIndex)))
#define LAT() ((double)*((double*)dataBuffers[1]+(timeIndex)))
#define LON() ((double)*((double*)dataBuffers[2]+(timeIndex)))
#define RADIUS() ((double)*((double*)dataBuffers[3]+(timeIndex)))
#define VALID_IMAGERY_H() ((uint8_t)*((uint8_t*)dataBuffers[4]+(timeIndex)))
#define VALID_IMAGERY_V() ((uint8_t)*((uint8_t*)dataBuffers[5]+(timeIndex)))
#define VMCPH() (MEAS(6, 0, 1))
#define VMCPV() (MEAS(7, 0, 1))
#define VPHOSH() (MEAS(8, 0, 1))
#define VPHOSV() (MEAS(9, 0, 1))
#define VBIASH() (MEAS(10, 0, 1))
#define VBIASV() (MEAS(11, 0, 1))
#define RAW_IMAGE_H() (((uint16_t*)dataBuffers[12] + 2640 * (timeIndex)))
#define RAW_IMAGE_V() (((uint16_t*)dataBuffers[13] + 2640 * (timeIndex)))

#endif // _TRACIS_INDEXING_H
