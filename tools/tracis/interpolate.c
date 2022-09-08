/*

    TRACIS Processor: tools/tracis/interpolate.c

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

#include "interpolate.h"
#include "tracis.h"
#include "tracis_settings.h"

#include <math.h>

void interpolateEphemeres(Ephemeres *ephem, double *times, size_t numberOfTimes, Ephemeres *newEphem)
{
    if (ephem->nEphem < MINIMUM_VELOCITY_EPOCHS)
        return;
    
    size_t lastIndex = 0;
    double thisTime = 0;
    double t1 = 0, t2 = 0, dt = 0;
    double fraction = 0;
    double v1 = 0, v2 = 0, dv = 0;
    double x = 0, y = 0, z = 0;
    double rad = 0;
    double lat = 0;
    double lon = 0;
    for (size_t i = 0; i < numberOfTimes; i++)
    {
        thisTime = times[i];
        // 
        while (ephem->time[lastIndex] <= thisTime && lastIndex < ephem->nEphem) 
        {
            lastIndex++;
        }
        // Extrapolate earlier or later, or the times are the same
        newEphem->time[i] = thisTime;
        if (ephem->time[lastIndex] >= thisTime || lastIndex == ephem->nEphem - 1)
        {
            newEphem->X[i] = ephem->X[lastIndex];
            newEphem->Y[i] = ephem->Y[lastIndex];
            newEphem->Z[i] = ephem->Z[lastIndex];
            newEphem->VN[i] = ephem->VN[lastIndex];
            newEphem->VE[i] = ephem->VE[lastIndex];
            newEphem->VC[i] = ephem->VC[lastIndex];
            newEphem->Latitude[i] = ephem->Latitude[lastIndex];
            newEphem->Longitude[i] = ephem->Longitude[lastIndex];
            newEphem->Radius[i] = ephem->Radius[lastIndex];
        }
        // Interpolate
        else
        {
            t1 = ephem->time[lastIndex];
            t2 = ephem->time[lastIndex+1];
            // Assumes t2 > t1
            dt = thisTime - t1;
            fraction = dt / (t2 - t1);
            newEphem->X[i] = ephem->X[lastIndex] + (ephem->X[lastIndex+1] - ephem->X[lastIndex]) * fraction;
            newEphem->Y[i] = ephem->Y[lastIndex] + (ephem->Y[lastIndex+1] - ephem->Y[lastIndex]) * fraction;
            newEphem->Z[i] = ephem->Z[lastIndex] + (ephem->Z[lastIndex+1] - ephem->Z[lastIndex]) * fraction;
            newEphem->VN[i] = ephem->VN[lastIndex] + (ephem->VN[lastIndex+1] - ephem->VN[lastIndex]) * fraction;
            newEphem->VE[i] = ephem->VE[lastIndex] + (ephem->VE[lastIndex+1] - ephem->VE[lastIndex]) * fraction;
            newEphem->VC[i] = ephem->VC[lastIndex] + (ephem->VC[lastIndex+1] - ephem->VC[lastIndex]) * fraction;
            x = newEphem->X[i];
            y = newEphem->Y[i];
            z = newEphem->Z[i];
            rad = sqrt(x*x + y*y + z*z);
            newEphem->Radius[i] = rad;
            newEphem->Latitude[i] = 90. - asin(z / rad) * 180.0 / M_PI;
            // Longitude
            newEphem->Longitude[i] = atan2(y, x) * 180.0 / M_PI;
        }
    }

    return;

}
