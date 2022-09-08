/*

    TRACIS Processor: tools/tracis/load_satellite_velocity.h

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

#ifndef _LOAD_SATELLITE_VELOCITY_H
#define _LOAD_SATELLITE_VELOCITY_H

#include <stdint.h>
#include <stdlib.h>

enum SAT_POSVEL_ERRORS {
    SAT_OK = 0,
    SAT_ERROR_FILE = -1,
    SAT_ERROR_UNAVAILABLE = -2,
    SAT_ERROR_TOO_FEW_EPOCHS = -3,
    SAT_ERROR_MEMORY = -4,
    SAT_ERROR_WRONG_NUMBER_OF_RECORDS_READ = -5
};

typedef struct Ephemeres 
{
    double *time;
    double *X;
    double *Y;
    double *Z;
    double *VN;
    double *VE;
    double *VC;
    double *Latitude;
    double *Longitude;
    double *Radius;
    size_t nEphem;
} Ephemeres;

void initEphemeres(Ephemeres *ephem);
int allocEphemeres(Ephemeres *ephem, size_t nEphem);
void freeEphemeres(Ephemeres *ephem);

// Using long to be consistent with CDF epoch parsing in slidem.c
int loadEphemeres(const char *modFilename, Ephemeres *ephem);

#endif // _LOAD_SATELLITE_VELOCITY_H
