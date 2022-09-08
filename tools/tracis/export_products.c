/*

    TRACIS Processor: tools/tracis/export_products.c

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

// #include <zip.h>
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
        goto cleanup;
    }
    else
    {
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

        fprintf(stdout, "%sExported %ld image records to %s.cdf\n", infoHeader, numberOfImagePairs, cdfFilename);
        fflush(stdout);
        status = CDF_OK;

    }

cleanup:
    closeCdf(exportCdfId);
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
        goto cleanup;
    }
    else
    {
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

        fprintf(stdout, "%sExported %ld column sum records to %s.cdf\n", infoHeader, numberOfColumnSums, cdfFilename);
        fflush(stdout);
        status = CDF_OK;

    }

cleanup:
    closeCdf(exportCdfId);
    return status;

}

int exportTracisHeader(const char *tracisFilename, const char *efiFilenames, size_t nFiles, time_t processingStartTime, time_t validityStartTime, time_t validityStopTime, bool lowRes)
{

    // Level 2 product ZIP file neads a HDR file.
    size_t sLen = strlen(tracisFilename);
    if (sLen < TRACIS_BASE_FILENAME_LENGTH)
        return HEADER_FILENAME;

    char tracisCdfFilename[FILENAME_MAX];
    snprintf(tracisCdfFilename, sLen + 5, "%s.cdf", tracisFilename);
    char sizeString[22];
    struct stat fileInfo;
    int statstatus = stat(tracisCdfFilename, &fileInfo);
    if (statstatus == -1)
    {
        perror(NULL);
        return HEADER_CDFFILEINFO;
    }
    sprintf(sizeString, "%+021d", (int)fileInfo.st_size);

    char headerFilename[FILENAME_MAX];
    snprintf(headerFilename, sLen + 5, "%s.HDR", tracisFilename);

    char creationDate[UTC_DATE_LENGTH];
    utcDateString(processingStartTime, creationDate);
    char nowDate[UTC_DATE_LENGTH];
    utcNowDateString(nowDate);

    char validityStart[UTC_DATE_LENGTH];
    utcDateString(validityStartTime, validityStart);
    char validityStop[UTC_DATE_LENGTH];
    utcDateString(validityStopTime, validityStop);

    char sensingStart[UTC_DATE_LENGTH + 7];
    double firstMeasurementTimeUnix = (double) validityStartTime;
    utcDateStringWithMicroseconds(firstMeasurementTimeUnix, sensingStart);
    double lastMeasurementTimeUnix = (double) validityStopTime;
    char sensingStop[UTC_DATE_LENGTH + 7];
    utcDateStringWithMicroseconds(lastMeasurementTimeUnix, sensingStop);

    // CRC of all records in CDF file (Table 5-1 entry 1.21 of L1b product specification \cite{esal1bspec}).
    // In contrast with a DBL file, this is not computed for CDF
    // which contains additional info besides record values)
    char crcString[7];
    sprintf(crcString, "%+06d", -1);

    int status = HEADER_OK;
    int bytes = 0;

    xmlTextWriterPtr hdr = xmlNewTextWriterFilename(headerFilename, 0);
    if (hdr == NULL)
        return HEADER_CREATE;

    bytes = xmlTextWriterStartDocument(hdr, "1.0", "UTF-8", "no");
    if (bytes == -1)
    {
        status = HEADER_START;
        goto cleanup;
    }
    xmlTextWriterSetIndent(hdr, 1);
    xmlTextWriterSetIndentString(hdr, "  ");

    bytes = xmlTextWriterStartElement(hdr, "Earth_Explorer_Header");
    if (bytes == -1)
    {
        status = HEADER_WRITE_ERROR;
        goto cleanup;
    }
    xmlTextWriterWriteAttribute(hdr, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");


        // <Fixed_Header>
        xmlTextWriterStartElement(hdr, "Fixed_Header");

            xmlTextWriterWriteElement(hdr, "FileName", tracisFilename + sLen - TRACIS_BASE_FILENAME_LENGTH);

            if (lowRes)
                xmlTextWriterWriteElement(hdr, "File_Description", "Swarm TII Raw And Corrected Imagery / Spectra (TRACIS) low resolution (TII imagery) product.");
            else
                xmlTextWriterWriteElement(hdr, "File_Description", "Swarm TII Raw And Corrected Imagery / Spectra (TRACIS) high-resolution (2 Hz) product.");

            xmlTextWriterStartElement(hdr, "Notes");
            xmlTextWriterEndElement(hdr);

            xmlTextWriterWriteElement(hdr, "Mission", "Swarm");

            xmlTextWriterWriteElement(hdr, "File_Class", TRACIS_PRODUCT_TYPE);
            if (lowRes)
                xmlTextWriterWriteElement(hdr, "File_Type", TRACIS_FILE_TYPE_LR);
            else
                xmlTextWriterWriteElement(hdr, "File_Type", TRACIS_FILE_TYPE_HR);
            xmlTextWriterStartElement(hdr, "Validity_Period");
                xmlTextWriterWriteElement(hdr, "Validity_Start", validityStart);
                xmlTextWriterWriteElement(hdr, "Validity_Stop", validityStop);
            xmlTextWriterEndElement(hdr);

            xmlTextWriterWriteElement(hdr, "File_Version", EXPORT_VERSION_STRING);

            xmlTextWriterStartElement(hdr, "Source");

                xmlTextWriterWriteElement(hdr, "System", "SPC");
                xmlTextWriterWriteElement(hdr, "Creator", "SPC_UOC");
                xmlTextWriterWriteElement(hdr, "Creator_Version", SOFTWARE_VERSION);
                xmlTextWriterWriteElement(hdr, "Creation_Date", creationDate);

            xmlTextWriterEndElement(hdr);


        // </Fixed_Header>
        xmlTextWriterEndElement(hdr);


        // <Variable_Header>
        xmlTextWriterStartElement(hdr, "Variable_Header");

            xmlTextWriterStartElement(hdr, "MPH");
                xmlTextWriterWriteElement(hdr, "Product", tracisFilename + sLen - TRACIS_BASE_FILENAME_LENGTH);
                xmlTextWriterWriteElement(hdr, "Product_Format", "CDF");
                xmlTextWriterWriteElement(hdr, "Proc_Stage_Code", TRACIS_PRODUCT_TYPE);
                xmlTextWriterWriteElement(hdr, "Ref_Doc", "SW-TN-UoC-GS-001_TRACIS_Product_Definition");
                xmlTextWriterWriteElement(hdr, "Proc_Center", "UOC");
                xmlTextWriterWriteElement(hdr, "Proc_Time", creationDate);
                xmlTextWriterWriteElement(hdr, "Software_Version", "UOC_TRACIS/" SOFTWARE_VERSION);
                xmlTextWriterWriteElement(hdr, "Product_Err", "0");

                xmlTextWriterStartElement(hdr, "Tot_Size");
                    xmlTextWriterWriteAttribute(hdr, "unit", "bytes");
                    xmlTextWriterWriteString(hdr, sizeString);
                xmlTextWriterEndElement(hdr);
                xmlTextWriterWriteElement(hdr, "CRC", crcString);                

            xmlTextWriterEndElement(hdr);


            xmlTextWriterStartElement(hdr, "SPH");
                if (lowRes)
                    xmlTextWriterWriteElement(hdr, "SPH_Descriptor", TRACIS_FILE_TYPE_LR);                
                else
                    xmlTextWriterWriteElement(hdr, "SPH_Descriptor", TRACIS_FILE_TYPE_HR);                
                xmlTextWriterWriteElement(hdr, "Original_Filename", tracisCdfFilename + sLen - TRACIS_BASE_FILENAME_LENGTH);

            xmlTextWriterStartElement(hdr, "Sensing_Time_Interval");
                xmlTextWriterWriteElement(hdr, "Sensing_Start", sensingStart);
                xmlTextWriterWriteElement(hdr, "Sensing_Stop", sensingStop);
            xmlTextWriterEndElement(hdr);

            xmlTextWriterStartElement(hdr, "Product_Confidence_Data");
                xmlTextWriterWriteElement(hdr, "Quality_Indicator", "000");
            xmlTextWriterEndElement(hdr);

            xmlTextWriterStartElement(hdr, "List_of_Input_File_Names");
                char nInputFilesStr[20];
                sprintf(nInputFilesStr, "%ld", nFiles);
                xmlTextWriterWriteAttribute(hdr, "count", nInputFilesStr);
                for (int i = 0; i < nFiles; i++)
                {
                    size_t fLen = strlen(efiFilenames + i * FILENAME_MAX);
                    xmlTextWriterWriteElement(hdr, "File_Name", efiFilenames + i * FILENAME_MAX + fLen - 59);
                }
            xmlTextWriterEndElement(hdr);

            xmlTextWriterStartElement(hdr, "List_of_Output_File_Names");

                xmlTextWriterWriteAttribute(hdr, "count", "1");
                xmlTextWriterWriteElement(hdr, "File_Name", tracisCdfFilename + sLen - TRACIS_BASE_FILENAME_LENGTH);

            xmlTextWriterEndElement(hdr);


            xmlTextWriterEndElement(hdr);

        // </Variable_Header>
        xmlTextWriterEndElement(hdr);

    // </Earth_Explorer_Header>
    xmlTextWriterEndElement(hdr);

    xmlTextWriterEndDocument(hdr);
    bytes = xmlTextWriterFlush(hdr);

cleanup:
    xmlFreeTextWriter(hdr);

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

    // Write CDF files
    status = exportTracisCdfLR(tracisLRFilename, satellite, EXPORT_VERSION_STRING, store, numberOfLRRecords, imageEphem, efiFilenames, nEfiFiles);

    if (status != CDF_OK)
    {
        fprintf(stdout, "%sLR CDF export failed. Exiting.\n", infoHeader);
        return EXPORT_CDF;
    }

    double firstMeasurementTimeHR = store->colSumTimes[0];
    double lastMeasurementTimeHR = store->colSumTimes[numberOfHRRecords-1];
    double startHR = 0;
    double endHR = 0;
    EPOCHtoUnixTime(&firstMeasurementTimeHR, &startHR, 1);
    EPOCHtoUnixTime(&lastMeasurementTimeHR, &endHR, 1);

    status = exportTracisCdfHR(tracisHRFilename, satellite, EXPORT_VERSION_STRING, store, numberOfHRRecords, colSumEphem, efiFilenames, nEfiFiles);

    if (status != CDF_OK)
    {
        fprintf(stdout, "%sHR CDF export failed. Exiting.\n", infoHeader);
        return EXPORT_CDF;
    }

    // From SLIDEM main.c:
    // Write Header file for L2 archiving

    // status = exportTracisHeader(tracisLRFilename, efiFilenames, nEfiFiles, processingStartTime, (time_t) floor(startLR), (time_t) floor(endLR), true);

    // if (status != HEADER_OK)
    // {
    //     fprintf(stdout, "%sError writing LR HDR file.\n", infoHeader);
    //     return -1;
    // }

    // status = exportTracisHeader(tracisHRFilename, efiFilenames, nEfiFiles, processingStartTime, (time_t) floor(startHR), (time_t) floor(endHR), false);

    // if (status != HEADER_OK)
    // {
    //     fprintf(stdout, "%sError writing HR HDR file.\n", infoHeader);
    //     return EXPORT_HDR;
    // }

    // Archive the CDF and HDR files as ZIP files
    // status = archiveFiles(tracisLRFilename);
    // if (status != EXPORT_OK)
    //     return status;
    // status = archiveFiles(tracisHRFilename);

    return status;
}

int archiveFiles(char *filenameBase)
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
        char command[5*FILENAME_MAX + 100];
        sprintf(command, "zip -Z store -q -r -j %s.ZIP %s.HDR %s.cdf && rm %s.HDR %s.cdf", filenameBase, filenameBase, filenameBase, filenameBase, filenameBase);
        sysStatus = system(command);
        if (WIFEXITED(sysStatus) && (WEXITSTATUS(sysStatus) == 0))
        {
            fprintf(stdout, "%sArchived data in %s.ZIP\n", infoHeader, filenameBase);
        }
        else
        {
            fprintf(stderr, "%sFailed to archive HDR and CDF files.\n", infoHeader);
        }
    }
    else
    {
        fprintf(stderr, "zip is unusable. Not exporting %s.\n", filenameBase);
        return EXPORT_ZIP;
    }

    return EXPORT_OK;

}