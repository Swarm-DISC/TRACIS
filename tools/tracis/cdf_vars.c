/*

    TRACIS Processor: tools/tracis/cdf_vars.c

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

#include "cdf_vars.h"
#include "utilities.h"
#include "tracis_settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


CDFstatus createVarFrom1DVar(CDFid id, char *name, long dataType, long startIndex, long stopIndex, void *buffer, bool compressed)
{
    CDFstatus status;
    long exportDimSizes[1] = {0};
    long recVary = {VARY};
    long dimNoVary = {NOVARY};
    long varNumber;
    long cType;
    long cParams[CDF_MAX_PARMS];

    status = CDFcreatezVar(id, name, dataType, 1, 0L, exportDimSizes, recVary, dimNoVary, &varNumber);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFsetzVarSparseRecords(id, varNumber, NO_SPARSERECORDS);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    cType = GZIP_COMPRESSION;
    cParams[0] = CDF_GZIP_COMPRESSION_LEVEL; // GZIP compression level 6 as suggested compromised between speed and size by CDF C reference 
    status = CDFsetzVarCompression(id, varNumber, cType, cParams);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    // We get a message about BLOCKINGFACTOR_TOO_SMALL2 for some variables. Because the number of records is typically less than 2000 per file, we won't set a bocking factor.
    // // Blocking factor 43200 as requested by DTU
    // status = CDFsetzVarBlockingFactor(id, varNumber, CDF_BLOCKING_FACTOR);
    // if (status != CDF_OK)
    // {
    //     printErrorMessage(status);
    //     return status;
    // }

    long dataTypeSize;
    status = CDFgetDataTypeSize(dataType, &dataTypeSize);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputVarRangeRecordsByVarName(id, name, 0, stopIndex-startIndex, (void*)((uint8_t*)buffer + (dataTypeSize*startIndex)));
    if (status != CDF_OK)
    {
        printErrorMessage(status);
    }
    return status;
}

CDFstatus createVarFrom2DVar(CDFid id, char *name, long dataType, long startIndex, long stopIndex, void *buffer1D, uint8_t dimSize, bool compressed)
{
    CDFstatus status = CDF_OK;
    long long nRecs = stopIndex - startIndex + 1;
    long dimSizes[1] = {0};
    long recVary = {VARY};
    long dimVary[1] = {VARY};
    long varNumber;
    long cType;
    long cParams[CDF_MAX_PARMS];

    dimSizes[0] = dimSize;

    status = CDFcreatezVar(id, name, dataType, 1, 1L, dimSizes, recVary, dimVary, &varNumber);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    cType = GZIP_COMPRESSION;
    cParams[0] = CDF_GZIP_COMPRESSION_LEVEL; // GZIP compression level 6 as suggested compromised between speed and size by CDF C reference 
    if (compressed)
    {
        status = CDFsetzVarCompression(id, varNumber, cType, cParams);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    // Blocking factor 43200 as requested by DTU
    status = CDFsetzVarBlockingFactor(id, varNumber, CDF_BLOCKING_FACTOR);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFsetzVarSparseRecords(id, varNumber, NO_SPARSERECORDS);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    long dataSize;
    status = CDFgetDataTypeSize(dataType, &dataSize);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputVarRangeRecordsByVarName(id, name, 0, stopIndex-startIndex, (void *)buffer1D);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
    }

    return status;
}

CDFstatus createVarFromImage(CDFid id, char *name, long dataType, long startIndex, long stopIndex, void *imageBuffer, bool compressed)
{
    CDFstatus status = CDF_OK;
    long long nRecs = stopIndex - startIndex + 1;
    long dimSizes[2] = {0, 0};
    long recVary = {VARY};
    long dimVary[2] = {VARY, VARY};
    long varNumber;
    long cType;
    long cParams[CDF_MAX_PARMS];

    dimSizes[0] = IMAGE_COLS;
    dimSizes[1] = IMAGE_ROWS;

    status = CDFcreatezVar(id, name, dataType, 1, 2L, dimSizes, recVary, dimVary, &varNumber);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    cType = GZIP_COMPRESSION;
    cParams[0] = CDF_GZIP_COMPRESSION_LEVEL; // GZIP compression level 6 as suggested compromised between speed and size by CDF C reference 
    if (compressed)
    {
        status = CDFsetzVarCompression(id, varNumber, cType, cParams);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    status = CDFsetzVarSparseRecords(id, varNumber, NO_SPARSERECORDS);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    long dataSize;
    status = CDFgetDataTypeSize(dataType, &dataSize);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    status = CDFputVarRangeRecordsByVarName(id, name, startIndex, stopIndex-startIndex, imageBuffer);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
    }

    return status;
}

