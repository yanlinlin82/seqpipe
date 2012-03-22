#!/bin/bash

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# This script implements the pindel call structure variations protocol.
# Written by Wang Meng, 2012-03-20.

# The input arguments list is as follows:
# <config file> <platform> <whether remove intermediate files> 
# <number of threads> <length between paired-end adapters>

# Finial output: flt.sv.vcf


if [ $# -ne 5 ]
then
    echo "$0 : Too few arguments!"
    exit 1
fi

if [ ! -f map_result.sorted.bam ]
then
    echo "Cannot find map_result.sorted.bam at current directory."
    exit 1
fi

# load configuration file
source "$1"

PLATFORM=$2
RM_INTER=$3
THREAD_NUM=$4
LIB_LENGTH=$5


# make a temp pindel config file
echo "map_result.sorted.bam ${LIB_LENGTH} CBI" > pindel_config.conf
mkdir sv

# Run pindel
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin pindel call sv\t0\t[OK]"
time pindel -f "${REF_PATH}/${REF_NAME}.fa" -i pindel_config.conf -l -k -s \
-c ALL -o sv/sv 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when pindel \
    call sv\t0\t[FAIL]"
    exit 1
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, pindel call sv \
finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f pindel_config.conf
fi

# Change the result into vcf format
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin change pindel result into \
vcf\t0\t[OK]"
time pindel2vcf -p sv/sv_D -r "${REF_PATH}/${REF_NAME}.fa" -R "${REF_NAME}" \
-d 20110705 -v sv/sv_D.vcf 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when change \
    pindel result into vcf\t0\t[FAIL]"
    exit 1
fi
time pindel2vcf -p sv/sv_SI -r "${REF_PATH}/${REF_NAME}.fa" -R "${REF_NAME}" \
-d 20110705 -v sv/sv_SI.vcf 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when change \
    pindel result into vcf\t0\t[FAIL]"
    exit 1
fi
time pindel2vcf -p sv/sv_INV -r "${REF_PATH}/${REF_NAME}.fa" -R "${REF_NAME}" \
-d 20110705 -v sv/sv_INV.vcf 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when change \
    pindel result into vcf\t0\t[FAIL]"
    exit 1
fi
time pindel2vcf -p sv/sv_TD -r "${REF_PATH}/${REF_NAME}.fa" -R "${REF_NAME}" \
-d 20110705 -v sv/sv_TD.vcf 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when change \
    pindel result into vcf\t0\t[FAIL]"
    exit 1
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, change result into \
vcf finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f sv/sv_D
    rm -f sv/sv_SI
    rm -f sv/sv_INV
    rm -f sv/sv_TD
fi


exit 0
