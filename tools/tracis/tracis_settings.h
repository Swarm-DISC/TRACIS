/*

    TRACIS Processor: tools/tracis/tracis_settings.h

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

#ifndef _TRACIS_SETTING_H
#define _TRACIS_SETTING_H

#define SOFTWARE_VERSION_STRING "TRACIS version 2023-03-17"
#define SOFTWARE_VERSION "01.03"
#define TRACIS_PRODUCT_CODE_LR "TISL1B" // TII Imagery / Spectra
#define TRACIS_PRODUCT_CODE_HR "TISH1B" // Column sum spectra
#define TRACIS_PRODUCT_TYPE "OPER"
#define TRACIS_FILE_TYPE_LR "EFIxTISL1B"
#define TRACIS_FILE_TYPE_HR "EFIxTISH1B"
#define EXPORT_VERSION_STRING "0102"

#define TRACIS_BASE_FILENAME_LENGTH 55

#define SECONDS_OF_DATA_REQUIRED_FOR_PROCESSING 1 // 1 second
#define SECONDS_OF_DATA_REQUIRED_FOR_EXPORTING 1 // 1 second

#define IMAGE_ROWS 66
#define IMAGE_COLS 40

#define TRACIS_K 1.38e-23
#define TRACIS_EPS 8.85e-12
#define TRACIS_QE 1.602e-19
#define TRACIS_RP 0.004 // m
#define TRACIS_WFP 0.351 // m
#define TRACIS_HFP 0.229 // m
#define TRACIS_MAMU 1.66e-27 // kg 


#define MAX_ALLOWED_CDF_GAP_SECONDS 86400.0 // CDF export split into separate files at gaps exceeding 24 Hours

#define NUM_EXPORT_VARIABLES_LR 41
#define NUM_EXPORT_VARIABLES_HR 15

#define CDF_GZIP_COMPRESSION_LEVEL 6L
#define CDF_BLOCKING_FACTOR 43200L

#define MINIMUM_VELOCITY_EPOCHS 10

#define ENERGY_BINS 20
#define MAX_ENERGY 35.0 // eV
#define MIN_ENERGY 0.0 // eV
#define MISSING_ENERGY -1.0
#define MAX_ENERGY_RESOLUTION 2.0 // maximum energy is calculated to the nearest this in eV based on inner dome voltage

#define COLUMN_SUM_ENERGY_BINS 32

#define ANGULAR_BINS 72
#define MAX_ANGLE 135
#define MIN_ANGLE -135.0
#define MISSING_ANGLE -200.0

#define MIN_RADIUS 5.0 // pixels
#define MAX_RADIUS 32.0 // pixels
#define R_INNER_PIXELS 10.5 / 0.3556
#define R_INNER_CUSHION 5

#endif // _TRACIS_SETTING_H
