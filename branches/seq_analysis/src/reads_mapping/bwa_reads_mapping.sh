#!/bin/bash

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
   
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# This script implements the bwa reads mapping protocol.
# Written by Wang Meng, 2012-03-18.

# The input arguments list is as follows:
# <config file> <platform> <whether remove intermediate files> 
# <number of threads> <length between paired-end adapters> 
# <number of lanes> <lane1 pe1 path>  <lane1 pe2 path>
# [lane2 pe1 path] [lane2 pe2 path] ...

# Finial output: map_result.sorted.bam


if [ $# -lt 8 ]
then
    echo -e "Usage: $0 <config file> <platform> <whether remove intermediate\c"
    echo -e " files> <number of threads> <library size> <number of lanes> \c"
    echo "<pe1_1.fq> <pe1_2.fq> [pe2_1.fq] [pe2_2.fq]"
    exit 1
fi

# load configuration file
source "$1"

PLATFORM=$2
RM_INTER=$3
THREAD_NUM=$4
LIB_LENGTH=$5
LANES=$6
ID="CBI"
SM=$ID

if [ $FQ_VERSION = 1.5 ]
then
    FQ_VERSION="-I"
else
    FQ_VERSION=""
fi

shift 6

# if index doesn't exist, first build the index
if [ ! -e "${REF_PATH}/${REF_NAME}.fa.fai" ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin bwa index\t0\t[OK]"
    time bwa index -a bwtsw "${REF_PATH}/${REF_NAME}.fa" 1>&2
    if [ $? != 0 ]
    then
        echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
        bwa index\t0\t[FAIL]"
        exit 1
    fi
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, bwa index  \
    finished successfully\t0\t[OK]"
fi

# align reads to reference genome
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin bwa aln\t0\t[OK]"
for reads in $@
do
    time bwa aln -t $THREAD_NUM -i $END_IND -e $GAP_EXT $FQ_VERSION \
    "${REF_PATH}/${REF_NAME}.fa" "$reads" > "${reads}.sai"
    if [ $? != 0 ]
    then
        echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
        bwa aln\t0\t[FAIL]"
        exit 2
    fi
done
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, bwa aln finished \
successfully\t0\t[OK]"

# map reads to reference genome
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin bwa sampe\t0\t[OK]"
i=1
while [ $i -le $LANES ]
do
    reads1=$1
    reads2=$2
    shift 2
    
    time bwa sampe -P -r "@RG\tID:$ID\tSM:$SM\tPL:$PLATFORM" -a $LIB_LENGTH \
    "${REF_PATH}/${REF_NAME}.fa" "${reads1}.sai" "${reads2}.sai" \
    "$reads1" "$reads2" | samtools view -Sb - > "L${i}_bwa_result.bam"
    if [ $? != 0 ]
    then
        echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
        bwa sampe\t0\t[FAIL]"
        exit 3
    fi
    
    # remove intermediate files if possible
    if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
    then
        rm -f "${reads1}.sai" "${reads2}.sai"
    fi
    
    i=$(($i+1))
done
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, bwa sampe finished \
successfully\t0\t[OK]"


# merge all lane's result
if [ $LANES -eq 1 ]
then
    mv L1_bwa_result.bam bwa_result.bam
else
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin samtools cat\t0\t[OK]"
    names=""
    i=1
    while [ $i -le $LANES ]
    do
        names="${names} L${i}_bwa_result.bam"
        i=$(($i+1))
    done
    
    time samtools cat -o bwa_result.bam $names 1>&2

    if [ $? != 0 ]
    then
        echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
        samtools cat\t0\t[FAIL]"
        exit 4
    fi
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, samtools cat \
    finished successfully\t0\t[OK]"
    
    # remove intermediate files if possible
    if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
    then
        rm -f $names
    fi
fi

# sort bam result using picard
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin sort bam using picard\t0\t[OK]"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar $PICARD_HOME/SortSam.jar \
    MAX_RECORDS_IN_RAM=1875000 \
    VALIDATION_STRINGENCY=SILENT \
    SO=coordinate \
    I=bwa_result.bam \
    O=map_result.sorted.bam \
    CREATE_INDEX=true 1>&2
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when sort \
    bam by picard\t0\t[FAIL]"
    exit 5
fi

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, sort bam finished \
successfully\t0\t[OK]"

# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f bwa_result.bam
fi

exit 0
