/*

    TRACIS Processor: tools/tracis/export_products.c

    Copyright (C) 2023  Johnathan K Burchill

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


#include "export_products.h"

#include "tracis.h"
#include "tracis_settings.h"
#include "cdf_vars.h"
#include "cdf_attrs.h"

#include <libxml/xmlwriter.h>
#include <sys/stat.h>
#include <math.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cdf.h>


extern char infoHeader[50];

CDFstatus exportTracisCdfLR(const char *cdfFilename, const char satellite, const char *exportVersion, ImageStorage *store, size_t numberOfImagePairs, Ephemeres *ephem, char *efiFilenames, size_t nEfiFiles)
{

    CDFid exportCdfId;
    CDFstatus status = CDF_OK;
    status = CDFcreateCDF((char *)cdfFilename, &exportCdfId);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    // export variables
    createVarFrom1DVar(exportCdfId, "Timestamp", CDF_EPOCH, 0, numberOfImagePairs-1, store->imageTimes, true);
    createVarFrom1DVar(exportCdfId, "Latitude", CDF_REAL8, 0, numberOfImagePairs-1, ephem->Latitude, true);
    createVarFrom1DVar(exportCdfId, "Longitude", CDF_REAL8, 0, numberOfImagePairs-1, ephem->Longitude, true);
    createVarFrom1DVar(exportCdfId, "Radius", CDF_REAL8, 0, numberOfImagePairs-1, ephem->Radius, true);
    createVarFromImage(exportCdfId, "Raw_image_H", CDF_UINT2, 0, numberOfImagePairs-1, store->rawImagesH, true);
    createVarFromImage(exportCdfId, "Raw_image_V", CDF_UINT2, 0, numberOfImagePairs-1, store->rawImagesV, true);
    createVarFromImage(exportCdfId, "Processed_image_H", CDF_UINT2, 0, numberOfImagePairs-1, store->correctedImagesH, true);
    createVarFromImage(exportCdfId, "Processed_image_V", CDF_UINT2, 0, numberOfImagePairs-1, store->correctedImagesV, true);

    createVarFrom1DVar(exportCdfId, "Valid_imagery_H", CDF_UINT1, 0, numberOfImagePairs-1, store->validImageryH, true);
    createVarFrom1DVar(exportCdfId, "Valid_imagery_V", CDF_UINT1, 0, numberOfImagePairs-1, store->validImageryV, true);
    createVarFrom1DVar(exportCdfId, "TII_imaging_mode", CDF_UINT1, 0, numberOfImagePairs-1, store->imagingMode, true);

    createVarFrom1DVar(exportCdfId, "Image_anomaly_flags_H", CDF_UINT1, 0, numberOfImagePairs-1, store->anomalyFlagH, true);
    createVarFrom1DVar(exportCdfId, "Image_anomaly_flags_V", CDF_UINT1, 0, numberOfImagePairs-1, store->anomalyFlagV, true);

    createVarFrom1DVar(exportCdfId, "CCD_dark_current_H", CDF_UINT2, 0, numberOfImagePairs-1, store->ccdDarkCurrentH, true);
    createVarFrom1DVar(exportCdfId, "CCD_dark_current_V", CDF_UINT2, 0, numberOfImagePairs-1, store->ccdDarkCurrentV, true);
    createVarFrom1DVar(exportCdfId, "CCD_temperature_H", CDF_REAL4, 0, numberOfImagePairs-1, store->ccdTemperatureH, true);
    createVarFrom1DVar(exportCdfId, "CCD_temperature_V", CDF_REAL4, 0, numberOfImagePairs-1, store->ccdTemperatureV, true);
    createVarFrom1DVar(exportCdfId, "V_MCP_H", CDF_REAL4, 0, numberOfImagePairs-1, store->VMcpH, true);
    createVarFrom1DVar(exportCdfId, "V_MCP_V", CDF_REAL4, 0, numberOfImagePairs-1, store->VMcpV, true);
    createVarFrom1DVar(exportCdfId, "V_Phos_H", CDF_REAL4, 0, numberOfImagePairs-1, store->VPhosH, true);
    createVarFrom1DVar(exportCdfId, "V_Phos_V", CDF_REAL4, 0, numberOfImagePairs-1, store->VPhosV, true);
    createVarFrom1DVar(exportCdfId, "V_Bias_H", CDF_REAL4, 0, numberOfImagePairs-1, store->VBiasH, true);
    createVarFrom1DVar(exportCdfId, "V_Bias_V", CDF_REAL4, 0, numberOfImagePairs-1, store->VBiasV, true);
    createVarFrom1DVar(exportCdfId, "V_Faceplate", CDF_REAL4, 0, numberOfImagePairs-1, store->VFaceplate, true);
    createVarFrom1DVar(exportCdfId, "Shutter_duty_cycle_H", CDF_REAL4, 0, numberOfImagePairs-1, store->ShutterDutyCycleH, true);
    createVarFrom1DVar(exportCdfId, "Shutter_duty_cycle_V", CDF_REAL4, 0, numberOfImagePairs-1, store->ShutterDutyCycleV, true);

    createVarFromImage(exportCdfId, "Energy_map_H", CDF_REAL4, 0, numberOfImagePairs-1, store->energyMapH, true);
    createVarFromImage(exportCdfId, "Energy_map_V", CDF_REAL4, 0, numberOfImagePairs-1, store->energyMapV, true);

    createVarFromImage(exportCdfId, "Angle_of_arrival_map_H", CDF_REAL4, 0, numberOfImagePairs-1, store->angleOfArrivalMapH, true);
    createVarFromImage(exportCdfId, "Angle_of_arrival_map_V", CDF_REAL4, 0, numberOfImagePairs-1, store->angleOfArrivalMapV, true);

    createVarFrom2DVar(exportCdfId, "Energy_spectrum_H", CDF_REAL4, 0, numberOfImagePairs-1, store->energySpectrumH, ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Energy_spectrum_V", CDF_REAL4, 0, numberOfImagePairs-1, store->energySpectrumV, ENERGY_BINS, true);

    createVarFrom2DVar(exportCdfId, "Angle_of_arrival_spectrum_H", CDF_REAL4, 0, numberOfImagePairs-1, store->angleOfArrivalSpectrumH, ANGULAR_BINS, true);
    createVarFrom2DVar(exportCdfId, "Angle_of_arrival_spectrum_V", CDF_REAL4, 0, numberOfImagePairs-1, store->angleOfArrivalSpectrumV, ANGULAR_BINS, true);

    createVarFrom2DVar(exportCdfId, "Raw_energy_spectrum_H", CDF_REAL4, 0, numberOfImagePairs-1, store->rawEnergySpectrumH, ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Raw_energy_spectrum_V", CDF_REAL4, 0, numberOfImagePairs-1, store->rawEnergySpectrumV, ENERGY_BINS, true);

    createVarFrom2DVar(exportCdfId, "Raw_angle_of_arrival_spectrum_H", CDF_REAL4, 0, numberOfImagePairs-1, store->rawAngleOfArrivalSpectrumH, ANGULAR_BINS, true);
    createVarFrom2DVar(exportCdfId, "Raw_angle_of_arrival_spectrum_V", CDF_REAL4, 0, numberOfImagePairs-1, store->rawAngleOfArrivalSpectrumV, ANGULAR_BINS, true);

    createVarFrom2DVar(exportCdfId, "Energies_H", CDF_REAL4, 0, numberOfImagePairs-1, store->energiesH, ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Energies_V", CDF_REAL4, 0, numberOfImagePairs-1, store->energiesV, ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Angles_of_arrival", CDF_REAL4, 0, numberOfImagePairs-1, store->anglesOfArrival, ANGULAR_BINS, true);

    double minTime = store->imageTimes[0];
    double maxTime = store->imageTimes[numberOfImagePairs-1];

    addAttributesLR(exportCdfId, cdfFilename, efiFilenames, nEfiFiles, SOFTWARE_VERSION_STRING " " SOFTWARE_VERSION, satellite, exportVersion, minTime, maxTime);

    closeCdf(exportCdfId);
    status = archiveFiles(cdfFilename);
    if (status == EXPORT_OK)
        fprintf(stdout, "%sArchived %ld image records in CDF file in %s.ZIP\n", infoHeader, numberOfImagePairs, cdfFilename);
    else
        fprintf(stdout, "%sUnable to archive CDF file (check your zip program)\n", infoHeader);

    fflush(stdout);

    return status;

}

CDFstatus exportTracisCdfHR(const char *cdfFilename, const char satellite, const char *exportVersion, ImageStorage *store, size_t numberOfColumnSums, Ephemeres *ephem, char *efiFilenames, size_t nEfiFiles)
{

    CDFid exportCdfId;
    CDFstatus status = CDF_OK;
    status = CDFcreateCDF((char *)cdfFilename, &exportCdfId);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    // export variables
    createVarFrom1DVar(exportCdfId, "Timestamp", CDF_EPOCH, 0, numberOfColumnSums-1, store->colSumTimes, true);
    createVarFrom1DVar(exportCdfId, "Latitude", CDF_REAL8, 0, numberOfColumnSums-1, ephem->Latitude, true);
    createVarFrom1DVar(exportCdfId, "Longitude", CDF_REAL8, 0, numberOfColumnSums-1, ephem->Longitude, true);
    createVarFrom1DVar(exportCdfId, "Radius", CDF_REAL8, 0, numberOfColumnSums-1, ephem->Radius, true);

    createVarFrom1DVar(exportCdfId, "TII_imaging_mode", CDF_UINT1, 0, numberOfColumnSums-1, store->colSumImagingMode, true);

    createVarFrom1DVar(exportCdfId, "V_MCP_Setting_H", CDF_REAL4, 0, numberOfColumnSums-1, store->mcpVoltageSettingH, true);
    createVarFrom1DVar(exportCdfId, "V_MCP_Setting_V", CDF_REAL4, 0, numberOfColumnSums-1, store->mcpVoltageSettingV, true);
    createVarFrom1DVar(exportCdfId, "V_Phos_Setting_H", CDF_REAL4, 0, numberOfColumnSums-1, store->phosphorVoltageSettingH, true);
    createVarFrom1DVar(exportCdfId, "V_Phos_Setting_V", CDF_REAL4, 0, numberOfColumnSums-1, store->phosphorVoltageSettingV, true);
    createVarFrom1DVar(exportCdfId, "V_Bias_Setting_H", CDF_REAL4, 0, numberOfColumnSums-1, store->biasGridVoltageSettingH, true);
    createVarFrom1DVar(exportCdfId, "V_Bias_Setting_V", CDF_REAL4, 0, numberOfColumnSums-1, store->biasGridVoltageSettingV, true);

    createVarFrom2DVar(exportCdfId, "Column_sum_spectrum_H", CDF_UINT2, 0, numberOfColumnSums-1, store->colSumSpectrumH, COLUMN_SUM_ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Column_sum_spectrum_V", CDF_UINT2, 0, numberOfColumnSums-1, store->colSumSpectrumV, COLUMN_SUM_ENERGY_BINS, true);

    createVarFrom2DVar(exportCdfId, "Column_sum_energies_H", CDF_REAL4, 0, numberOfColumnSums-1, store->colSumEnergiesH, COLUMN_SUM_ENERGY_BINS, true);
    createVarFrom2DVar(exportCdfId, "Column_sum_energies_V", CDF_REAL4, 0, numberOfColumnSums-1, store->colSumEnergiesV, COLUMN_SUM_ENERGY_BINS, true);


    double minTime = store->colSumTimes[0];
    double maxTime = store->colSumTimes[numberOfColumnSums-1];

    addAttributesHR(exportCdfId, cdfFilename, efiFilenames, nEfiFiles, SOFTWARE_VERSION_STRING " " SOFTWARE_VERSION, satellite, exportVersion, minTime, maxTime);

    closeCdf(exportCdfId);
    status = archiveFiles(cdfFilename);
    if (status == EXPORT_OK)
        fprintf(stdout, "%sArchived %ld column sum records in CDF file in %s.ZIP\n", infoHeader, numberOfColumnSums, cdfFilename);
    else
        fprintf(stdout, "%sUnable to archive CDF file (check your zip program)\n", infoHeader);

        fflush(stdout);

    return status;

}

int exportProducts(char satellite, ImageStorage *store, size_t numberOfLRRecords, Ephemeres *imageEphem, char *tracisLRFilename, size_t numberOfHRRecords, Ephemeres *colSumEphem, char *tracisHRFilename, char *efiFilenames, size_t nEfiFiles, time_t processingStartTime)
{
    int status = EXPORT_OK;

    time_t processingStopTime = time(NULL);

    // Low res dataset
    double firstMeasurementTimeLR = store->imageTimes[0];
    double lastMeasurementTimeLR = store->imageTimes[numberOfLRRecords-1];
    double startLR = 0;
    double endLR = 0;
    EPOCHtoUnixTime(&firstMeasurementTimeLR, &startLR, 1);
    EPOCHtoUnixTime(&lastMeasurementTimeLR, &endLR, 1);

    // Write archived CDF files
    status = exportTracisCdfLR(tracisLRFilename, satellite, EXPORT_VERSION_STRING, store, numberOfLRRecords, imageEphem, efiFilenames, nEfiFiles);

    if (status != EXPORT_OK)
    {
        fprintf(stdout, "%sLR CDF or ZIP export failed. Exiting.\n", infoHeader);
        return EXPORT_CDF;
    }

    double firstMeasurementTimeHR = store->colSumTimes[0];
    double lastMeasurementTimeHR = store->colSumTimes[numberOfHRRecords-1];
    double startHR = 0;
    double endHR = 0;
    EPOCHtoUnixTime(&firstMeasurementTimeHR, &startHR, 1);
    EPOCHtoUnixTime(&lastMeasurementTimeHR, &endHR, 1);

    status = exportTracisCdfHR(tracisHRFilename, satellite, EXPORT_VERSION_STRING, store, numberOfHRRecords, colSumEphem, efiFilenames, nEfiFiles);

    if (status != EXPORT_OK)
    {
        fprintf(stdout, "%sHR CDF or ZIP export failed. Exiting.\n", infoHeader);
        return EXPORT_CDF;
    }

    return status;
}

int archiveFiles(const char *filenameBase)
{
    int sysStatus = system(NULL);
    if (sysStatus == 0)
    {
        fprintf(stderr, "%sSystem shell call not available. Not archiving CDF.\n", infoHeader);
        return EXPORT_ZIP;
    }

    sysStatus = system("zip -q 1 > /dev/null");
    if (WIFEXITED(sysStatus) && WEXITSTATUS(sysStatus) == 12)
    {
        char command[3*FILENAME_MAX + 100];
        sprintf(command, "zip -Z store -q -r -j %s.ZIP %s.cdf && rm %s.cdf", filenameBase, filenameBase, filenameBase);
        sysStatus = system(command);
        if (!(WIFEXITED(sysStatus) && (WEXITSTATUS(sysStatus) == 0)))
            fprintf(stderr, "%sFailed to archive CDF file.\n", infoHeader);
    }
    else
    {
        fprintf(stderr, "zip program is unusable. Not exporting %s.\n", filenameBase);
        return EXPORT_ZIP;
    }

    return EXPORT_OK;

}