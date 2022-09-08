/*

    TRACIS Processor: tools/tracis/interpolate.h

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

#ifndef _INTERPOLATE_H
#define _INTERPOLATE_H

#include "load_satellite_velocity.h"
#include <stdint.h>
#include <stdlib.h>
// #include <gsl/gsl_interp.h>
// #include <gsl/gsl_spline.h>

void interpolateEphemeres(Ephemeres *ephem, double *times, size_t numberOfTimes, Ephemeres *newEphem);

#endif // _INTERPOLATE_H
