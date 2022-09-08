/*

    TRACIS Processor: tools/tracis/utilities.c

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

#include "utilities.h"

#include "tracis_settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <fts.h>
#include <math.h>



// Prefix for all fprintf messages
extern char infoHeader[50];


// Generates the filename for exported CDF file, with full path
int constructExportFileNames(const char satellite, long year, long month, long day, const char *exportDir, char *cdfFileNameLR, char *cdfFileNameHR)
{

    if (satellite != 'A' && satellite != 'B' && satellite != 'C')
    {
        return UTIL_ERR_SATELLITE_LETTER;
    }

    sprintf(cdfFileNameLR, "%s/SW_%s_EFI%c%s_%04d%02d%02dT000000_%04d%02d%02dT235959_%s", exportDir, TRACIS_PRODUCT_TYPE, satellite, TRACIS_PRODUCT_CODE_LR, (int)year, (int)month, (int)day, (int)year, (int)month, (int)day, EXPORT_VERSION_STRING);

    sprintf(cdfFileNameHR, "%s/SW_%s_EFI%c%s_%04d%02d%02dT000000_%04d%02d%02dT235959_%s", exportDir, TRACIS_PRODUCT_TYPE, satellite, TRACIS_PRODUCT_CODE_HR, (int)year, (int)month, (int)day, (int)year, (int)month, (int)day, EXPORT_VERSION_STRING);

    return UTIL_NO_ERROR;

}

long numberOfAvailableRecords(const char *fpFilename)
{
    CDFsetValidate(VALIDATEFILEoff);
    CDFid calCdfId;
    CDFstatus status;
    status = CDFopenCDF(fpFilename, &calCdfId);
    if (status != CDF_OK) 
    {
        // Not necessarily an error. For example, some dates will have not calibration data.
        printErrorMessage(status);
        return 0;
    }

    // Get number of records for zVar "epoch"
    long nRecords = 0;
    status = CDFgetzVarAllocRecords(calCdfId, CDFgetVarNum(calCdfId, "Timestamp"), &nRecords);
    if (status != CDF_OK) 
    {
        printErrorMessage(status);
        nRecords = 0;
    }
    closeCdf(calCdfId);
    return nRecords;

}

void closeCdf(CDFid id)
{
    CDFstatus status;
    status = CDFcloseCDF(id);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
    }

}

void printErrorMessage(CDFstatus status)
{
    char errorMessage[CDF_STATUSTEXT_LEN + 1];
    CDFgetStatusText(status, errorMessage);
    fprintf(stdout, "%s%s\n", infoHeader, errorMessage);
}

void initImageStorage(ImageStorage *store)
{
    store->imageTimes = NULL;
    store->rawImagesH = NULL;
    store->rawImagesV = NULL;
    store->correctedImagesH = NULL;
    store->correctedImagesV = NULL;
    store->validImageryH = NULL;
    store->validImageryV = NULL;
    store->imagingMode = NULL;
    store->anomalyFlagH = NULL;
    store->anomalyFlagV = NULL;
    store->ccdDarkCurrentH = NULL;
    store->ccdDarkCurrentV = NULL;
    store->ccdTemperatureH = NULL;
    store->ccdTemperatureV = NULL;
    store->VMcpH = NULL;
    store->VMcpV = NULL;
    store->VPhosH = NULL;
    store->VPhosV = NULL;
    store->VBiasH = NULL;
    store->VBiasV = NULL;
    store->VFaceplate = NULL;
    store->ShutterDutyCycleH = NULL;
    store->ShutterDutyCycleV = NULL;
    store->energyMapH = NULL;
    store->energyMapV = NULL;
    store->angleOfArrivalMapH = NULL;
    store->angleOfArrivalMapV = NULL;
    store->energySpectrumH = NULL;
    store->energySpectrumV = NULL;
    store->angleOfArrivalSpectrumH = NULL;
    store->angleOfArrivalSpectrumV = NULL;
    store->rawEnergySpectrumH = NULL;
    store->rawEnergySpectrumV = NULL;
    store->rawAngleOfArrivalSpectrumH = NULL;
    store->rawAngleOfArrivalSpectrumV = NULL;
    store->energiesH = NULL;
    store->energiesV = NULL;
    store->anglesOfArrival = NULL;

    store->colSumTimes = NULL;
    store->biasGridVoltageSettingH = NULL;
    store->biasGridVoltageSettingV = NULL;
    store->mcpVoltageSettingH = NULL;
    store->mcpVoltageSettingV = NULL;
    store->phosphorVoltageSettingH = NULL;
    store->phosphorVoltageSettingV = NULL;
    store->colSumSpectrumH = NULL;
    store->colSumSpectrumV = NULL;
    store->colSumEnergiesH = NULL;
    store->colSumEnergiesV = NULL;
    store->colSumImagingMode = NULL;
    
}

int allocateImageMemory(ImageStorage *store, size_t numberOfImagePairs, size_t numberOfColumnSums)
{
    if ((store->imageTimes = (double*)malloc(numberOfImagePairs * sizeof(double))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->rawImagesH = (uint8_t*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->rawImagesV = (uint8_t*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->correctedImagesH = (uint8_t*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->correctedImagesV = (uint8_t*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->validImageryH = (uint8_t*)malloc(numberOfImagePairs * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->validImageryV = (uint8_t*)malloc(numberOfImagePairs * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->imagingMode = (uint8_t*)malloc(numberOfImagePairs * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->anomalyFlagH = (uint8_t*)malloc(numberOfImagePairs * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->anomalyFlagV = (uint8_t*)malloc(numberOfImagePairs * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->ccdDarkCurrentH = (uint16_t*)malloc(numberOfImagePairs * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->ccdDarkCurrentV = (uint16_t*)malloc(numberOfImagePairs * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->ccdTemperatureH = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->ccdTemperatureV = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->VMcpH = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->VMcpV = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->VPhosH = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->VPhosV = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->VBiasH = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->VBiasV = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->VFaceplate = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->ShutterDutyCycleH = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->ShutterDutyCycleV = (float*)malloc(numberOfImagePairs * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->energyMapH = (float*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->energyMapV = (float*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->angleOfArrivalMapH = (float*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->angleOfArrivalMapV = (float*)malloc(numberOfImagePairs * IMAGE_ROWS * IMAGE_COLS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->energySpectrumH = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->energySpectrumV = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->angleOfArrivalSpectrumH = (float*)malloc(numberOfImagePairs * ANGULAR_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->angleOfArrivalSpectrumV = (float*)malloc(numberOfImagePairs * ANGULAR_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->rawEnergySpectrumH = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->rawEnergySpectrumV = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->rawAngleOfArrivalSpectrumH = (float*)malloc(numberOfImagePairs * ANGULAR_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;
    if ((store->rawAngleOfArrivalSpectrumV = (float*)malloc(numberOfImagePairs * ANGULAR_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->energiesH = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->energiesV = (float*)malloc(numberOfImagePairs * ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->anglesOfArrival = (float*)malloc(numberOfImagePairs * ANGULAR_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    // 2 Hz

    if ((store->colSumTimes = (double*)malloc(numberOfColumnSums * sizeof(double))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->biasGridVoltageSettingH = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->biasGridVoltageSettingV = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->mcpVoltageSettingH = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->mcpVoltageSettingV = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->phosphorVoltageSettingH = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->phosphorVoltageSettingV = (float*)malloc(numberOfColumnSums * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->colSumSpectrumH = (uint16_t*)malloc(numberOfColumnSums * COLUMN_SUM_ENERGY_BINS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->colSumSpectrumV = (uint16_t*)malloc(numberOfColumnSums * COLUMN_SUM_ENERGY_BINS * sizeof(uint16_t))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->colSumEnergiesH = (float*)malloc(numberOfColumnSums * COLUMN_SUM_ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->colSumEnergiesV = (float*)malloc(numberOfColumnSums * COLUMN_SUM_ENERGY_BINS * sizeof(float))) == NULL)
        return UTIL_ERR_MEMORY;

    if ((store->colSumImagingMode = (uint8_t*)malloc(numberOfColumnSums * sizeof(uint8_t))) == NULL)
        return UTIL_ERR_MEMORY;


    return UTIL_NO_ERROR;
}

void freeImageMemory(ImageStorage *store)
{
    free(store->imageTimes);
    free(store->rawImagesH);
    free(store->rawImagesV);
    free(store->correctedImagesH);
    free(store->correctedImagesV);
    free(store->validImageryH);
    free(store->validImageryV);
    free(store->imagingMode);
    free(store->anomalyFlagH);
    free(store->anomalyFlagV);
    free(store->ccdDarkCurrentH);
    free(store->ccdDarkCurrentV);
    free(store->ccdTemperatureH);
    free(store->ccdTemperatureV);
    free(store->VMcpH);
    free(store->VMcpV);
    free(store->VPhosH);
    free(store->VPhosV);
    free(store->VBiasH);
    free(store->VBiasV);
    free(store->VFaceplate);
    free(store->ShutterDutyCycleH);
    free(store->ShutterDutyCycleV);
    free(store->energyMapH);
    free(store->energyMapV);
    free(store->angleOfArrivalMapH);
    free(store->angleOfArrivalMapV);
    free(store->energySpectrumH);
    free(store->energySpectrumV);
    free(store->angleOfArrivalSpectrumH);
    free(store->angleOfArrivalSpectrumV);
    free(store->rawEnergySpectrumH);
    free(store->rawEnergySpectrumV);
    free(store->rawAngleOfArrivalSpectrumH);
    free(store->rawAngleOfArrivalSpectrumV);
    free(store->energiesH);
    free(store->energiesV);
    free(store->anglesOfArrival);
    free(store->colSumTimes);
    free(store->biasGridVoltageSettingH);
    free(store->biasGridVoltageSettingV);
    free(store->mcpVoltageSettingH);
    free(store->mcpVoltageSettingV);
    free(store->phosphorVoltageSettingH);
    free(store->phosphorVoltageSettingV);
    free(store->colSumSpectrumH);
    free(store->colSumSpectrumV);
    free(store->colSumEnergiesH);
    free(store->colSumEnergiesV);
    free(store->colSumImagingMode);

    return;
}

int getInputFilename(const char satelliteLetter, long year, long month, long day, const char *path, const char *dataset, char *filename)
{
	char *searchPath[2] = {NULL, NULL};
    searchPath[0] = (char *)path;

	FTS * fts = fts_open(searchPath, FTS_PHYSICAL | FTS_NOCHDIR, NULL);	
	if (fts == NULL)
	{
		printf("Could not open directory %s for reading.", path);
		return UTIL_ERR_HM_FILENAME;
	}
	FTSENT * f = fts_read(fts);

    bool gotHmFile = false;
    long fileYear;
    long fileMonth;
    long fileDay;
    long lastVersion = -1;
    long fileVersion;
	while(f != NULL)
	{
        // Most Swarm CDF file names have a length of 59 characters. The MDR_MAG_LR files have a lend of 70 characters.
        // The MDR_MAG_LR files have the same filename structure up to character 55.
		if ((strlen(f->fts_name) == 59 || strlen(f->fts_name) == 70) && *(f->fts_name+11) == satelliteLetter && strncmp(f->fts_name+13, dataset, 5) == 0)
		{
            char fyear[5] = { 0 };
            char fmonth[3] = { 0 };
            char fday[3] = { 0 };
            char version[5] = { 0 };
            strncpy(fyear, f->fts_name + 19, 4);
            fileYear = atol(fyear);
            strncpy(fmonth, f->fts_name + 23, 2);
            fileMonth = atol(fmonth);
            strncpy(fday, f->fts_name + 25, 2);
            fileDay = atol(fday);
            strncpy(version, f->fts_name + 51, 4);
            fileVersion = atol(version);
            if (fileYear == year && fileMonth == month && fileDay == day && fileVersion > lastVersion)
            {
                lastVersion = fileVersion;
                sprintf(filename, "%s", f->fts_path);
                gotHmFile = true;
            }
		}
		f = fts_read(fts);
	}

	fts_close(fts);

    if (gotHmFile)
    {
        return UTIL_NO_ERROR;
    }
    else
    {
        return UTIL_ERR_HM_FILENAME;
    }

}

// Calculates day of year: 1 January is day 1.
int dayOfYear(long year, long month, long day, int* yday)
{
    time_t date;
    struct tm dateStruct;
    dateStruct.tm_year = year - 1900;
    dateStruct.tm_mon = month - 1;
    dateStruct.tm_mday = day;
    dateStruct.tm_hour = 0;
    dateStruct.tm_min = 0;
    dateStruct.tm_sec = 0;
    dateStruct.tm_yday = 0;
    date = timegm(&dateStruct);
    struct tm *dateStructUpdated = gmtime(&date);
    if (dateStructUpdated == NULL)
    {
        fprintf(stdout, "%sUnable to get day of year from specified date.\n", infoHeader);
        *yday = 0;
        return UTIL_ERR_DAY_OF_YEAR_CONVERSION;
    }
    *yday = dateStructUpdated->tm_yday + 1;
    return UTIL_NO_ERROR;
}

int dateAdjust(int *year, int *month, int *day, int deltaDays)
{
    if (year == NULL || month == NULL || day == NULL)
        return UTIL_ERR_DATE_ADJUST;

    time_t date;
    struct tm dateStruct;
    dateStruct.tm_year = *year - 1900;
    dateStruct.tm_mon = *month - 1;
    dateStruct.tm_mday = *day + deltaDays;
    dateStruct.tm_hour = 0;
    dateStruct.tm_min = 0;
    dateStruct.tm_sec = 0;
    date = timegm(&dateStruct);
    struct tm *dateStructUpdated = gmtime(&date);
    if (dateStructUpdated == NULL)
    {
        fprintf(stdout, "%sUnable to update date from specified date.\n", infoHeader);
        return UTIL_ERR_DAY_OF_YEAR_CONVERSION;
    }
    *year = dateStructUpdated->tm_year + 1900;
    *month = dateStructUpdated->tm_mon + 1;
    *day = dateStructUpdated->tm_mday;
}

void utcDateString(time_t seconds, char *dateString)
{
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);

    return;
}

void utcNowDateString(char *dateString)
{
    time_t seconds = time(NULL);
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);

    return;
}

void utcDateStringWithMicroseconds(double exactSeconds, char *dateString)
{
    time_t seconds = (time_t) floor(exactSeconds);
    int microseconds = (int) floor(1000000.0 * (exactSeconds - (double) seconds));
    struct tm *d = gmtime(&seconds);
    sprintf(dateString, "UTC=%04d-%02d-%02dT%02d:%02d:%02d.%06d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec, microseconds);

    return;

}
