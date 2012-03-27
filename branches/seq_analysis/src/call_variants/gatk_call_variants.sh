#!/bin/bash

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
   
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# This script implements the gatk call SNP and indel protocol.
# Written by Wang Meng, 2012-03-18.

# The input arguments list is as follows:
# <config file> <platform> <whether remove intermediate files> 
# <number of threads>

# Input: map_result.sorted.bam
# Finial output: flt.snp.vcf flt.indel.vcf


if [ $# -ne 4 ]
then
    echo -e "Usage: $0 <config file> <platform> <whether remove intermediate\c"
    echo " files> <number of threads>"
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


# get the unique mapped reads
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin get unique mapped reads\t0\t[OK]"
time samtools view -h map_result.sorted.bam | egrep 'XT:A:U|^@' \
| samtools view -Sb - > map_result.sorted.unique.bam.tmp
if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when get \
    unique mapped reads\t0\t[FAIL]"
    exit 1
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, get unique mapped \
reads finished successfully\t0\t[OK]"

# Reorderbam by picard
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin reorder bam by picard\
\t0\t[OK]"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar $PICARD_HOME/ReorderSam.jar \
    INPUT=map_result.sorted.unique.bam.tmp \
    OUTPUT=map_result.sorted.unique.bam \
    REFERENCE="${GATK_REF}" 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when reorder \
    bam \t0\t[FAIL]"
    exit 1
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, reorder bam \
finished successfully\t0\t[OK]"
rm -f map_result.sorted.unique.bam.tmp


# mark duplicates by picard
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin mark duplicates by picard\
\t0\t[OK]"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar $PICARD_HOME/MarkDuplicates.jar \
    INPUT=map_result.sorted.unique.bam \
    OUTPUT=map_result.sorted.unique.markdup.bam \
    METRICS_FILE=bwa_result.sorted.unique.markdup.metrix \
    REMOVE_DUPLICATES=true \
    CREATE_INDEX=true 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when mark \
    duplicates\t0\t[FAIL]"
    exit 2
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, mark duplicates \
finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.bam
fi

# Realignment by GATK
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin realignment by GATK\t0\t[OK]"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T RealignerTargetCreator \
    -R "${GATK_REF}" \
    -I map_result.sorted.unique.markdup.bam \
    -o map_result.sorted.unique.markdup.realign.intervals \
    -known "$DBSNP_PATH" \
    --num_threads $THREAD_NUM 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    realignment\t0\t[FAIL]"
    exit 3
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T IndelRealigner \
    -R "${GATK_REF}" \
    -I map_result.sorted.unique.markdup.bam \
    -targetIntervals map_result.sorted.unique.markdup.realign.intervals \
    -o map_result.sorted.unique.markdup.realign.bam 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    realignment\t0\t[FAIL]"
    exit 4
fi

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, realignment \
finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.intervals
    rm -f map_result.sorted.unique.markdup.bam
    rm -f map_result.sorted.unique.markdup.bai
fi


# Fix mate information by Picard
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin fix mate info by Picard\t0\t[OK]"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar \
    $PICARD_HOME/FixMateInformation.jar \
	CREATE_INDEX=true \
    INPUT=map_result.sorted.unique.markdup.realign.bam \
    OUTPUT=map_result.sorted.unique.markdup.realign.fixmate.bam 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when fix mate \
    info\t0\t[FAIL]"
    exit 5
fi
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, fix mate info \
finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.bam
	rm -f map_result.sorted.unique.markdup.realign.bai
fi


# Recalibration by GATK
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin recalibration by GATK\t0\t[OK]"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T CountCovariates \
    -R "${GATK_REF}" \
    -I map_result.sorted.unique.markdup.realign.fixmate.bam \
    -knownSites "$DBSNP_PATH" \
    -cov ReadGroupCovariate \
    -cov QualityScoreCovariate \
    -cov CycleCovariate \
    -cov DinucCovariate \
    -recalFile map_result.sorted.unique.markdup.realign.fixmate.recal.csv \
    --num_threads $THREAD_NUM 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    recalibration\t0\t[FAIL]"
    exit 6
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T TableRecalibration \
    -R "${GATK_REF}" \
    -I map_result.sorted.unique.markdup.realign.fixmate.bam \
    -recalFile map_result.sorted.unique.markdup.realign.fixmate.recal.csv \
    -o map_result.sorted.unique.markdup.realign.fixmate.recal.bam 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    recalibration\t0\t[FAIL]"
    exit 7
fi

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, recalibration \
finished successfully\t0\t[OK]"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.fixmate.bam
	rm -f map_result.sorted.unique.markdup.realign.fixmate.bai
    rm -f map_result.sorted.unique.markdup.realign.fixmate.recal.csv
fi



# Call Raw variants by GATK
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin Call Raw variants by \
GATK\t0\t[OK]"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T UnifiedGenotyper \
    -R "${GATK_REF}" \
    -I map_result.sorted.unique.markdup.realign.fixmate.recal.bam \
    -glm BOTH \
    --min_base_quality_score 20 \
    --dbsnp "$DBSNP_PATH" \
    -stand_call_conf $STAND_CALL_CONF \
    -stand_emit_conf $STAND_EMIT_CONF \
    -o raw_snp_indel.vcf \
    --num_threads $THREAD_NUM 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when Call \
    Raw variants by GATK\t0\t[FAIL]"
    exit 8
fi

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, Call Raw variants by \
GATK finished successfully\t0\t[OK]"

# Variant filtration
echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tBegin variant filtration\t0\t[OK]"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T SelectVariants \
    -R "${GATK_REF}" \
    --variant raw_snp_indel.vcf \
    -selectType SNP \
    -selectType MNP \
    -o raw.snp.vcf 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    select SNP\t0\t[FAIL]"
    exit 9
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T SelectVariants \
    -R "${GATK_REF}" \
    --variant raw_snp_indel.vcf \
    -selectType INDEL \
    -o raw.indel.vcf 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured \
    when select indel\t0\t[FAIL]"
    exit 10
fi

# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f raw_snp_indel.vcf
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T VariantFiltration \
    -R "${GATK_REF}" \
    --variant raw.snp.vcf \
    -o flt.snp.vcf \
    --clusterWindowSize 10 \
    --filterExpression "MQ0>=4&&((MQ0/(1.0*DP))>0.1)" \
    --filterName "HARD_TO_VALIDATE" 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured \
    when filter SNP\t0\t[FAIL]"
    exit 11
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T VariantFiltration \
    -R "${GATK_REF}" \
    --variant raw.indel.vcf \
    -o flt.indel.vcf \
    --filterExpression "MQ0>=4&&((MQ0/(1.0*DP))>0.1)" \
    --filterName "HARD_TO_VALIDATE" \
    --filterExpression "QUAL<10" \
    --filterName "QualFilter" 1>&2

if [ $? != 0 ]
then
    echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tOops, error occured when \
    filter indel\t0\t[FAIL]"
    exit 12
fi

echo -e "$(date '+%Y-%m-%d %H:%M:%S')\tCongratulates, Variant filtration \
by GATK finished successfully\t0\t[OK]"


exit 0
