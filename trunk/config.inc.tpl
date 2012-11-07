#!/bin/bash

###########################################################################
# This part defines global variables.

TMP_DIR=/rd/tmp                         # Temporary directory
JAVA_MAX_MEM_SIZE=2G                    # For Java's -Xmx option
JAVA_GC_THREAD_NUM=2                    # For Java's -XX:ParallelGCThreads option
JAVA_OPTS="-Xmx${JAVA_MAX_MEM_SIZE} -XX:ParallelGCThreads=${JAVA_GC_THREAD_NUM} -Djava.io.tmpdir=${TMP_DIR}"

# Picard options
PICARD_ROOT=/rd/build/picard-tools       # Path of picard tools
MAX_RECORDS_IN_RAM=500000                # For enhance performance of picard
PICARD_OPTS="MAX_RECORDS_IN_RAM=${MAX_RECORDS_IN_RAM}"

# GATK options
GATK_ROOT=/rd/build/gatk                                # Path of GATK tools
GATK_KEY=${GATK_ROOT}/gatk_cbi.key                      # Key file for offline using GATK
GATK_BUNDLE_ROOT=/rd/data/public/gatk_bundle/1.5/b37
GATK_VCF_DBSNP=${GATK_BUNDLE_ROOT}/dbsnp_135.b37.vcf           # Path of GATK dbSNP VCF file
GATK_VCF_HAPMAP=${GATK_BUNDLE_ROOT}/hapmap_3.3.b37.sites.vcf   # Path of GATK HapMap VCF file
GATK_VCF_OMNI=${GATK_BUNDLE_ROOT}/1000G_omni2.5.b37.sites.vcf  # Path of GATK 1000G OMNI VCF file
GATK_OPTS="-et NO_ET -K ${GATK_KEY}"

###########################################################################

