/*

    TRACIS Processor: tools/tracis/export_products.h

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

#ifndef _EXPORT_PRODUCTS_H
#define _EXPORT_PRODUCTS_H

#include "utilities.h"
#include "load_satellite_velocity.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <cdf.h>

#define UTC_DATE_LENGTH 24

CDFstatus exportTracisCdfLR(const char *cdfFilename, const char satellite, const char *exportVersion, ImageStorage *store, size_t numberOfImagePairs, Ephemeres *ephem, char *efiFilenames, size_t nEfiFiles);

CDFstatus exportTracisCdfHR(const char *cdfFilename, const char satellite, const char *exportVersion, ImageStorage *store, size_t numberOfColumnSums, Ephemeres *ephem, char *efiFilenames, size_t nEfiFiles);

int exportTracisHeader(const char *tracisFilename, const char *efiFilenames, size_t nFiles, time_t processingStartTime, time_t startTime, time_t stopTime, bool lowResolutionDataset);

int exportProducts(char satellite, ImageStorage *store, size_t numberOfLRRecords, Ephemeres *imageEphem, char *tracisLRFilename, size_t numberOfHRRecords, Ephemeres *colSumEphem, char *tracisHRFilename, char *efiFilenames, size_t nEfiFiles, time_t processingStartTime);

int archiveFiles(char *filenameBase);

enum EXPORT_FLAGS {
    EXPORT_OK = 0,
    EXPORT_MEM = 1,
    EXPORT_METAINFO = 2,
    HEADER_OK = 3,
    HEADER_CREATE = 4,
    HEADER_START = 5,
    HEADER_WRITE_ERROR = 6,
    HEADER_FILENAME = 7,
    HEADER_CDFFILEINFO = 8,
    EXPORT_CDF = 9,
    EXPORT_HDR = 10,
    EXPORT_ZIP = 11
};


#endif // _EXPORT_PRODUCTS_H
