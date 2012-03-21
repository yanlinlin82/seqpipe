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
    echo "$0 : Too few arguments!"
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
    echo "+-----------------------+"
    echo "|    Begin bwa index    |"
    echo "+-----------------------+"
    time bwa index -a bwtsw "${REF_PATH}/${REF_NAME}.fa" 1>&2
    if [ $? != 0 ]
    then
        echo "+------------------------------------------+"
        echo "|    Oops, error occured when bwa index    |"
        echo "+------------------------------------------+"
        exit 1
    fi
    echo "+------------------------------------------------------+"
    echo "|    Congratulates, bwa index finished successfully    |"
    echo "+------------------------------------------------------+"
fi

# align reads to reference genome
echo "+---------------------+"
echo "|    Begin bwa aln    |"
echo "+---------------------+"
for reads in $@
do
    time bwa aln -t $THREAD_NUM -i $END_IND -e $GAP_EXT $FQ_VERSION \
    "${REF_PATH}/${REF_NAME}.fa" "$reads" > "${reads}.sai"
    if [ $? != 0 ]
    then
        echo "+----------------------------------------+"
        echo "|    Oops, error occured when bwa aln    |"
        echo "+----------------------------------------+"
        exit 2
    fi
done
echo "+----------------------------------------------------+"
echo "|    Congratulates, bwa aln finished successfully    |"
echo "+----------------------------------------------------+"

# map reads to reference genome
echo "+-----------------------+"
echo "|    Begin bwa sampe    |"
echo "+-----------------------+"
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
        echo "+------------------------------------------+"
        echo "|    Oops, error occured when bwa sampe    |"
        echo "+------------------------------------------+"
        exit 3
    fi
    
    # remove intermediate files if possible
    if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
    then
        rm -f "${reads1}.sai" "${reads2}.sai"
    fi
    
    i=$(($i+1))
done
echo "+------------------------------------------------------+"
echo "|    Congratulates, bwa sampe finished successfully    |"
echo "+------------------------------------------------------+"


# merge all lane's result
if [ $LANES -eq 1 ]
then
    mv L1_bwa_result.bam bwa_result.bam
else
    echo "+--------------------------+"
    echo "|    Begin samtools cat    |"
    echo "+--------------------------+"
    names=""
    i=1
    while [ $i -le $LANES ]
    do
        names="${names} L${i}_bwa_result.bam"
    done
    
    time samtools cat -o bwa_result.bam "$names" 1>&2
    if [ $? != 0 ]
    then
        echo "+---------------------------------------------+"
        echo "|    Oops, error occured when samtools cat    |"
        echo "+---------------------------------------------+"
        exit 4
    fi
    echo "+---------------------------------------------------------+"
    echo "|    Congratulates, samtools cat finished successfully    |"
    echo "+---------------------------------------------------------+"
    
    # remove intermediate files if possible
    if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
    then
        rm -f "$names"
    fi
fi

# sort bam result using picard
echo "+-----------------------------------+"
echo "|    Begin sort bam using picard    |"
echo "+-----------------------------------+"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar $PICARD_HOME/SortSam.jar \
    MAX_RECORDS_IN_RAM=1875000 \
    SO=coordinate \
    I=bwa_result.bam \
    O=map_result.sorted.bam \
    CREATE_INDEX=true 1>&2
if [ $? != 0 ]
then
    echo "+---------------------------------------------------+"
    echo "|    Oops, error occured when sort bam by picard    |"
    echo "+---------------------------------------------------+"
    exit 5
fi

echo "+-----------------------------------------------------+"
echo "|    Congratulates, sort bam finished successfully    |"
echo "+-----------------------------------------------------+"

# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f bwa_result.bam
fi

exit 0
