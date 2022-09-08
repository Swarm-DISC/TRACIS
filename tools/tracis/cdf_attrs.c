/*

    TRACIS Processor: tools/tracis/cdf_attrs.c

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

#include "cdf_attrs.h"
#include "utilities.h"
#include "tracis_settings.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

CDFstatus addgEntry(CDFid id, long attrNum, long entryNum, const char *entry)
{
    CDFstatus status = CDFputAttrgEntry(id, attrNum, entryNum, CDF_CHAR, strlen(entry), (void *)entry);
    return status;
}

CDFstatus addVariableAttributes(CDFid id, varAttr attr)
{
    CDFstatus status;
    char * variableName = attr.name;
    long varNum = CDFvarNum(id, variableName);
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "FIELDNAM"), varNum, CDF_CHAR, strlen(variableName), variableName);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "LABLAXIS"), varNum, CDF_CHAR, strlen(variableName), variableName);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VAR_TYPE"), varNum, CDF_CHAR, 4, "data");
    if (status != CDF_OK)
    {
       printErrorMessage(status);
        return status;
    }
    if (varNum != 0) // Everything but time
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TIME_BASE"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DISPLAY_TYPE"), varNum, CDF_CHAR, 11, "time_series");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DEPEND_0"), varNum, CDF_CHAR, 9, "Timestamp");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else // Add the time base to Time
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TIME_BASE"), varNum, CDF_CHAR, 3, "AD0");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DISPLAY_TYPE"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DEPEND_0"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TYPE"), varNum, CDF_CHAR, strlen(attr.type), attr.type);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    if (attr.units[0] == '*')
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "UNITS"), varNum, CDF_CHAR, 1, " ");
    }
    else
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "UNITS"), varNum, CDF_CHAR, strlen(attr.units), attr.units);
    }
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "CATDESC"), varNum, CDF_CHAR, strlen(attr.desc), attr.desc);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    // data type for valid min and max
    if (strcmp(attr.type, "CDF_EPOCH") == 0)
    {
        double val = attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_EPOCH, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_EPOCH, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_UINT1") == 0)
    {
        uint8_t val = (uint8_t) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_UINT1, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (uint8_t) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_UINT1, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_UINT2") == 0)
    {
        uint16_t val = (uint16_t) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_UINT2, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (uint16_t) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_UINT2, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_UINT4") == 0)
    {
        uint32_t val = (uint32_t) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_UINT4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (uint32_t) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_UINT4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_REAL4") == 0)
    {
        float val = (float) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_REAL4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (float) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_REAL4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_REAL8") == 0)
    {
        double val = (double) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_REAL8, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (double) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_REAL8, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "FORMAT"), varNum, CDF_CHAR, strlen(attr.format), attr.format);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    return status;
}

void addAttributesLR(CDFid id, const char *cdfFilename, const char *efiFilenames, size_t nFiles, const char *softwareVersion, const char satellite, const char *version, double minTime, double maxTime)
{
    long attrNum = 0;
    char buf[1000] = {0};
    size_t fLen = 0;

    // Global attributes
    CDFcreateAttr(id, "Project", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Living Planet Programme");
    CDFcreateAttr(id, "Mission_group", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm");
    CDFcreateAttr(id, "TITLE", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c TRACIS ion image and spectra data (low resolution).", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "PI_name", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "David Knudsen");   
    CDFcreateAttr(id, "PI_affiliation", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Acknowledgement", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm EFI TRACIS data are available from https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Software_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, softwareVersion);
    CDFcreateAttr(id, "MODS", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Initial release.");
    char fullFileName[FILENAME_MAX] = {0};
    sprintf(fullFileName, "%s.cdf", cdfFilename + strlen(cdfFilename) - TRACIS_BASE_FILENAME_LENGTH);
    CDFcreateAttr(id, "File_Name", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, fullFileName);
    CDFcreateAttr(id, "List_Of_Input_Files", GLOBAL_SCOPE, &attrNum);
    for (int i = 0; i < nFiles; i++)
    {
        fLen = strlen(efiFilenames + i * FILENAME_MAX);
        addgEntry(id, attrNum, i, efiFilenames + i * FILENAME_MAX + fLen - 59);
    }

    CDFcreateAttr(id, "File_naming_convention", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "SW_%s_EFIxTISL1B", TRACIS_PRODUCT_TYPE);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_source_description", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c TII Raw And Corrected Imagery / Spectra low-resolution product", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Source_name", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c>Swarm %c", satellite, satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Data_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "L0>Low resolution data");
    CDFcreateAttr(id, "Data_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, version);
    CDFcreateAttr(id, "Descriptor", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "TRACIS>Swarm TII Raw And Corrected Imagery / Spectra (low resolution)");
    CDFcreateAttr(id, "Discipline", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Space Physics>Ionospheric Science");
    CDFcreateAttr(id, "Generated_by", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Generation_date", GLOBAL_SCOPE, &attrNum);
    time_t created;
    time(&created);
    struct tm * dp = gmtime(&created);
    char dateCreated[255] = { 0 };
    sprintf(dateCreated, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", dp->tm_year+1900, dp->tm_mon+1, dp->tm_mday, dp->tm_hour, dp->tm_min, dp->tm_sec);
    addgEntry(id, attrNum, 0, dateCreated);
    CDFcreateAttr(id, "LINK_TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm EFI TRACIS data are available at");
    CDFcreateAttr(id, "LINK_TITLE", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm Data Access");
    CDFcreateAttr(id, "HTTP_LINK", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Electric Fields (space)");
    addgEntry(id, attrNum, 1, "Particles (space)");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Plasma and Solar Wind");
    CDFcreateAttr(id, "TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm Electric Field Instrument Thermal Ion Imager ion imagery and spectra.");
    addgEntry(id, attrNum, 1, "Burchill and Knudsen, 2022. Thermal Ion Imager Performance.");
    addgEntry(id, attrNum, 2, "Knudsen, D.J., Burchill, J.K., Buchert, S.C., Eriksson, A.I., Gill, R., Wahlund, J.E., Ahlen, L., Smith, M. and Moffat, B., 2017. Thermal ion imagers and Langmuir probes in the Swarm electric field instruments. Journal of Geophysical Research: Space Physics, 122(2), pp.2655-2673.");
    CDFcreateAttr(id, "Time_resolution", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "variable");
    CDFcreateAttr(id, "spase_DatasetResourceID", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "spase://ESA/Instrument/Swarm%c/TIS_LR/variable_rate", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_file_id", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "swarm%c_TISL1B_L0__v%s", tolower(satellite), version);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_source", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c_TISL1B_L0", satellite);
    addgEntry(id, attrNum, 0, buf);

    CDFcreateAttr(id, "FIELDNAM", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "CATDESC", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "UNITS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VAR_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DEPEND_0", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DISPLAY_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "LABLAXIS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMIN", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMAX", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "FORMAT", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TIME_BASE", VARIABLE_SCOPE, &attrNum);

    const varAttr variableAttrs[] = {
        {"Timestamp", "CDF_EPOCH", "*", " ", minTime, maxTime, "%f"},
        {"Latitude", "CDF_REAL8", "degrees", "Geocentric latitude.", -90., 90., "%5.1f"},
        {"Longitude", "CDF_REAL8", "degrees", "Geocentric longitude.", -180., 180., "%6.1f"},
        {"Radius", "CDF_REAL8", "m", "Geocentric radius.", 6400000., 7400000., "%9.1f"},
        {"Raw_image_H", "CDF_UINT2", "DN", "40x66 array of raw image data for TII H sensor", 0, 4095, "%4.0f"},
        {"Raw_image_V", "CDF_UINT2", "DN", "40x66 array of raw image data for TII V sensor", 0, 4095, "%4.0f"},
        {"Processed_image_H", "CDF_UINT2", "DN", "40x66 array of gain-corrected image data for TII H sensor", 0, 4095, "%4.0f"},
        {"Processed_image_V", "CDF_UINT2", "DN", "40x66 array of gain-corrected image data for TII V sensor", 0, 4095, "%4.0f"},

        {"Valid_imagery_H", "CDF_UINT1", "*", "0: no or partial image; 1: full image (H sensor)", 0, 1, "%1.0f"},
        {"Valid_imagery_V", "CDF_UINT1", "*", "0: no or partial image; 1: full image (V sensor)", 0, 1, "%1.0f"},
        {"Image_anomaly_flags_H", "CDF_UINT1", "*", "H sensor bit-wise OR’d imaging anomaly flags.", 0, 255, "%3.0f"},
        {"Image_anomaly_flags_V", "CDF_UINT1", "*", "V sensor bit-wise OR’d imaging anomaly flags.", 0, 255, "%3.0f"},
        {"TII_imaging_mode", "CDF_UINT1", "*", "Imaging mode. 0: non-nominal; 1: nominal science", 0, 255, "%1.0f"},

        {"CCD_dark_current_H", "CDF_UINT2", "DN", "H sensor CCD dark current (from a single pixel)", 0, 4095, "%4.0f"},
        {"CCD_dark_current_V", "CDF_UINT2", "DN", "V sensor CCD dark current (from a single pixel)", 0, 4095, "%4.0f"},
        {"CCD_temperature_H", "CDF_REAL4", "deg. C", "H sensor CCD temperature (derived from dark current)", 0, 4095, "%4.1f"},
        {"CCD_temperature_V", "CDF_REAL4", "deg. C", "V sensor CCD temperature (derived from dark current)", 0, 4095, "%4.1f"},
        {"V_MCP_H", "CDF_REAL4", "V", "H sensor MCP front voltage", -2500, 0, "%7.1f"},
        {"V_MCP_V", "CDF_REAL4", "V", "V sensor MCP front voltage", -2500, 0, "%7.1f"},
        {"V_Phos_H", "CDF_REAL4", "V", "H sensor phosphor voltage", 0, 8500, "%6.1f"},
        {"V_Phos_V", "CDF_REAL4", "V", "V sensor phosphor voltage", 0, 8500, "%6.1f"},
        {"V_Bias_H", "CDF_REAL4", "V", "H sensor inner dome bias voltage", -105, 0, "%6.1f"},
        {"V_Bias_V", "CDF_REAL4", "V", "V sensor inner dome bias voltage", -105, 0, "%6.1f"},
        {"V_Faceplate", "CDF_REAL4", "V", "Faceplate voltage", -6.0, 0.0, "%4.1f"},
        {"Shutter_duty_cycle_H", "CDF_REAL4", "*", "H sensor electrostatic shutter open duty cycle", 0.0, 1.0, "%5.3f"},
        {"Shutter_duty_cycle_V", "CDF_REAL4", "*", "V sensor electrostatic shutter open duty cycle", 0.0, 1.0, "%5.3f"},
        {"Energy_map_H", "CDF_REAL4", "eV", "H sensor pixel energy map", 0.0, 50.0, "%5.2f"},
        {"Energy_map_V", "CDF_REAL4", "eV", "V sensor pixel energy map", 0.0, 50.0, "%5.2f"},
        {"Angle_of_arrival_map_H", "CDF_REAL4", "eV", "H sensor pixel energy map", 0.0, 360.0, "%6.2f"},
        {"Angle_of_arrival_map_V", "CDF_REAL4", "eV", "V sensor pixel energy map", 0.0, 360.0, "%6.2f"},
        {"Energy_spectrum_H", "CDF_REAL4", "*", "H sensor energy spectrum", 0, 1e6, "%5.2g"},
        {"Energy_spectrum_V", "CDF_REAL4", "*", "V sensor energy spectrum", 0, 1e6, "%5.2g"},
        {"Angle_of_arrival_spectrum_H", "CDF_REAL4", "*", "H sensor angle-of-arrival spectrum", 0, 1e6, "%5.2g"},
        {"Angle_of_arrival_spectrum_V", "CDF_REAL4", "*", "V sensor angle-of-arrival spectrum", 0, 1e6, "%5.2g"},
        {"Raw_energy_spectrum_H", "CDF_REAL4", "*", "H sensor energy spectrum from uncorrected image", 0, 1e6, "%5.2g"},
        {"Raw_energy_spectrum_V", "CDF_REAL4", "*", "V sensor energy spectrum from uncorrected image", 0, 1e6, "%5.2g"},
        {"Raw_angle_of_arrival_spectrum_H", "CDF_REAL4", "*", "H sensor angle-of-arrival spectrum from uncorrected image", 0, 1e6, "%5.2g"},
        {"Raw_angle_of_arrival_spectrum_V", "CDF_REAL4", "*", "V sensor angle-of-arrival spectrum from uncorrected image", 0, 1e6, "%5.2g"},
        {"Energies_H", "CDF_REAL4", "eV", "Mean energies of H sensor energy spectrum bins.", 0, 50.0, "%5.2f"},
        {"Energies_V", "CDF_REAL4", "eV", "Mean energies of V sensor energy spectrum bins.", 0, 50.0, "%5.2f"},
        {"Angles_of_arrival", "CDF_REAL4", "Degrees", "Midpoint angles of angle-of-arrival spectrum bins.", -180.0, 180.0, "%6.2f"}
    };

    for (uint8_t i = 0; i < NUM_EXPORT_VARIABLES_LR; i++)
    {
        addVariableAttributes(id, variableAttrs[i]);
    }

}

void addAttributesHR(CDFid id, const char *cdfFilename, const char *efiFilenames, size_t nFiles, const char *softwareVersion, const char satellite, const char *version, double minTime, double maxTime)
{
    long attrNum = 0;
    char buf[1000] = {0};
    size_t fLen = 0;

    // Global attributes
    CDFcreateAttr(id, "Project", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Living Planet Programme");
    CDFcreateAttr(id, "Mission_group", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm");
    CDFcreateAttr(id, "TITLE", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c TRACIS ion image and spectra data (high resolution).", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "PI_name", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "David Knudsen");   
    CDFcreateAttr(id, "PI_affiliation", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Acknowledgement", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm EFI TRACIS data are available from https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Software_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, softwareVersion);
    CDFcreateAttr(id, "MODS", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Initial release.");
    char fullFileName[FILENAME_MAX] = {0};
    sprintf(fullFileName, "%s.cdf", cdfFilename + strlen(cdfFilename) - TRACIS_BASE_FILENAME_LENGTH);
    CDFcreateAttr(id, "File_Name", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, fullFileName);
    CDFcreateAttr(id, "List_Of_Input_Files", GLOBAL_SCOPE, &attrNum);
    for (int i = 0; i < nFiles; i++)
    {
        fLen = strlen(efiFilenames + i * FILENAME_MAX);
        addgEntry(id, attrNum, i, efiFilenames + i * FILENAME_MAX + fLen - 59);
    }

    CDFcreateAttr(id, "Logical_source_description", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c TII Raw And Corrected Imagery / Spectra high-resolution product", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Source_name", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c>Swarm %c", satellite, satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Data_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "H0>High resolution data");
    CDFcreateAttr(id, "Data_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, version);
    CDFcreateAttr(id, "Descriptor", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "TRACIS>Swarm TII Raw And Corrected Imagery / Spectra (high resolution)");
    CDFcreateAttr(id, "Discipline", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Space Physics>Ionospheric Science");
    CDFcreateAttr(id, "Generated_by", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Generation_date", GLOBAL_SCOPE, &attrNum);
    // Get rid of trailing newline from creation date
    time_t created;
    time(&created);
    struct tm * dp = gmtime(&created);
    char dateCreated[255] = { 0 };
    sprintf(dateCreated, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", dp->tm_year+1900, dp->tm_mon+1, dp->tm_mday, dp->tm_hour, dp->tm_min, dp->tm_sec);
    addgEntry(id, attrNum, 0, dateCreated);
    CDFcreateAttr(id, "LINK_TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm EFI TRACIS data are available at");
    CDFcreateAttr(id, "LINK_TITLE", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm Data Access");
    CDFcreateAttr(id, "HTTP_LINK", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Electric Fields (space)");
    addgEntry(id, attrNum, 1, "Particles (space)");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Plasma and Solar Wind");
    CDFcreateAttr(id, "TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm Electric Field Instrument Thermal Ion Imager ion imagery and spectra.");

    addgEntry(id, attrNum, 6, "Burchill and Knudsen, 2022. Thermal Ion Imager Performance.");
    addgEntry(id, attrNum, 7, "Knudsen, D.J., Burchill, J.K., Buchert, S.C., Eriksson, A.I., Gill, R., Wahlund, J.E., Ahlen, L., Smith, M. and Moffat, B., 2017. Thermal ion imagers and Langmuir probes in the Swarm electric field instruments. Journal of Geophysical Research: Space Physics, 122(2), pp.2655-2673.");
    CDFcreateAttr(id, "Time_resolution", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "variable");
    CDFcreateAttr(id, "spase_DatasetResourceID", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "spase://ESA/Instrument/Swarm%c/TIS_HR/variable_rate", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "File_naming_convention", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "SW_%s_EFIxTISH1B", TRACIS_PRODUCT_TYPE);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_file_id", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "swarm%c_TISH1B_H0__v%s", tolower(satellite), version);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_source", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c_TISH1B_H0", satellite);
    addgEntry(id, attrNum, 0, buf);

    CDFcreateAttr(id, "FIELDNAM", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "CATDESC", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "UNITS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VAR_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DEPEND_0", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DISPLAY_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "LABLAXIS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMIN", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMAX", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "FORMAT", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TIME_BASE", VARIABLE_SCOPE, &attrNum);

    const varAttr variableAttrs[] = {
        {"Timestamp", "CDF_EPOCH", "*", " ", minTime, maxTime, "%f"},
        {"Latitude", "CDF_REAL8", "degrees", "Geocentric latitude.", -90., 90., "%5.1f"},
        {"Longitude", "CDF_REAL8", "degrees", "Geocentric longitude.", -180., 180., "%6.1f"},
        {"Radius", "CDF_REAL8", "m", "Geocentric radius.", 6400000., 7400000., "%9.1f"},
        {"TII_imaging_mode", "CDF_UINT1", "*", "Imaging mode. 0: non-nominal; 1: nominal science", 0, 65535, "%1.0f"},
        {"V_MCP_Setting_H", "CDF_REAL4", "V", "H sensor MCP front voltage", -2500, 0, "%7.1f"},
        {"V_MCP_Setting_V", "CDF_REAL4", "V", "V sensor MCP front voltage", -2500, 0, "%7.1f"},
        {"V_Phos_Setting_H", "CDF_REAL4", "V", "H sensor phosphor voltage", 0, 8500, "%6.1f"},
        {"V_Phos_Setting_V", "CDF_REAL4", "V", "V sensor phosphor voltage", 0, 8500, "%6.1f"},
        {"V_Bias_Setting_H", "CDF_REAL4", "V", "H sensor inner dome bias voltage", -105, 0, "%6.1f"},
        {"V_Bias_Setting_V", "CDF_REAL4", "V", "V sensor inner dome bias voltage", -105, 0, "%6.1f"},
        {"Column_sum_spectrum_H", "CDF_UINT2", "DN", "H sensor column sum spectrum", 0, 65535, "%5d"},
        {"Column_sum_spectrum_V", "CDF_UINT2", "DN", "V sensor column sum spectrum", 0, 65535, "%5d"},
        {"Column_sum_energies_H", "CDF_REAL4", "eV", "Energies of H sensor column spectrum bins.", 0, 50.0, "%5.2f"},
        {"Column_sum_energies_V", "CDF_REAL4", "eV", "Energies of V sensor column spectrum bins.", 0, 50.0, "%5.2f"}
    };

    for (uint8_t i = 0; i < NUM_EXPORT_VARIABLES_HR; i++)
    {
        addVariableAttributes(id, variableAttrs[i]);
    }

}

