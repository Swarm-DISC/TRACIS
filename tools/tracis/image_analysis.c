/*

    TRACIS Processor: tools/tracis/image_analysis.c

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

#include "image_analysis.h"

#include "tracis_settings.h"

#include <tii/isp.h>

#include <string.h>
#include <stdio.h>
#include <math.h>


void calculateRadiusMap(char satellite, int sensor, float *radiusMap)
{
    float x0 = 0.0;
    float y0 = 0.0;
    float x = 0.0;
    float y = 0.0;

    detectorCoordinates(satellite, sensor, &x0, &y0);

    for (int i = 0; i < TII_COLS; i++)
    {
        for (int j = 0; j < TII_ROWS; j++)
        {
            x = x0 - (float) i;
            y = y0 - (float) j;
            if (radiusMap != NULL)
                *(radiusMap+i*TII_ROWS + j) = hypot(x, y);
        }
    }

    return;
}

void calculateEnergyMap(char satellite, int sensor, float innerDomeVoltage, float mcpVoltage, float *energyMap)
{
    float x0 = 0.0;
    float y0 = 0.0;
    float x = 0.0;
    float y = 0.0;
    float rInner = R_INNER_PIXELS;
    float rMax = rInner + R_INNER_CUSHION; // Ions can reach beyond rInner, add arbitrary cushion
    float r = 0.0;

    // Functional form of E-vs-r, TBD, needs parameterization in MCP voltage
    float a = 0.0;
    float b = 0.0;
    float energy = 0.0;

    detectorCoordinates(satellite, sensor, &x0, &y0);

    for (int i = 0; i < TII_COLS; i++)
    {
        for (int j = 0; j < TII_ROWS; j++)
        {
            x = x0 - (float) i;
            y = y0 - (float) j;
            r = hypot(x, y);

            if (r < rMax)
            {
                energy = eofr((double) r, innerDomeVoltage, mcpVoltage);
                // Polynomials are extrapolated at lower radii
                // Set negative energies to -1.0 eV
                if (energy < 0.0)
                    energy = MISSING_ENERGY;
            }
            else
                energy = MISSING_ENERGY;

            *(energyMap+i*TII_ROWS + j) = energy;
        }
    }

    return;

}

void calculateAngleOfArrivalMap(char satellite, int sensor, float *angleOfArrivalMap)
{
    float x0 = 0.0;
    float y0 = 0.0;
    float x = 0.0;
    float y = 0.0;
    float rInner = R_INNER_PIXELS;
    float rho = 0.0;

    float angle = 0.0;

    detectorCoordinates(satellite, sensor, &x0, &y0);

    for (int i = 0; i < TII_COLS; i++)
    {
        for (int j = 0; j < TII_ROWS; j++)
        {
            x = x0 - (float) i;
            y = y0 - (float) j;
            rho = hypot(x, y) / rInner;
            if (rho < 1.0)
                angle = atan2(y, x) * 180.0 / M_PI;
            else
                angle = MISSING_ANGLE;

            *(angleOfArrivalMap+i*TII_ROWS + j) = angle;
        }
    }

}

void energySpectrum(uint16_t *image, float *energyMap, float *radiusMap, double *gainMap, float innerDomeBias, float mcpVoltage, float *energySpectrum, float *energies)
{
    float radius = 0.0;
    float energy = 0.0;
    float referenceSpectrum[ENERGY_BINS] = {0.0};
    float meanEnergies[ENERGY_BINS] = {0.0};
    float referenceSprectrumTotal = 0.0;
    float energySpectrumTotal = 0.0;

    int bin = 0;
    bzero(energySpectrum, sizeof(float) * ENERGY_BINS);

    float counts = 0.0;
    double gainValue = 0.0;

    // Energy bins based on uniform radius bins
    float rMin = MIN_RADIUS; // pixels
    float rMax = MAX_RADIUS;
    float deltaR = (rMax - rMin) / (float) ENERGY_BINS;
        
    for (int i = 0; i < IMAGE_COLS * IMAGE_ROWS; i++)
    {
        radius = *(radiusMap + i);
        energy = *(energyMap + i);
        if (gainMap != NULL)
            gainValue = *(gainMap + i); // Only add to spectrum if gain value is non-zero
        else 
            gainValue = 1.0; // No cropping
        // TBD custom normalization taking into account gain map cropping,
        // or is it better to use a constant normalization for all pixels within rInner?

        // energy == -1.0 indicates pixels beyond inner dome.
        if (energy != MISSING_ENERGY && gainValue > 0.0)
        {
            bin = (int) floor((radius - rMin) / deltaR);
            if (bin >=0 && bin < ENERGY_BINS)
            {
                counts = (float)(*(image + i));
                // TBD adjust counts to physical value, take into account geometry factor?
                *(energySpectrum + bin) += counts;
                energySpectrumTotal += counts;
                referenceSpectrum[bin] += 1.0;
                meanEnergies[bin] += energy;
                if (energy < 0)
                    printf("%f\n", energy);
                referenceSprectrumTotal++;
            }
        }
    }

    for (int i = 0; i < ENERGY_BINS; i++)
    {
        if (referenceSpectrum[i] > 0.0)
        {
            *(energySpectrum + i) /= (referenceSpectrum[i]);
            meanEnergies[i] /= referenceSpectrum[i];
        }
    }

    if (energies != NULL)
    {
        for (int i = 0; i < ENERGY_BINS; i++)
            *(energies + i) = meanEnergies[i];
    }


    return;

}

void angleOfArrivalSpectrum(uint16_t *image, float *angleOfArrivalMap, float *radiusMap, double *gainMap, float *angleOfArrivalSpectrum, float *anglesOfArrival)
{
    float angle = 0.0;
    float maxAngle = MAX_ANGLE;
    float minAngle = MIN_ANGLE;
    float deltaAngle = (maxAngle - minAngle) / (float) ANGULAR_BINS;
    float referenceSpectrum[ANGULAR_BINS] = {0.0};
    float meanAngle[ANGULAR_BINS] = {0.0};
    float referenceSprectrumTotal = 0.0;
    float angleOfArrivalSpectrumTotal = 0.0;

    float radius = 0.0;
    float rMin = MIN_RADIUS;

    int bin = 0;
    bzero(angleOfArrivalSpectrum, sizeof(float) * ANGULAR_BINS);

    float counts = 0.0;
    double gainValue = 0.0;

    for (int i = 0; i < IMAGE_COLS * IMAGE_ROWS; i++)
    {
        radius = *(radiusMap + i);
        angle = *(angleOfArrivalMap + i);
        if (gainMap != NULL)
            gainValue = *(gainMap + i); // Only add to spectrum if gain value is non-zero
        else 
            gainValue = 1.0; // No cropping
        // TBD custom normalization taking into account gain map cropping,
        // or is it better to use a constant normalization for all pixels within rInner?

        // angle == -200.0 indicates pixels beyond inner dome.
        if (angle != MISSING_ANGLE && gainValue > 0.0)
        {
            bin = (int) floor((angle - minAngle) / deltaAngle);
            // Angle-of-arrival spectrum needs to be consisitent with minimum radius for energy spectrum (due to negative energies in polynomial eofr model at low radii).
            if (bin >=0 && bin < ANGULAR_BINS && radius >= rMin)
            {
                counts = (float)(*(image + i));
                // TBD adjust counts to physical value, take into account geometry factor?
                *(angleOfArrivalSpectrum + bin) += counts;
                angleOfArrivalSpectrumTotal += counts;
                referenceSpectrum[bin] += 1.0;
                meanAngle[bin] += angle;
                referenceSprectrumTotal++;
            }
        }
    }

    for (int i = 0; i < ANGULAR_BINS; i++)
    {
        if (referenceSpectrum[i] > 0.0)
        {
            *(angleOfArrivalSpectrum + i) /= (referenceSpectrum[i]);
            meanAngle[i] /= referenceSpectrum[i];
        }
    }

    if (anglesOfArrival != NULL)
    {
        for (int i = 0; i < ANGULAR_BINS; i++)
            *(anglesOfArrival + i) = meanAngle[i];
    }

    return;

}

void detectorCoordinates(char satellite, int sensor, float *xc, float *yc)
{
    float x0 = 0.0;
    float y0 = 0.0;

    switch (satellite)
    {
        case 'A':
            if (sensor == H_SENSOR)
            {
                x0 = 33.0411;
                y0 = 31.1261;
            }
            else
            {
                x0 = 32.3852;
                y0 = 33.5174;
            }
            break;
        case 'B':
            if (sensor == H_SENSOR)
            {
                x0 = 33.8073;
                y0 = 32.496;
            }
            else
            {
                x0 = 33.8465;
                y0 = 33.3973;
            }
            break;
        case 'C':
            if (sensor == H_SENSOR)
            {
                x0 = 33.429;
                y0 = 29.2934;
            }
            else
            {
                x0 = 31.7489;
                y0 = 35.8786;
            }
            break;
        default:
            fprintf(stderr, "Invalid satellite in detectorCoordinates().\n");
            return;
    }

    if (xc != NULL)
        *xc = x0;

    if (yc != NULL)
        *yc = y0;

    return;
}


float eofr(double r, float innerDomeVoltage, float mcpVoltage)
{
    // Passing r by casting as double to avoid
    // making a temporary variable for r^3 calculation in double precision
    // TODO incorporate MCP voltage into transfer function

    double a = 0.0, b = 0.0, c = 0.0, d = 0.0;

    float energy = 0.0;

    // Model values from "Along-track-drift-anomaly.nb simulations with CEFI XTracer."

    if (innerDomeVoltage < -90.0) // -99.0 V simulation, need to update for actual voltages
    {
        a = -0.9849666707472252;
        b = 0.25076488696479904;
        c = 0.02909150117625129;
        d = 0.00017439906324331827;
    }
    else // Simulated for -60.3 V, but will apply to any other voltage until covered by new simulations.
    {
        a = -2.4014615344450805;
        b = 0.7439055999344776;
        c = -0.030922802815747417;
        d = 0.001209654508105981;
    }

    energy = (float) (a + b*r + c*r*r + d*r*r*r);

    if (innerDomeVoltage > -60.) // Scale by inner dome voltage until new simulations are done
    {
        energy = energy / 60.3 * fabs(innerDomeVoltage);
    }

    return energy;
}

int energyBin(float energy)
{
    // Mathematica generator: energies = {0}~Join~(10^Range[Log[10, 0.25], Log[10, 30.], (Log[10, 30./0.25]/39)])
    // 15 bins

    static float energyBinBoundaries[ENERGY_BINS+1] =  {0, 0.25, 0.351927, 0.495411, 0.697395, 0.981729, 1.38199, 1.94544, \
2.73861, 3.85517, 5.42696, 7.63958, 10.7543, 15.1389, 21.3112, 30.};

    if (energy < energyBinBoundaries[0])
        return -1;

    int bin = 0;
    while (bin < ENERGY_BINS && energy > energyBinBoundaries[bin])
        bin++;
    
    return bin;
}
