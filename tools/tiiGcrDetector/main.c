/*

    TRACIS: tools/tiiGcrDetector/main.c

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

#include "indexing.h"

#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <fts.h>

#include <cdf.h>

#include <gsl/gsl_statistics_short.h>


#define SOFTWARE_VERSION "1.0.0"
#define STATUS_INTERVAL_FRACTION 0.1
#define NUM_CDF_VARS 14

#define IMAGE_PIXELS 2640
#define GCR_BINNED_THRESHOLD 5000
#define MAX_PIXEL_VALUE 2000
#define GCR_SIGMAS 5
#define MINCOL 19
#define MINROW 19
#define MAXROW 46
#define MINROW2 4
#define MAXROW2 61


enum ProcessData
{
    PROCESS_DATA_OK = 0,
    PROCESS_DATA_MEM
};

int loadData(const char * filename, uint8_t **dataBuffers, long *numberOfRecords);
void printErrorMessage(CDFstatus status);

int processData(uint8_t **dataBuffers, long nRecs);
void processDataBinned(uint8_t **dataBuffers, long nRecs);

int alphabeticalFts(const FTSENT **a, const FTSENT **b)
{
    if (a == NULL || *a == NULL | b == NULL || *b == NULL)
        return 0;
    return strcmp((*a)->fts_name, (*b)->fts_name);
}

int main(int argc, char* argv[])
{
    time_t currentTime;
    struct tm * timeParts;
    time(&currentTime);
    timeParts = localtime(&currentTime);
    char * dateString = asctime(timeParts);
    char date[255];
    snprintf(date, strlen(dateString), "%s", dateString);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0)
        {
            fprintf(stdout, "tiiGcrDetector - prints time, latitude, longitude, altitude, sensor and location in image of detected GCRs in ready state (HV off), version %s.\n", SOFTWARE_VERSION);
            fprintf(stdout, "Copyright (C) 2022  Johnathan K Burchill\n");
            fprintf(stdout, "This program comes with ABSOLUTELY NO WARRANTY.\n");
            fprintf(stdout, "This is free software, and you are welcome to redistribute it\n");
            fprintf(stdout, "under the terms of the GNU General Public License.\n");

            exit(0);
        }
    }


    if (argc != 4)
    {
        fprintf(stderr, "usage: %s directory satelliteLetter datasetVersion\n", argv[0]);
        exit(1);
    }
    const char *directory = argv[1];
    const char *satelliteLetter = argv[2];
    const char *version = argv[3];

    // Count number of files in directory:
    long nFiles = 0;

    char *searchPath[2] = {NULL, NULL};
    searchPath[0] = (char *)directory;
    FTS * fts = fts_open(searchPath, FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT, &alphabeticalFts);     
    if (fts == NULL)
    {
        fprintf(stderr, "Could not open directory %s for reading.", directory);
            exit(EXIT_FAILURE);
    }
    FTSENT * f = fts_read(fts);
    while(f != NULL)
    {
        if ((strlen(f->fts_name) == 59 && *(f->fts_name+11) == satelliteLetter[0] && strncmp(f->fts_name+12, "TISL1B", 6) == 0 && strncmp(f->fts_name + 51, version, 4) == 0))
        {
            nFiles++;
        }
        f = fts_read(fts);
    }
    if (nFiles == 0)
    {
        fprintf(stderr, "Swarm %s: no TRACIS TISL1B files found.\n", satelliteLetter);
        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "found %ld files\n", nFiles);
    // Reopen file listing for processing
    fts_close(fts);
    fts = fts_open(searchPath, FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT, &alphabeticalFts);     
    if (fts == NULL)
    {
        fprintf(stderr, "Could not open directory %s for reading.", directory);
            exit(EXIT_FAILURE);
    }
    f = fts_read(fts);
    long processedFiles = 0;
    long statusInterval = (long) (STATUS_INTERVAL_FRACTION * (double) nFiles);

    CDFstatus status;
    long nRecs = 0;
    char fullPath[CDF_PATHNAME_LEN] = {0};

    uint8_t * dataBuffers[NUM_CDF_VARS];
    while(f != NULL)
    {
        if ((strlen(f->fts_name) == 59 && *(f->fts_name+11) == satelliteLetter[0] && strncmp(f->fts_name+12, "TISL1B", 6) == 0 && strncmp(f->fts_name + 51, version, 4) == 0))
        {
            // The memory pointers
            for (uint8_t i = 0; i < NUM_CDF_VARS; i++)
            {
                dataBuffers[i] = NULL;
            }
            nRecs = 0;
            status = loadData(f->fts_path, dataBuffers, &nRecs);
            if (nRecs > 0)
            {
                // fprintf(stderr, "Processing %s\n", f->fts_name);
                processData(dataBuffers, nRecs);
                fflush(stdout);
                fflush(stderr);
            }

            // free memory
            for (uint8_t i = 0; i < NUM_CDF_VARS; i++)
            {
                free(dataBuffers[i]);
            }

            processedFiles++;
            if (processedFiles % statusInterval == 0)
            {
                fprintf(stderr, "Processed %ld files (%.1f%%)\n", processedFiles, (float)processedFiles / (float)nFiles * 100.0);
                fflush(stderr);
            }


        }
        f = fts_read(fts);
    }

    fts_close(fts);

}

int loadData(const char * filename, uint8_t **dataBuffers, long *numberOfRecords)
{
    char validationFileName[CDF_PATHNAME_LEN];
    snprintf(validationFileName, strlen(filename)-3, "%s", filename);

    // Open the CDF file with validation
    CDFsetValidate(VALIDATEFILEon);
    CDFid calCdfId;
    CDFstatus status;
    status = CDFopenCDF(validationFileName, &calCdfId);
    if (status != CDF_OK) 
    {
        printErrorMessage(status);
        // fprintf(stderr, "Skipping this file.\n");
        return status;
    }

    // Attributes
    long attrN;
    long entryN;
    char attrName[CDF_ATTR_NAME_LEN256+1];
    long attrScope; long maxEntry; long dataType; long numElems;

    // Check CDF info
    long numDims, decoding, encoding, majority, maxrRec, numrVars, maxzRec, numzVars, numAttrs, format;
    long dimSizes[CDF_MAX_DIMS];

    status = CDFgetFormat(calCdfId, &format);
    status = CDFgetDecoding(calCdfId, &decoding);

    status = CDFinquireCDF(calCdfId, &numDims, dimSizes, &encoding, &majority, &maxrRec, &numrVars, &maxzRec, &numzVars, &numAttrs);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        fprintf(stderr, "Problem with data file. Skipping this file.\n");
        CDFcloseCDF(calCdfId);
        return status;
    }
    long nRecs = 0, memorySize = 0;
    status = CDFgetzVarAllocRecords(calCdfId, CDFgetVarNum(calCdfId, "Timestamp"), &nRecs);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        fprintf(stderr, "Problem with data file. Skipping this file.\n");
        CDFcloseCDF(calCdfId);
        return status;
    }

    // Variables
    uint8_t nVars = NUM_CDF_VARS;
    char* variables[NUM_CDF_VARS] = {
        "Timestamp",
        "Latitude",
        "Longitude",
        "Radius",
        "Valid_imagery_H",
        "Valid_imagery_V",
        "V_MCP_H",
        "V_MCP_V",
        "V_Phos_H",
        "V_Phos_V",
        "V_Bias_H",
        "V_Bias_V",
        "Raw_image_H",
        "Raw_image_V"        
    };
    for (uint8_t i = 0; i < nVars; i++)
    {
        status = CDFconfirmzVarExistence(calCdfId, variables[i]);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            fprintf(stderr, "Error reading variable %s. Skipping this file.\n", variables[i]);
            CDFcloseCDF(calCdfId);
            return status;
        }
    }
    
    long varNum, numValues, numVarBytes;
    long numBytesPrev, numBytesToAdd, numBytesNew;

    for (uint8_t i = 0; i < nVars; i++)
    {
        varNum = CDFgetVarNum(calCdfId, variables[i]);
        if (varNum < CDF_OK)
        {
            printErrorMessage(varNum);
            fprintf(stderr, "Error reading variable ID for %s. Skipping this file.\n", variables[i]);
            CDFcloseCDF(calCdfId);
            return varNum;
        }
        status = CDFgetzVarNumDims(calCdfId, varNum, &numDims);
        status = CDFgetzVarDimSizes(calCdfId, varNum, dimSizes);
        status = CDFgetzVarDataType(calCdfId, varNum, &dataType);
        // Calculate new size of memory to allocate
        status = CDFgetDataTypeSize(dataType, &numVarBytes);
        numValues = 1;
        for (uint8_t j = 0; j < numDims; j++)
        {
            numValues *= dimSizes[j];
        }
        memorySize = numValues * nRecs * numVarBytes;
        dataBuffers[i] = (uint8_t*) malloc((size_t) memorySize);
        status = CDFgetzVarAllRecordsByVarID(calCdfId, varNum, dataBuffers[i]);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            fprintf(stderr, "Error loading data for %s. Skipping this file.\n", variables[i]);
            CDFcloseCDF(calCdfId);
            return status;
        }
    }
    // close CDF
    CDFcloseCDF(calCdfId);
    // Update number of records found
    *numberOfRecords = *numberOfRecords + nRecs;

}

void printErrorMessage(CDFstatus status)
{
    char errorMessage[CDF_STATUSTEXT_LEN + 1];
    CDFgetStatusText(status, errorMessage);
    fprintf(stderr, "%s\n", errorMessage);
}

int processData(uint8_t **dataBuffers, long nRecs)
{
    long timeIndex = 0;

    double median = 0.0;
    double mad = 0.0;
    double mean = 0.0;
    double stddev = 0.0;

    double unixTime = 0.0;
    int row = 0;
    int col = 0;

    long imageGcrCountH = 0;
    long imageGcrCountV = 0;
    long dayGcrCountH = 0;
    long dayGcrCountV = 0;

    uint16_t *medianBuffer = malloc(IMAGE_PIXELS * sizeof (uint16_t));
    double *madWork = malloc(IMAGE_PIXELS * sizeof (double));
    if (medianBuffer == NULL || madWork == NULL)
    {
        return PROCESS_DATA_MEM;
    }

    // H sensor
    for (timeIndex = 0; timeIndex < nRecs; timeIndex++)
    {
        imageGcrCountH = 0;
        if (VALID_IMAGERY_H() && VBIASH() > -1.0 && VMCPH() > -20.0 && VPHOSH() < 50.0)
        {
            // fprintf(stdout, "Image %ld\n", timeIndex);
            for (int i = 0; i < IMAGE_PIXELS; i++)
            {
                // Disregard unphysical pixel values
                if (RAW_IMAGE_H()[i] <= MAX_PIXEL_VALUE)
                    medianBuffer[i] = RAW_IMAGE_H()[i];
                else
                    medianBuffer[i] = 0;
            }
            median = gsl_stats_short_median(medianBuffer, 1, IMAGE_PIXELS);
            // medianBuffer is now sorted
            mad = gsl_stats_short_mad(medianBuffer, 1, IMAGE_PIXELS, madWork);
            mean = gsl_stats_short_mean(medianBuffer, 1, IMAGE_PIXELS);
            stddev = gsl_stats_short_sd(medianBuffer, 1, IMAGE_PIXELS);
            EPOCHtoUnixTime(TIME_ADDR(), &unixTime, 1);
            for (int i = 0; i < IMAGE_PIXELS; i++)
            {
                // if (mad > 0 && (double) RAW_IMAGE_H()[i] > (median + (GCR_SIGMAS * mad)))
                row = i % 66;
                col = i / 66;
                if (stddev > 0 && (double) RAW_IMAGE_H()[i] > (median + (GCR_SIGMAS * stddev)) && RAW_IMAGE_H()[i] <= MAX_PIXEL_VALUE && ((col >= MINCOL && row >= MINROW2 && row <= MAXROW2) || (row >= MINROW && row <= MAXROW)))
                {
                    // fprintf(stdout, "%.2lf H %ld (%.1f,%.1f,%.1f), col %d, row %d, intensity: %d, median: %.2lf, mad: %.2lf, mean: %.2lf, sd: %.2lf\n", unixTime, timeIndex+1, LAT(), LON(), RADIUS(), col+1, row+1, RAW_IMAGE_H()[i], median, mad, mean, stddev);
                    imageGcrCountH++;
                    dayGcrCountH++;
                }
            }
            if (imageGcrCountH > 0)
                fprintf(stdout, "%.1lf H image %ld: %ld hot pixels %.1f N %.1f E @ R=%.2f km\n", unixTime, timeIndex+1, imageGcrCountH, LAT(), LON(), RADIUS()/1000.0);
        }
    }
    // V sensor
    for (timeIndex = 0; timeIndex < nRecs; timeIndex++)
    {
        imageGcrCountV = 0;
        if (VALID_IMAGERY_V() && VBIASV() > -1.0 && VMCPV() > -20.0 && VPHOSV() < 50.0)
        {
            // fprintf(stdout, "Image %ld\n", timeIndex);
            for (int i = 0; i < IMAGE_PIXELS; i++)
            {
                // Disregard unphysical pixel values
                if (RAW_IMAGE_V()[i] <= MAX_PIXEL_VALUE)
                    medianBuffer[i] = RAW_IMAGE_V()[i];
                else
                    medianBuffer[i] = 0;
            }
            median = gsl_stats_short_median(medianBuffer, 1, IMAGE_PIXELS);
            // medianBuffer is now sorted
            mad = gsl_stats_short_mad(medianBuffer, 1, IMAGE_PIXELS, madWork);
            mean = gsl_stats_short_mean(medianBuffer, 1, IMAGE_PIXELS);
            stddev = gsl_stats_short_sd(medianBuffer, 1, IMAGE_PIXELS);
            EPOCHtoUnixTime(TIME_ADDR(), &unixTime, 1);
            for (int i = 0; i < IMAGE_PIXELS; i++)
            {
                // if (mad > 0 && (double) RAW_IMAGE_H()[i] > (median + (GCR_SIGMAS * mad)))
                row = i % 66;
                col = i / 66;
                if (stddev > 0 && (double) RAW_IMAGE_V()[i] > (median + (GCR_SIGMAS * stddev)) && RAW_IMAGE_V()[i] <= MAX_PIXEL_VALUE && ((col >= MINCOL && row >= MINROW2 && row <= MAXROW2) || (row >= MINROW && row <= MAXROW)))
                {
                    // fprintf(stdout, "%.2lf V %ld (%.1f,%.1f,%.1f), col %d, row %d, intensity: %d, median: %.2lf, mad: %.2lf, mean: %.2lf, sd: %.2lf\n", unixTime, timeIndex+1, LAT(), LON(), RADIUS(), col+1, row+1, RAW_IMAGE_H()[i], median, mad, mean, stddev);
                    imageGcrCountV++;
                    dayGcrCountV++;
                }
            }
            if (imageGcrCountV > 0)
                fprintf(stdout, "%.1lf V image %ld: %ld hot pixels %.1f N %.1f E @ R=%.2f km\n", unixTime, timeIndex+1, imageGcrCountV, LAT(), LON(), RADIUS()/1000.0);
        }

    }
    timeIndex = 0;
    long year, month, day, hour, minute, second, millisecond;
    EPOCHbreakdown(TIME(), &year, &month, &day, &hour, &minute, &second, &millisecond);
    if (dayGcrCountH > 0 || dayGcrCountV > 0)
        fprintf(stdout, "Processed %4ld%02ld%02ld %ld H GCRs %ld V GCRs\n", year, month, day, dayGcrCountH, dayGcrCountV);

    free(madWork);
    free(medianBuffer);
    return PROCESS_DATA_OK;
}

void processDataBinned(uint8_t **dataBuffers, long nRecs)
{
    long timeIndex = 0;

    uint16_t *image = NULL;

    int32_t binned0[11][10] = {0};
    int32_t binned1[11][10] = {0};
    int32_t diff = 0;
    bool candidateImage = false;

    for (timeIndex = 0; timeIndex < nRecs; timeIndex++)
    {
        // fprintf(stdout, "Image %ld\n", timeIndex);
        for (int i = 0; i < 11; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                binned0[i][j] = binned1[i][j];
                binned1[i][j] = 0;
            }
        }
        if (VALID_IMAGERY_H() && VBIASH() > -1.0 && VMCPH() > -20.0 && VPHOSH() < 50.0)
        {
            candidateImage = true;
            image = RAW_IMAGE_H();
            for (int i = 6; i < 61; i++)
            {
                for (int j = 4; j < 36; j++)
                {
                    binned1[i/6][j/4] += image[j*66 + i];
                }
            }
            for (int i = 0; i < 11; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    diff = binned1[i][j] - binned0[i][j];
                    if (diff >= GCR_BINNED_THRESHOLD)
                        fprintf(stdout, "Possible GCR: image %ld, (%.1f,%.1f), row %d, col %d, intensity: %d\n", timeIndex, LAT(), LON(), i, j, diff);
                }
            }
        }
        else
        {
            candidateImage = false;
        }
    }
}
