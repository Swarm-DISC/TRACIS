/*

    TRACIS Processor: tools/tracis/utilities.h

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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include <stdlib.h>

#include <cdf.h>


// Constructs a full path to the export CDF file in the argument constructExportFileName.
int constructExportFileNames(const char satellite, long year, long month, long day, const char *exportDir, char *cdfFileNameLR, char *cdfFileNameHR);

// Returns the number of records in the faceplate current CDF file.
long numberOfAvailableRecords(const char *ifpFilename);

void closeCdf(CDFid id);

// Prints an error message from the CDFstatus
void printErrorMessage(CDFstatus status);

typedef struct ImageStorage {
    // Obtained at full image cadence
    double *imageTimes;
    uint8_t *rawImagesH;
    uint8_t *rawImagesV;
    uint8_t *correctedImagesH;
    uint8_t *correctedImagesV;
    uint8_t *validImageryH;
    uint8_t *validImageryV;
    uint8_t *imagingMode;
    uint8_t *anomalyFlagH;
    uint8_t *anomalyFlagV;

    uint16_t *ccdDarkCurrentH;
    uint16_t *ccdDarkCurrentV;
    float *ccdTemperatureH;
    float *ccdTemperatureV;
    float *VMcpH;
    float *VMcpV;
    float *VPhosH;
    float *VPhosV;
    float *VBiasH;
    float *VBiasV;
    float *VFaceplate;
    float *ShutterDutyCycleH;
    float *ShutterDutyCycleV;
    double *latitude;
    double *longitude;
    double *radius;
    double *vn;
    double *ve;
    double *vc;

    float *energyMapH;
    float *energyMapV;
    float *angleOfArrivalMapH;
    float *angleOfArrivalMapV;

    float *energySpectrumH;
    float *energySpectrumV;
    float *angleOfArrivalSpectrumH;
    float *angleOfArrivalSpectrumV;

    float *rawEnergySpectrumH;
    float *rawEnergySpectrumV;
    float *rawAngleOfArrivalSpectrumH;
    float *rawAngleOfArrivalSpectrumV;

    float *energiesH;
    float *energiesV;
    float *anglesOfArrival;

    // Obtained at 2 Hz
    double *colSumTimes;
    float *biasGridVoltageSettingH;
    float *biasGridVoltageSettingV;
    float *mcpVoltageSettingH;
    float *mcpVoltageSettingV;
    float *phosphorVoltageSettingH;
    float *phosphorVoltageSettingV;
    uint16_t *colSumSpectrumH;
    uint16_t *colSumSpectrumV;
    float *colSumEnergiesH;
    float *colSumEnergiesV;
    uint8_t *colSumImagingMode;

} ImageStorage;

void initImageStorage(ImageStorage *store);

int allocateImageMemory(ImageStorage *store, size_t numberOfImagePairs, size_t numberOfColumnSums);

void freeImageMemory(ImageStorage *store);

int getInputFilename(const char satelliteLetter, long year, long month, long day, const char *path, const char *dataset, char *filename);

int dayOfYear(long year, long month, long day, int* yday);

enum UTIL_ERRORS {
    UTIL_NO_ERROR = 0,
    UTIL_ERR_FP_FILENAME = -1,
    UTIL_ERR_HM_FILENAME = -2,
    UTIL_ERR_INPUT_FILE_MISMATCH = -3,
    UTIL_ERR_SATELLITE_LETTER = -4,
    UTIL_ERR_DAY_OF_YEAR_CONVERSION = -5,
    UTIL_ERR_MEMORY = -6,
    UTIL_ERR_DATE_ADJUST = -7
};

int dateAdjust(int *year, int *month, int *day, int deltaDays);

void utcDateString(time_t seconds, char *dateString);

void utcDateStringWithMicroseconds(double seconds, char *dateString);

void utcNowDateString(char *dateString);


#endif // UTILITIES_H
