#!/bin/bash

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
   
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# This script is the integration of all protocols and provide an simple
# user interface to configuration.
# Written by Wang Meng, 2012-03-18.


# Default settings
CONF_PATH=$(dirname $(realpath $0))/config       #configuration file path
PLATFORM="illumina"                  #sequencing platform
RM_INTER="y"                         #whether to remove intermediate files
THREAD_NUM=3                         #number of threads
LANES=1                              #number of sample lanes
LIB_LENGTH=""   #the length between paired-end adapters in paired-end sequence

if [ $# -ne 0 ]
then
    while getopts "c:p:rt:n:l:vh" optionName
    do
        case "$optionName" in
        c) CONF_PATH="$OPTARG";;
        p) PLATFORM="$OPTARG";;
        r) RM_INTER="n";;
        t) THREAD_NUM=$OPTARG;;
        n) LANES=$OPTARG;;
        l) LIB_LENGTH=$OPTARG;;
        v) echo "Version: 1.0"
           exit 0
           ;;
        h|[?])
           echo ""
           echo -e "Usage: seq_analysis [option] -l <int> -n <int> <PE1_1.fq> \c"
           echo "<PE1_2.fq> [PE2_1.fq] [PE2_2.fq]..."
           echo ""
           echo " -c STRING       config file path[default]"
           echo " -p STRING       platform[illumina]"
           echo " -r              keep intermediate files"
           echo " -t INT          number of threads[3]"
           echo " -n INT          number of lanes[1]"
           echo " -l INT          library size(Mandatory)"
           echo " -v              program version"
           echo " -h              help"
           echo ""
           exit 0
           ;;
        esac
    done
    shift $(($OPTIND - 1))
    NAMES=$@
else

clear
echo "------------------------------------------------------------"
echo "       Welcome to use Seq Analysis"
echo "             ---- A tool for resequencing data analysis"
echo ""
echo "  Version: 1.0"
echo "  Copyright (C) 2012 Center for Bioinformatics, PKU"
echo "------------------------------------------------------------"
echo ""

# Get customer configuration
while true
do
    echo -e "1/6 Config file path(to use default, just press Enter) : \c"
    read VAR
    if [ "${VAR}" != "" ] && [ -f $VAR ]
    then
        CONF_PATH=$VAR
        break
    fi
    if [ "${VAR}" = "" ]
    then
        break
    fi
    echo "Invalidate file path!"
done

echo ""
echo -e "2/6 Platform(default is illumina) : \c"
read VAR
if [ "${VAR}" != "" ]
then
    PLATFORM=$VAR
fi

echo ""
echo -e "3/6 Remove intermediate files([y/n], default is y) : \c"
read VAR
if [ "${VAR}" != "" ]
then
    RM_INTER=$VAR
fi

echo ""
echo -e "4/6 Number of threads(default is 3) : \c"
read VAR
if [ "${VAR}" != "" ]
then
    THREAD_NUM=$VAR
fi

echo ""
while [ "${LIB_LENGTH}" = "" ]
do
    echo -e "5/6 length between paired-end adapters : \c"
    read VAR
    if [ "${VAR}" != "" ]
    then
        LIB_LENGTH=$VAR
    fi
done

echo ""
echo -e "6/6 Number of sample lanes(default is 1) : \c"
read VAR
if [ "${VAR}" != "" ]
then
    LANES=$VAR
fi
echo ""

# Get names of pair end files for each lane
NAMES=""
i=1
while [ $i -le $LANES ]
do
    while true
    do
        echo -e "Enter pair end reads 1 of lane $i (.fq or .fq.gz): \c"
        read VAR
        if [ "${VAR}" != "" ] && [ -f $VAR ]
        then
            NAMES="${NAMES} ${VAR}"
            break
        fi
        echo "Invalidate file path!"
    done
    while true
    do
        echo -e "Enter pair end reads 2 of lane $i (.fq or .fq.gz): \c"
        read VAR
        if [ "${VAR}" != "" ] && [ -f $VAR ]
        then
            NAMES="${NAMES} ${VAR}"
            break
        fi
        echo "Invalidate file path!"
    done
    i=$(($i+1))
done
fi

# Create log directory if necessory
if [ ! -d log ]
then
    mkdir log
fi

# Load configuration file
source "$CONF_PATH"

echo ""
echo ""
echo "------------------------------------------------------------"
# check fastq file version
quality=$(less $VAR | head -n 4 | tail -n 1)
if [ $FQ_VERSION = 1.5 ] && [ "$(echo $quality | grep [a-z])" != "" ]
then
	echo "That's OK! Pipeline begin at $(date)"
elif [ $FQ_VERSION != 1.5 ] && [ "$(echo $quality | grep [a-z])" = "" ]
then
	echo "That's OK! Pipeline begin at $(date)"
else
	echo "WARNING: fastq version seems not consistent with config"
	echo "         file specification. Continue anyway."
	echo "Pipeline begin at $(date)"
fi
echo ""
echo "To see details of each program's running message, please"
echo "refer to the log directory."
echo ""
echo "This pipeline may run several hours or several days, "
echo "wish you have a good time during these days~"
echo "------------------------------------------------------------"
echo ""

echo "Configuration:" >> log/journal
echo "Config file path          : $CONF_PATH" >> log/journal
echo "Platform                  : $PLATFORM" >> log/journal
echo "Remove intermediate files : $RM_INTER" >> log/journal
echo "Number of threads         : $THREAD_NUM" >> log/journal
echo "Insert size               : $LIB_LENGTH" >> log/journal
echo "Number of sample lanes    : $LANES" >> log/journal
echo "" >> log/journal

echo "Program Version:"                                | tee -a log/journal
echo "BWA      : $(bwa 2>&1 | grep Version | cut -d' ' -f2)" \
                                                       | tee -a log/journal
echo "GATK     : $(echo $GATK_HOME | cut -d'-' -f2)"   | tee -a log/journal
echo "Picard   : $(echo $PICARD_HOME | cut -d'-' -f3)" | tee -a log/journal
echo "Samtools : $(samtools 2>&1 | grep Version | cut -d' ' -f2)" \
                                                       | tee -a log/journal
echo "Pindel   : $(pindel | grep 'Pindel version' | uniq | cut -d' ' -f3 \
                                      | sed 's/,//')"  | tee -a log/journal
echo ""                                                | tee -a log/journal

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tPipeline begin\t0\t[OK]" \
| tee -a log/journal

# Run reads mapping
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tReads mapping begin\t0\t[OK]" \
| tee -a log/journal

$(dirname $(realpath $0))/reads_mapping/bwa_reads_mapping.sh $CONF_PATH \
$PLATFORM $RM_INTER $THREAD_NUM $LIB_LENGTH $LANES $NAMES \
2>>log/reads_mapping.log | tee -a log/journal

if [ ${PIPESTATUS[0]} -eq 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tReads mapping finished\t0\t[OK]" \
    | tee -a log/journal
else
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tReads mapping failed\t0\t[FAIL]" \
    | tee -a log/journal
    echo ""
    echo "------------------------------------------------------------"
    echo "Something bad happened. Details are in the log dirctory."
    echo "See you next time!"
    echo "------------------------------------------------------------"
    echo ""
    exit 1
fi

# Statistics on mapping result
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tMap result statistics begin\t0\t[OK]" \
| tee -a log/journal
echo "" | tee -a log/map_result.sta

samtools flagstat map_result.sorted.bam | sed 's/ + /_+_/' | sed 's/ /#/g' \
| sed 's/#/ | /' | sed 's/_/ /g' | sed 's/^/| /' | sed 's/$/ |/' | column -t \
| sed 's/#/ /g'  | tee -a log/map_result.sta

if [ ${PIPESTATUS[0]} -eq 0 ]
then
    echo "" | tee -a log/map_result.sta
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tMap result statistics finished\t0\t[OK]" \
    | tee -a log/journal
else
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tMap result statistics failed\t0\t[FAIL]" \
    | tee -a log/journal
    echo ""
    echo "------------------------------------------------------------"
    echo "Something bad happened. Details are in the log dirctory."
    echo "See you next time!"
    echo "------------------------------------------------------------"
    echo ""
    exit 1
fi



# Run call SNP and Indel
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall SNP and Indel begin\t0\t[OK]" \
| tee -a log/journal

$(dirname $(realpath $0))/call_variants/gatk_call_variants.sh $CONF_PATH \
$PLATFORM $RM_INTER $THREAD_NUM 2>>log/call_variants.log | tee -a log/journal

if [ ${PIPESTATUS[0]} -eq 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall SNP and Indel finished\t0\t[OK]" \
    | tee -a log/journal
else
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall SNP and Indel failed\t0\t[FAIL]" \
    | tee -a log/journal
    echo ""
    echo "------------------------------------------------------------"
    echo "Something bad happened. Details are in the log dirctory."
    echo "See you next time!"
    echo "------------------------------------------------------------"
    echo ""
    exit 2
fi

# Statistics on GATK call SNP and Indel result



# Run call structure variation
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall structure variation begin\t0\t[OK]" \
| tee -a log/journal

$(dirname $(realpath $0))/call_sv/pindel_call_sv.sh $CONF_PATH $PLATFORM \
$RM_INTER $THREAD_NUM $LIB_LENGTH 2>>log/call_sv.log | tee -a log/journal

if [ ${PIPESTATUS[0]} -eq 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall structure variation finished\t0\t[OK]" \
    | tee -a log/journal
else
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCall structure variation failed\t0\t[FAIL]" \
    | tee -a log/journal
    echo ""
    echo "------------------------------------------------------------"
    echo "Something bad happened. Details are in the log dirctory."
    echo "See you next time!"
    echo "------------------------------------------------------------"
    echo ""
    exit 3
fi

# Statistics on Pindel call structure variation



# Pipeline finished
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tPipeline finished\t0\t[OK]" \
| tee -a log/journal

echo ""
echo "------------------------------------------------------------"
echo "Congratulate! Pipeline finished successfully at $(date)."
echo "Bye!"
echo "------------------------------------------------------------"
echo ""

exit 0
