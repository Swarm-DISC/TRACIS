/*

    TRACIS Processor: tools/tracis/load_inputs.c

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

#include "load_inputs.h"
#include "tracis_settings.h"
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cdf.h>

extern char infoHeader[50];

void loadInputs(const char *cdfFile, int expectedNumberOfFileVariables, char *variables[], int nVariables, uint8_t **dataBuffers, long *numberOfRecords)
{
    // Open the CDF file with validation
    CDFsetValidate(VALIDATEFILEoff);
    CDFid cdfId;
    CDFstatus status;
    // Attributes
    long attrN;
    long entryN;
    char attrName[CDF_ATTR_NAME_LEN256+1];
    long attrScope, maxEntry;

    // Check CDF info
    long decoding, encoding, majority, maxrRec, numrVars, maxzRec, numzVars, numAttrs, format;

    long numBytesToAdd, numVarBytes, numValues;
    long varNum, dataType, numElems, numRecs, numDims, recVary;
    long dimSizes[CDF_MAX_DIMS], dimVarys[CDF_MAX_DIMS];
    CDFdata data;

    status = CDFopenCDF(cdfFile, &cdfId);
    if (status != CDF_OK) 
    {
        printErrorMessage(status);
        fprintf(stdout, "%s Could not open CDF file. Skipping this date.\n", infoHeader);
        return;
    }

    status = CDFgetFormat(cdfId, &format);
    status = CDFgetDecoding(cdfId, &decoding);
    status = CDFinquireCDF(cdfId, &numDims, dimSizes, &encoding, &majority, &maxrRec, &numrVars, &maxzRec, &numzVars, &numAttrs);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        fprintf(stdout, "\n%s Problem with CDF file. Skipping this date.\n", infoHeader);
        closeCdf(cdfId);
        return;
    }
    uint8_t nVars = numzVars;
    if (nVars != expectedNumberOfFileVariables)
    {
        fprintf(stdout, "\n%s Error: number of calibration variables should be %d. Got %ld. Skipping this date.\n", infoHeader, (uint8_t) nVariables, numzVars);
        closeCdf(cdfId);
        return;
    }
    for (uint8_t i = 0; i<nVariables; i++)
    {
        status = CDFconfirmzVarExistence(cdfId, variables[i]);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            fprintf(stdout, "\n%s Error reading variable %s from CDF file. Skipping this date.\n", infoHeader, variables[i]);
            closeCdf(cdfId);
                return;
        }
        else
        {
            // fprintf(stdout, "%s OK\n", infoHeader);
        }
    }
    

    for (uint8_t i = 0; i < nVariables; i++)
    {
        varNum = CDFgetVarNum(cdfId, variables[i]);
        status = CDFreadzVarAllByVarID(cdfId, varNum, &numRecs, &dataType, &numElems, &numDims, dimSizes, &recVary, dimVarys, &data);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            fprintf(stdout, "%s Error loading data for %s. Skipping this date.\n", infoHeader, variables[i]);
            closeCdf(cdfId);
            CDFdataFree(data);
            return;
        }
        // Calculate new size of memory to allocate
        status = CDFgetDataTypeSize(dataType, &numVarBytes);
        numValues = 1;
        for (uint8_t j = 0; j < numDims; j++)
        {
            numValues *= dimSizes[j];
        }
        numBytesToAdd = numValues * numRecs * numVarBytes;
        dataBuffers[i] = (uint8_t*) realloc(dataBuffers[i], (size_t) numBytesToAdd);
        memcpy(dataBuffers[i], data, numBytesToAdd);
        CDFdataFree(data);
    }
    // close CDF
    closeCdf(cdfId);

    // Update number of records found and memory allocated
    *numberOfRecords += numRecs;
    // *totalMemoryAllocated = fpMemorySize;

}

