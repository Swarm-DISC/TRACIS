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
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "image_analysis.h"

#include "tracis_settings.h"

#include <tii/isp.h>
#include <tii/detector.h>

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

int energyBin(float energy)
{
    static float energyBinBoundaries[ENERGY_BINS+1] =  {0, 0.25, 0.32164, 0.41381, 0.532392, 0.684956, 0.881237, 1.13377, 1.45866, 1.87666, 2.41443, 3.10632, 3.99647, 5.1417, 6.61512, 8.51076, 10.9496, 14.0874, 18.1243, 23.318, 30.};

    if (energy < energyBinBoundaries[0])
        return -1;

    int bin = 0;
    while (bin < ENERGY_BINS && energy > energyBinBoundaries[bin])
        bin++;
    
    return bin;
}
