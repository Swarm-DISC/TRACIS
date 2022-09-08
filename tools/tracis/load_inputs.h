/*

    TRACIS Processor: tools/tracis/load_inputs.h

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

#ifndef _LOAD_INPUTS_H
#define _LOAD_INPUTS_H

#include <stdint.h>


void loadInputs(const char *cdfFile, int expectedNumberOfFileVariables, char *variables[], int nVariables, uint8_t **dataBuffers, long *numberOfRecords);

#endif // _LOAD_INPUTS_H

