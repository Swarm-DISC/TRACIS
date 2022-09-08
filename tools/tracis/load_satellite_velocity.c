/*

    TRACIS Processor: tools/tracis/load_satellite_velocity.c

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

#include "load_satellite_velocity.h"

#include "tracis_settings.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

extern char infoHeader[50];

void initEphemeres(Ephemeres *ephem)
{
    ephem->time = NULL;
    ephem->X = NULL;
    ephem->Y = NULL;
    ephem->Z = NULL;
    ephem->VN = NULL;
    ephem->VE = NULL;
    ephem->VC = NULL;
    ephem->Latitude = NULL;
    ephem->Longitude = NULL;
    ephem->Radius = NULL;
    ephem->nEphem = 0;
}

void freeEphemeres(Ephemeres *ephem)
{
    free(ephem->time);
    free(ephem->X);
    free(ephem->Y);
    free(ephem->Z);
    free(ephem->VN);
    free(ephem->VE);
    free(ephem->VC);
    free(ephem->Latitude);
    free(ephem->Longitude);
    free(ephem->Radius);
    ephem->nEphem = 0;
}

int allocEphemeres(Ephemeres *ephem, size_t nEphem)
{
    ephem->nEphem = ephem->nEphem + nEphem;

    ephem->time = (double*) realloc(ephem->time, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->time == NULL)
        return SAT_ERROR_MEMORY;
    ephem->X = (double*) realloc(ephem->X, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->X == NULL)
        return SAT_ERROR_MEMORY;
    ephem->Y = (double*) realloc(ephem->Y, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->Y == NULL)
        return SAT_ERROR_MEMORY;
    ephem->Z = (double*) realloc(ephem->Z, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->Z == NULL)
        return SAT_ERROR_MEMORY;
    ephem->VN = (double*) realloc(ephem->VN, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->VN == NULL)
        return SAT_ERROR_MEMORY;
    ephem->VE = (double*) realloc(ephem->VE, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->VE == NULL)
        return SAT_ERROR_MEMORY;
    ephem->VC = (double*) realloc(ephem->VC, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->VC == NULL)
        return SAT_ERROR_MEMORY;
    ephem->Latitude = (double*) realloc(ephem->Latitude, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->Latitude == NULL)
        return SAT_ERROR_MEMORY;
    ephem->Longitude = (double*) realloc(ephem->Longitude, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->Longitude == NULL)
        return SAT_ERROR_MEMORY;
    ephem->Radius = (double*) realloc(ephem->Radius, (size_t) (ephem->nEphem * sizeof(double)));
    if (ephem->Radius == NULL)
        return SAT_ERROR_MEMORY;

    return SAT_OK;

}

int loadEphemeres(const char *modFilename, Ephemeres *ephem)
{
    int status = (int) SAT_ERROR_UNAVAILABLE;

    FILE *modFP = fopen(modFilename, "r");
    if (modFP == NULL)
    {
        return SAT_ERROR_FILE;
    }

    char buf[100] = {0};

    int year;
    int month;
    int day;
    int hour;
    int minute;
    double seconds;
    int sec;
    int msec;

    double x, y, z;
    double vx, vy, vz;
    double dummy;

    double cdfTime;

    char * str = NULL;
    int itemsConverted = 0;

    double cx, cy, cz, ex, ey, ez, nx, ny, nz;
    double cm, em, nm;
    double vn, ve, vc;
    double lat;
    double lon;
    double rad;


    long epochs = 0;
    long previousEpochs = ephem->nEphem;
    long records = ephem->nEphem;
    int a, b, c, d;
    double e;

    itemsConverted = fscanf(modFP, "%7c %d %d %d %d %lf %ld %5c %5c %3c %4c\n", buf, &a, &b, &c, &d, &e, &epochs, buf, buf, buf, buf);

    if (epochs < MINIMUM_VELOCITY_EPOCHS)
    {
        status = SAT_ERROR_TOO_FEW_EPOCHS;
        goto cleanup;
    }

    status = allocEphemeres(ephem, epochs);

    long lines = 1;

    while(fgets(buf, 100, modFP) != NULL)
    {
        lines++;
        if (buf[0] != '*')
        {
            continue;
        }
        sscanf(buf+2, "%d %d %d %d %d %lf", &year, &month, &day, &hour, &minute, &seconds);
        sec = (int) floor(seconds);
        msec = 1000 * (int)floor(seconds - (double)sec);
        cdfTime = computeEPOCH(year, month, day, hour, minute, sec, msec);
        if(fgets(buf, 100, modFP) == NULL || buf[0] != 'P')
        {
            status = SAT_ERROR_FILE;
            goto cleanup;
        }
        lines++;
        sscanf(buf+5, "%lf %lf %lf", &x, &y, &z);
        x *= 1000.0;
        y *= 1000.0;
        z *= 1000.0;
        if(fgets(buf, 100, modFP) == NULL || buf[0] != 'V')
        {
            status = SAT_ERROR_FILE;
            goto cleanup;
        }
        sscanf(buf+5, "%lf %lf %lf", &vx, &vy, &vz);
        lines++;
        records++;
        vx /= 10.;
        vy /= 10.;
        vz /= 10.;

        // Calculate Vnec
        // chat
        cx = -x; cy = -y; cz = -z;
        cm = sqrt(cx*cx + cy*cy + cz*cz);
        cx /= cm; cy /= cm; cz /= cm;
        // ehat
        ex = cy; ey = -cx; ez = 0.0;
        em = sqrt(ex*ex + ey*ey + ez*ez);
        ex /= em; ey /= em;
        // nhat
        nx = -cx * cz; ny = -cy * cz; nz = cx*cx + cy*cy;
        nm = sqrt(nx*nx + ny*ny + nz*nz);
        nx /= nm; ny /= nm; nz /= nm;

        // vnec
        vn = vx * nx + vy * ny + vz * nz;
        ve = vx * ex + vy * ey + vz * ez;
        vc = vx * cx + vy * cy + vz * cz;

        // Radius
        rad = sqrt(x*x + y*y + z*z);
        // Latitude
        lat = 90. - acos(z / rad) * 180.0 / M_PI;
        // Longitude
        lon = atan2(y, x) * 180.0 / M_PI;

        ephem->time[records-1] = cdfTime;
        ephem->X[records-1] = x;
        ephem->Y[records-1] = y;
        ephem->Z[records-1] = z;
        ephem->VN[records-1] = vn;
        ephem->VE[records-1] = ve;
        ephem->VC[records-1] = vc;
        ephem->Latitude[records-1] = lat;
        ephem->Longitude[records-1] = lon;
        ephem->Radius[records-1] = rad;
    }

    if (records != (epochs + previousEpochs))
    {
        status = SAT_ERROR_WRONG_NUMBER_OF_RECORDS_READ;
        goto cleanup;
    }

    status = SAT_OK;

cleanup:

    fclose(modFP);
    return status;

}

