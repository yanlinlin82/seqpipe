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

# Finial output: flt.snp.vcf flt.indel.vcf


if [ $# -ne 4 ]
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


# get the unique mapped reads
echo "+-------------------------------------+"
echo "|    Begin get unique mapped reads    |"
echo "+-------------------------------------+"
time samtools view map_result.sorted.bam | egrep 'XT:A:U|^@' \
| samtools view -Sb - > map_result.sorted.unique.bam
if [ $? != 0 ]
then
    echo "+--------------------------------------------------------+"
    echo "|    Oops, error occured when get unique mapped reads    |"
    echo "+--------------------------------------------------------+"
    exit 1
fi
echo "+--------------------------------------------------------------------+"
echo "|    Congratulates, get unique mapped reads finished successfully    |"
echo "+--------------------------------------------------------------------+"


# mark duplicates by picard
echo "+---------------------------------------+"
echo "|    Begin mark duplicates by picard    |"
echo "+---------------------------------------+"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar $PICARD_HOME/MarkDuplicates.jar \
    INPUT=map_result.sorted.unique.bam \
    OUTPUT=map_result.sorted.unique.markdup.bam \
    METRICS_FILE=bwa_result.sorted.unique.markdup.metrix \
    REMOVE_DUPLICATES=true \
    CREATE_INDEX=true 1>&2

if [ $? != 0 ]
then
    echo "+------------------------------------------------+"
    echo "|    Oops, error occured when mark duplicates    |"
    echo "+------------------------------------------------+"
    exit 2
fi
echo "+------------------------------------------------------------+"
echo "|    Congratulates, mark duplicates finished successfully    |"
echo "+------------------------------------------------------------+"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.bam
fi

# Realignment by GATK
echo "+---------------------------------+"
echo "|    Begin realignment by GATK    |"
echo "+---------------------------------+"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T RealignerTargetCreator \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    -I map_result.sorted.unique.markdup.bam \
    -o map_result.sorted.unique.markdup.realign.intervals \
    -known "$DBSNP_PATH" \
    --num_threads $THREAD_NUM 1>&2

if [ $? != 0 ]
then
    echo "+--------------------------------------------+"
    echo "|    Oops, error occured when realignment    |"
    echo "+--------------------------------------------+"
    exit 3
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T IndelRealigner \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    -I map_result.sorted.unique.markdup.bam \
    -targetIntervals map_result.sorted.unique.markdup.realign.intervals \
    -o map_result.sorted.unique.markdup.realign.bam 1>&2

if [ $? != 0 ]
then
    echo "+--------------------------------------------+"
    echo "|    Oops, error occured when realignment    |"
    echo "+--------------------------------------------+"
    exit 4
fi

echo "+--------------------------------------------------------+"
echo "|    Congratulates, realignment finished successfully    |"
echo "+--------------------------------------------------------+"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.intervals
    rm -f map_result.sorted.unique.markdup.bam
    rm -f map_result.sorted.unique.markdup.bai
fi


# Fix mate information by Picard
echo "+-------------------------------------+"
echo "|    Begin fix mate info by Picard    |"
echo "+-------------------------------------+"

time java -Xmx8G -XX:ParallelGCThreads=4 -jar \
    $PICARD_HOME/FixMateInformation.jar \
    INPUT=map_result.sorted.unique.markdup.realign.bam \
    OUTPUT=map_result.sorted.unique.markdup.realign.fixmate.bam 1>&2

if [ $? != 0 ]
then
    echo "+----------------------------------------------+"
    echo "|    Oops, error occured when fix mate info    |"
    echo "+----------------------------------------------+"
    exit 5
fi
echo "+----------------------------------------------------------+"
echo "|    Congratulates, fix mate info finished successfully    |"
echo "+----------------------------------------------------------+"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.bam
fi


# Recalibration by GATK
echo "+-----------------------------------+"
echo "|    Begin recalibration by GATK    |"
echo "+-----------------------------------+"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T CountCovariates \
    -R "${REF_PATH}/${REF_NAME}.fa" \
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
    echo "+----------------------------------------------+"
    echo "|    Oops, error occured when recalibration    |"
    echo "+----------------------------------------------+"
    exit 6
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T TableRecalibration \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    -I map_result.sorted.unique.markdup.realign.fixmate.bam \
    -recalFile map_result.sorted.unique.markdup.realign.fixmate.recal.csv \
    -o map_result.sorted.unique.markdup.realign.fixmate.recal.bam 1>&2

if [ $? != 0 ]
then
    echo "+----------------------------------------------+"
    echo "|    Oops, error occured when recalibration    |"
    echo "+----------------------------------------------+"
    exit 7
fi

echo "+----------------------------------------------------------+"
echo "|    Congratulates, recalibration finished successfully    |"
echo "+----------------------------------------------------------+"
# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f map_result.sorted.unique.markdup.realign.fixmate.bam
    rm -f map_result.sorted.unique.markdup.realign.fixmate.recal.csv
fi



# Call Raw variants by GATK
echo "+---------------------------------------+"
echo "|    Begin Call Raw variants by GATK    |"
echo "+---------------------------------------+"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T UnifiedGenotyper \
    -R "${REF_PATH}/${REF_NAME}.fa" \
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
    echo "+----------------------------------------------------------+"
    echo "|    Oops, error occured when Call Raw variants by GATK    |"
    echo "+----------------------------------------------------------+"
    exit 8
fi

echo "+----------------------------------------------------------------------+"
echo "|    Congratulates, Call Raw variants by GATK finished successfully    |"
echo "+----------------------------------------------------------------------+"

# Variant filtration
echo "+--------------------------------+"
echo "|    Begin variant filtration    |"
echo "+--------------------------------+"

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T SelectVariants \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    --variant raw_snp_indel.vcf \
    -selectType SNP \
    -selectType MNP \
    -o raw.snp.vcf 1>&2

if [ $? != 0 ]
then
    echo "+-------------------------------------------+"
    echo "|    Oops, error occured when select SNP    |"
    echo "+-------------------------------------------+"
    exit 9
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T SelectVariants \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    --variant raw_snp_indel.vcf \
    -selectType INDEL \
    -o raw.indel.vcf 1>&2

if [ $? != 0 ]
then
    echo "+---------------------------------------------+"
    echo "|    Oops, error occured when select indel    |"
    echo "+---------------------------------------------+"
    exit 10
fi

# remove intermediate files if possible
if [ $RM_INTER == 'y' ] || [ $RM_INTER == 'Y']
then
    rm -f raw_snp_indel.vcf
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T VariantFiltration \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    --variant raw.snp.vcf \
    -o flt.snp.vcf \
    --clusterWindowSize 10 \
    --filterExpression "MQ0>=4&&((MQ0/(1.0*DP))>0.1)" \
    --filterName "HARD_TO_VALIDATE" 1>&2

if [ $? != 0 ]
then
    echo "+-------------------------------------------+"
    echo "|    Oops, error occured when filter SNP    |"
    echo "+-------------------------------------------+"
    exit 11
fi

time java -Xmx8G -jar $GATK_HOME/GenomeAnalysisTK.jar -et NO_ET \
    -T VariantFiltration \
    -R "${REF_PATH}/${REF_NAME}.fa" \
    --variant raw.indel.vcf \
    -o flt.indel.vcf \
    --filterExpression "MQ0>=4&&((MQ0/(1.0*DP))>0.1)" \
    --filterName "HARD_TO_VALIDATE" \
    --filterExpression "QUAL<10" \
    --filterName "QualFilter" 1>&2

if [ $? != 0 ]
then
    echo "+---------------------------------------------+"
    echo "|    Oops, error occured when filter indel    |"
    echo "+---------------------------------------------+"
    exit 12
fi

echo "+-----------------------------------------------------------------------+"
echo "|    Congratulates, Variant filtration by GATK finished successfully    |"
echo "+-----------------------------------------------------------------------+"


exit 0
