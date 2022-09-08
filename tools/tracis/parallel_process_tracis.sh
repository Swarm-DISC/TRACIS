#!/bin/bash

# TRACIS Processor: tools/tracis/parallel_process_tracis.txt

# Copyright (C) 2022  Johnathan K Burchill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


# Generates TRACIS CDFs for specified satellite for a range of dates
# http://stackoverflow.com/questions/28226229/how-to-loop-through-dates-using-bash

satellite=$1
startDate=$2
stopDate=$3
exportVersion=$4
numCpus=$5


if test "$#" -ne "5"; then
  echo "Usage: $0 satelliteLetter startDate stopDate exportVersion numCpus"
  exit 1
fi

processingDate=$(date +%Y%m%dT%H%M%S)

dateToProcess="$startDate"
cpuNum=0

while [ $cpuNum -lt $numCpus ]; do
	cpuNum=$((cpuNum + 1))
	xterm -geometry 80x1 -e "process_tracis.sh ${satellite} ${dateToProcess} ${stopDate} $exportVersion $numCpus" &
	dateToProcess=$(date -I -d "$dateToProcess + 1 day")
done



