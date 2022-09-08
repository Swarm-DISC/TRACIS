/*

    TRACIS Processor: tools/tracis/image_analysis.h

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

#ifndef _IMAGE_ANALYSIS_H
#define _IMAGE_ANALYSIS_H

#include <stdint.h>

void calculateRadiusMap(char satellite, int sensor, float *radiusMap);

void calculateEnergyMap(char satellite, int sensor, float innerDomeVoltage, float mcpVoltage, float *energyMap);

void calculateAngleOfArrivalMap(char satellite, int sensor, float *energyMap);

void energySpectrum(uint16_t *image, float *energyMap, float *radiusMap, double *gainMap, float innerDomeBias, float mcpVoltage, float *energySpectrum, float *energies);

void angleOfArrivalSpectrum(uint16_t *image, float *angleOfArrivalMap, float *radiusMap, double *gainMap, float *angleOfArrivalSpectrum, float *anglesOfArrival);

void detectorCoordinates(char satellite, int sensor, float *xc, float *yc);

float eofr(double r, float innerDomeVoltage, float mcpVoltage);

int energyBin(float energy);


#endif // _IMAGE_ANALYSIS_H
