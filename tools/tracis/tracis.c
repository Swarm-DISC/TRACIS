/*

    TRACIS Processor: tools/tracis/tracis.c

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

#include "tracis.h"

#include "tracis_settings.h"
#include "tracis_flags.h"
#include "load_satellite_velocity.h"
#include "interpolate.h"
#include "utilities.h"
#include "export_products.h"
#include "image_analysis.h"

#include <tii/tii.h>

#include <tii/isp.h>
#include <tii/import.h>
#include <tii/utility.h>
#include <tii/analysis.h>
#include <tii/timeseries.h>
#include <tii/gainmap.h>
#include <tii/detector.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include <cdf.h>

char infoHeader[50];

int main(int argc, char **argv)
{

    time_t processingStartTime = time(NULL);

    if (argc != 4)
    {
        usage(argv[0]);
        exit(1);
    }

    char * satDate = argv[1];
    size_t sourceLen = strlen(satDate);

    if (sourceLen != 9)
    {
        usage(argv[0]);
	    exit(1);
    }

    char *modDir = argv[2];
    char *outputDir = argv[3];

    char satellite = satDate[0];
    int year = 0;
    int month = 0;
    int day = 0;
    sscanf(satDate+1, "%4d%2d%2d", &year, &month, &day);

    sprintf(infoHeader, "TRACIS %c%s %04d-%02d-%02d: ", satellite, EXPORT_VERSION_STRING, year, month, day);

    ImageStorage store = {0};
    initImageStorage(&store);

    char *efiFilenames = NULL;
    size_t nEfiFiles = 0;

    char tracisLRFilename[CDF_PATHNAME_LEN+1];
    char tracisLRFullFilename[CDF_PATHNAME_LEN+1+4];
    char tracisLRZipFilename[CDF_PATHNAME_LEN+1+4];
    char tracisHRFilename[CDF_PATHNAME_LEN+1];
    char tracisHRFullFilename[CDF_PATHNAME_LEN+1+4];
    char tracisHRZipFilename[CDF_PATHNAME_LEN+1+4];

    ImagePackets imagePackets = {0};
    SciencePackets sciencePackets = {0};
    LpTiiTimeSeries timeSeries = {0};

    double cdfTime = 0.0;
    Ephemeres ephem = {0};
    Ephemeres imageEphem = {0};
    Ephemeres colSumEphem = {0};
    initEphemeres(&ephem);
    initEphemeres(&imageEphem);
    initEphemeres(&colSumEphem);

    uint16_t pixelsH[NUM_FULL_IMAGE_PIXELS] = {0};
    uint16_t pixelsV[NUM_FULL_IMAGE_PIXELS] = {0};
    FullImagePacket *fip1 = NULL;
    FullImagePacket *fip2 = NULL;
    FullImageContinuedPacket *cip1 = NULL;
    FullImageContinuedPacket *cip2 = NULL;
    ImagePair imagePair = {0};
    ImageAuxData auxH = {0};
    ImageAuxData auxV = {0};

    ImageAnomalies h;
    ImageAnomalies v;

    int status = 0;

    double dayStart = 0;
    double dayEnd = 0;
    if (dateToSecondsSince1970(satDate + 1, &dayStart))
    {
        fprintf(stderr, "Could not parse %s to a date.\n", satDate);
        goto cleanup;
    }
    dayEnd = dayStart + 86400.0; // ignore leap second on this day


    // Exit if TRACIS ZIP file exists.
    if(constructExportFileNames(satellite, year, month, day, outputDir, tracisLRFilename, tracisHRFilename))
    {
        fprintf(stdout, "%sCould not construct TRACIS export filename. Exiting.\n", infoHeader);
        goto cleanup;
    }
    sprintf(tracisLRFullFilename, "%s.cdf", tracisLRFilename);
    sprintf(tracisHRZipFilename, "%s.ZIP", tracisHRFilename);
    if (access(tracisLRZipFilename, F_OK) == 0 || access(tracisHRZipFilename, F_OK) == 0)
    {
        fprintf(stdout, "One or more %sTRACIS ZIP files exist. Skipping this date.\n", infoHeader);
        goto cleanup;
    }

    // Data
    int nModFiles = 1;

    char modFilename[FILENAME_MAX];
    if (getInputFilename(satellite, year, month, day, modDir, "SC_1B", modFilename))
    {
        fprintf(stdout, "%sOPER MODx SC_1B input file is not available.\n", infoHeader);
        goto cleanup;
    }
    char modFilenamePrevious[FILENAME_MAX];
    int yearPrev = year;
    int monthPrev = month;
    int dayPrev = day;
    dateAdjust(&yearPrev, &monthPrev, &dayPrev, -1);
    if (getInputFilename(satellite, yearPrev, monthPrev, dayPrev, modDir, "SC_1B", modFilenamePrevious))
    {
        sprintf(modFilenamePrevious, "%s", "<unavailable>");
    }
    else
    {
        nModFiles = 2;
        status = loadEphemeres(modFilenamePrevious, &ephem);
    }
    status = loadEphemeres(modFilename, &ephem);
    if (status)
    {
        fprintf(stdout, "%sUnable to load satellite ephemeres.\n", infoHeader);
        goto cleanup;
    }

    efiFilenames = (char *)realloc(efiFilenames, (nEfiFiles + nModFiles)*FILENAME_MAX);
    if (efiFilenames == NULL)
    {
        fprintf(stdout, "%sOut of memory trying to store MOD filenames.\n", infoHeader);
        goto cleanup;
    }
    if (nModFiles == 2)
    {
        sprintf(efiFilenames + nEfiFiles * FILENAME_MAX, "%s", modFilenamePrevious);
        nEfiFiles++;
    }
    sprintf(efiFilenames + nEfiFiles * FILENAME_MAX, "%s", modFilename);
    nEfiFiles++;

    status = importImageryWithFilenames(satDate, &imagePackets, &efiFilenames, &nEfiFiles);
    if (status)
    {
        fprintf(stderr, "%sCould not import image data.\n", infoHeader);
        goto cleanup;
    }

    if (imagePackets.numberOfImages == 0)
    {
        fprintf(stderr, "%sNo images found for satellite %c on %s\n", infoHeader, satDate[0], satDate+1);
        goto cleanup;
    }
    
    initLpTiiTimeSeries(&timeSeries);
    importScience(satDate, &sciencePackets);
    getLpTiiTimeSeries(satDate[0], &sciencePackets, &timeSeries);

    initializeImagePair(&imagePair, &auxH, pixelsH, &auxV, pixelsV);
    getFirstImagePair(&imagePackets, &imagePair);
 
    size_t numberOfImagePairs = countImagePairs(&imagePackets, &imagePair, dayStart, dayEnd);
    size_t numberOfColumnSums = 0;
    for (size_t i = 0; i < timeSeries.n2Hz; i++)
    {
        if (!ignoreTime(timeSeries.lpTiiTime2Hz[i], dayStart, dayEnd))
            numberOfColumnSums++;
    }

    int pixelThreshold = 0;

    int imagesRead = 0;

    status = allocateImageMemory(&store, numberOfImagePairs, numberOfColumnSums);
    if (status)
    {
        printf("%sOut of memory trying to store image data.\n", infoHeader);
        goto cleanup;
    }

    size_t imageBytes = IMAGE_ROWS * IMAGE_COLS * sizeof(uint16_t);

    size_t numberOfRecords = 0;

    double *gainMapH = NULL;
    double *gainMapV = NULL;

    float radiusMapH[IMAGE_COLS*IMAGE_ROWS];
    float radiusMapV[IMAGE_COLS*IMAGE_ROWS];

    calculateRadiusMap(satellite, H_SENSOR, radiusMapH);
    calculateRadiusMap(satellite, V_SENSOR, radiusMapV);

    for (size_t i = 0; i < imagePackets.numberOfImages-1;)
    {

        status = getAlignedImagePair(&imagePackets, i, &imagePair, &imagesRead);
        if (status == ISP_NO_IMAGE_PAIR)
        {
            i++;
            continue;
        }
        i += imagesRead;

        // Skip if imagery is not within requested interval
        if (ignoreTime(imagePair.secondsSince1970, dayStart, dayEnd) || (imagePair.gotImageH == false && imagePair.gotImageV == false))
            continue;

        UnixTimetoEPOCH(&imagePair.secondsSince1970, &cdfTime, 1);
        store.imageTimes[numberOfRecords] = cdfTime;

        store.validImageryH[numberOfRecords] = imagePair.gotImageH;
        store.validImageryV[numberOfRecords] = imagePair.gotImageV;

        // Imaging mode
        store.imagingMode[numberOfRecords] = (scienceMode(imagePair.auxH) && scienceMode(imagePair.auxV));

        // Copy imagery to image time series
        memcpy(store.rawImagesH + numberOfRecords * imageBytes, imagePair.pixelsH, imageBytes);
        memcpy(store.rawImagesV + numberOfRecords * imageBytes, imagePair.pixelsV, imageBytes);

        // Raw anomaly data
        initializeAnomalyData(&h);
        initializeAnomalyData(&v);
        analyzeRawImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
        analyzeRawImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

        // Energy pixel map
        calculateEnergyMap(satellite, H_SENSOR, imagePair.auxH->BiasGridVoltageMonitor, imagePair.auxH->McpVoltageMonitor, store.energyMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS);

        calculateEnergyMap(satellite, V_SENSOR, imagePair.auxV->BiasGridVoltageMonitor, imagePair.auxV->McpVoltageMonitor, store.energyMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS);

        // Angle-of-arrival pixel map
        calculateAngleOfArrivalMap(satellite, H_SENSOR, store.angleOfArrivalMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS);

        calculateAngleOfArrivalMap(satellite, V_SENSOR, store.angleOfArrivalMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS);

        // Calculated from raw image
        // Raw energy spectrum
        energySpectrum(imagePair.pixelsH, store.energyMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapH, NULL, imagePair.auxH->BiasGridVoltageMonitor, imagePair.auxH->McpVoltageMonitor, store.rawEnergySpectrumH + numberOfRecords * ENERGY_BINS, store.energiesH + numberOfRecords * ENERGY_BINS);

        energySpectrum(imagePair.pixelsV, store.energyMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapV, NULL, imagePair.auxV->BiasGridVoltageMonitor, imagePair.auxV->McpVoltageMonitor, store.rawEnergySpectrumV + numberOfRecords * ENERGY_BINS, store.energiesV + numberOfRecords * ENERGY_BINS);

        // Raw angle-of-arrival spectrum
        angleOfArrivalSpectrum(imagePair.pixelsH, store.angleOfArrivalMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapH, NULL, store.rawAngleOfArrivalSpectrumH + numberOfRecords * ANGULAR_BINS, store.anglesOfArrival + numberOfRecords * ANGULAR_BINS);

        angleOfArrivalSpectrum(imagePair.pixelsV, store.angleOfArrivalMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapV, NULL, store.rawAngleOfArrivalSpectrumV + numberOfRecords * ANGULAR_BINS, NULL);

        // Gain corrected images and anomalies
        latestConfigValues(&imagePair, &timeSeries, &pixelThreshold, NULL, NULL, NULL, NULL, NULL, NULL);
        applyImagePairGainMaps(&imagePair, pixelThreshold, NULL, NULL);

        memcpy(store.correctedImagesH + numberOfRecords * imageBytes, imagePair.pixelsH, imageBytes);
        memcpy(store.correctedImagesV + numberOfRecords * imageBytes, imagePair.pixelsV, imageBytes);

        analyzeGainCorrectedImageAnomalies(imagePair.pixelsH, imagePair.gotImageH, imagePair.auxH->satellite, &h);
        analyzeGainCorrectedImageAnomalies(imagePair.pixelsV, imagePair.gotImageV, imagePair.auxV->satellite, &v);

        // Anomaly data
        store.anomalyFlagH[numberOfRecords] = TRACIS_FLAG_ESTIMATE_OK;
        if (h.classicWingAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_CLASSIC_WING_ANOMALY;
        if (h.peripheralAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_PERIPHERAL_ANOMALY;
        if (h.upperAngelsWingAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_UPPER_ANGELS_WING_ANOMALY;
        if (h.lowerAngelsWingAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_LOWER_ANGELS_WING_ANOMALY;
        if (h.bifurcationAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_BIFURCATION_ANOMALY;
        if (h.measlesAnomaly)
            store.anomalyFlagH[numberOfRecords] |= TRACIS_FLAG_MEASLES_ANOMALY;

        store.anomalyFlagV[numberOfRecords] = TRACIS_FLAG_ESTIMATE_OK;
        if (v.classicWingAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_CLASSIC_WING_ANOMALY;
        if (v.peripheralAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_PERIPHERAL_ANOMALY;
        if (v.upperAngelsWingAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_UPPER_ANGELS_WING_ANOMALY;
        if (v.lowerAngelsWingAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_LOWER_ANGELS_WING_ANOMALY;
        if (v.bifurcationAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_BIFURCATION_ANOMALY;
        if (v.measlesAnomaly)
            store.anomalyFlagV[numberOfRecords] |= TRACIS_FLAG_MEASLES_ANOMALY;

        // Misc data
        store.ccdDarkCurrentH[numberOfRecords] = imagePair.auxH->CcdDarkCurrent;
        store.ccdDarkCurrentV[numberOfRecords] = imagePair.auxV->CcdDarkCurrent;
        store.ccdTemperatureH[numberOfRecords] = imagePair.auxH->CcdTemperature;
        store.ccdTemperatureV[numberOfRecords] = imagePair.auxV->CcdTemperature;
        store.VMcpH[numberOfRecords] = imagePair.auxH->McpVoltageMonitor;
        store.VMcpV[numberOfRecords] = imagePair.auxV->McpVoltageMonitor;
        store.VPhosH[numberOfRecords] = imagePair.auxH->PhosphorVoltageMonitor;
        store.VPhosV[numberOfRecords] = imagePair.auxV->PhosphorVoltageMonitor;
        store.VBiasH[numberOfRecords] = imagePair.auxH->BiasGridVoltageMonitor;
        store.VBiasV[numberOfRecords] = imagePair.auxV->BiasGridVoltageMonitor;
        if (imagePair.gotImageH)
            store.VFaceplate[numberOfRecords] = imagePair.auxH->FaceplateVoltageMonitor;
        else
            store.VFaceplate[numberOfRecords] = imagePair.auxV->FaceplateVoltageMonitor;
        store.ShutterDutyCycleH[numberOfRecords] = imagePair.auxH->ShutterDutyCycle;
        store.ShutterDutyCycleV[numberOfRecords] = imagePair.auxV->ShutterDutyCycle;

        // Calculated from gain-corrected image
        gainMapH = getGainMap(imagePair.auxH->EfiInstrumentId, H_SENSOR, store.imageTimes[numberOfRecords]);
        gainMapV = getGainMap(imagePair.auxV->EfiInstrumentId, V_SENSOR, store.imageTimes[numberOfRecords]);

        // Energy spectrum
        energySpectrum(imagePair.pixelsH, store.energyMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapH, gainMapH, imagePair.auxH->BiasGridVoltageMonitor, imagePair.auxH->McpVoltageMonitor, store.energySpectrumH + numberOfRecords * ENERGY_BINS, NULL);

        energySpectrum(imagePair.pixelsV, store.energyMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapV, gainMapV, imagePair.auxV->BiasGridVoltageMonitor, imagePair.auxV->McpVoltageMonitor, store.energySpectrumV + numberOfRecords * ENERGY_BINS, NULL);

        // Angle-of-arrival spectrum
        angleOfArrivalSpectrum(imagePair.pixelsH, store.angleOfArrivalMapH + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapH, gainMapH, store.angleOfArrivalSpectrumH + numberOfRecords * ANGULAR_BINS, NULL);

        angleOfArrivalSpectrum(imagePair.pixelsV, store.angleOfArrivalMapV + numberOfRecords * IMAGE_COLS * IMAGE_ROWS, radiusMapV, gainMapV, store.angleOfArrivalSpectrumV + numberOfRecords * ANGULAR_BINS, NULL);

        numberOfRecords++;

    }

    // Column sum spectra, 2 Hz
    size_t colSumRecords = 0;
    float xcH = 0.0;
    float xcV = 0.0;
    float rH = 0.0;
    float rV = 0.0;
    float colSumEnergy = 0.0;
    detectorCoordinates(satellite, H_SENSOR, &xcH, NULL);
    detectorCoordinates(satellite, V_SENSOR, &xcV, NULL);
    size_t i = 0;
    while (ignoreTime(timeSeries.lpTiiTime2Hz[i], dayStart, dayEnd) && i < timeSeries.n2Hz)
        i++;
    size_t firstColumnInd = i;
    bool imagingMode = false;
    for (; i < timeSeries.n2Hz; i++)
    {
        if (ignoreTime(timeSeries.lpTiiTime2Hz[i], dayStart, dayEnd))
            break;

        UnixTimetoEPOCH(&timeSeries.lpTiiTime2Hz[i], &cdfTime, 1);
        store.colSumTimes[colSumRecords] = cdfTime;

        for (int j = 0; j < COLUMN_SUM_ENERGY_BINS; j++)
        {
            rH = fabs(xcH - (double)(32 - j));
            colSumEnergy = eofr(rH, timeSeries.biasGridVoltageSettingH[i], timeSeries.mcpVoltageSettingH[i]);
            if (colSumEnergy < 0.0)
                colSumEnergy = MISSING_ENERGY;
            store.colSumEnergiesH[colSumRecords * COLUMN_SUM_ENERGY_BINS + j] = colSumEnergy;

            rV = fabs(xcV - (double)(32 - j));
            colSumEnergy = eofr(rV, timeSeries.biasGridVoltageSettingV[i], timeSeries.mcpVoltageSettingV[i]);
            if (colSumEnergy < 0.0)
                colSumEnergy = MISSING_ENERGY;
            store.colSumEnergiesV[colSumRecords * COLUMN_SUM_ENERGY_BINS + j] = colSumEnergy;
        }

        store.mcpVoltageSettingH[colSumRecords] = (float) timeSeries.mcpVoltageSettingH[i];
        store.mcpVoltageSettingV[colSumRecords] = (float) timeSeries.mcpVoltageSettingV[i];
        store.phosphorVoltageSettingH[colSumRecords] = (float) timeSeries.phosphorVoltageSettingH[i];
        store.phosphorVoltageSettingV[colSumRecords] = (float) timeSeries.phosphorVoltageSettingV[i];
        store.biasGridVoltageSettingH[colSumRecords] = (float) timeSeries.biasGridVoltageSettingH[i];
        store.biasGridVoltageSettingV[colSumRecords] = (float) timeSeries.biasGridVoltageSettingV[i];

        imagingMode = store.mcpVoltageSettingH[colSumRecords] < -1000.0 && store.phosphorVoltageSettingH[colSumRecords] > 3900.0 && store.biasGridVoltageSettingH[colSumRecords] < -50.0 && store.mcpVoltageSettingV[colSumRecords] < -1000.0 && store.phosphorVoltageSettingV[colSumRecords] > 3900.0 && store.biasGridVoltageSettingV[colSumRecords] < -50.0;
        store.colSumImagingMode[colSumRecords] = imagingMode;

        colSumRecords++;

    }
    size_t lastColumnInd = i-1;
    memcpy(store.colSumSpectrumH, timeSeries.columnSumH + COLUMN_SUM_ENERGY_BINS * firstColumnInd, numberOfColumnSums * sizeof(uint16_t) * COLUMN_SUM_ENERGY_BINS);
    memcpy(store.colSumSpectrumV, timeSeries.columnSumV + COLUMN_SUM_ENERGY_BINS * firstColumnInd, numberOfColumnSums * sizeof(uint16_t) * COLUMN_SUM_ENERGY_BINS);

    // TODO Fix image times to account for delay packing by onboard processor
    // Interpolate ephemeres at image times
    status = allocEphemeres(&imageEphem, numberOfRecords);
    if (status)
    {
        printf("%sOut of memory trying to store interpolated ephemeres.\n", infoHeader);
        goto cleanup;
    }
    interpolateEphemeres(&ephem, store.imageTimes, numberOfRecords, &imageEphem);

    status = allocEphemeres(&colSumEphem, numberOfColumnSums);
    if (status)
    {
        printf("%sOut of memory trying to store interpolated ephemeres.\n", infoHeader);
        goto cleanup;
    }
    interpolateEphemeres(&ephem, store.colSumTimes, numberOfColumnSums, &colSumEphem);

    status = exportProducts(satellite, &store, numberOfRecords, &imageEphem, tracisLRFilename, numberOfColumnSums, &colSumEphem, tracisHRFilename, efiFilenames, nEfiFiles, processingStartTime);

cleanup:
    if (imagePackets.fullImagePackets != NULL) free(imagePackets.fullImagePackets);
    if (imagePackets.continuedPackets != NULL) free(imagePackets.continuedPackets);
    freeLpTiiTimeSeries(&timeSeries);

    freeImageMemory(&store);
    freeEphemeres(&ephem);
    freeEphemeres(&imageEphem);
    freeEphemeres(&colSumEphem);
    free(efiFilenames);

    fflush(stdout);

    exit(status);
}

void usage(const char * name)
{
    printf("\nTII Raw and Corrected Imagery and Spectra (TRACIS) Processor %s compiled %s %s UTC\n", TRACIS_VERSION_STRING, __DATE__, __TIME__);
    printf("\nLicense: GPL 3.0 ");
    printf("Copyright 2022 Johnathan Kerr Burchill\n");
    printf("\nUsage:\n");
    printf("\n  %s Xyyyymmdd modFileDir outputDir\n", name);
    printf("\n");
    printf("X designates the Swarm satellite (A, B or C). Must be run from directory containing EFI L0 files.\n");

    return;
}

