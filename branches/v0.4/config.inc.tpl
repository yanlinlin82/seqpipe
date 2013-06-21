#!/bin/bash

###########################################################################
# This part defines global variables.

TMP_DIR=/tmp                            # Temporary directory

JAVA_MAX_MEM_SIZE=2G                    # For Java's -Xmx option
JAVA_GC_THREAD_NUM=2                    # For Java's -XX:ParallelGCThreads option
JAVA_OPTS=                              # For other extensions
_JAVA_OPTS=-Xmx${JAVA_MAX_MEM_SIZE} -XX:ParallelGCThreads=${JAVA_GC_THREAD_NUM} -Djava.io.tmpdir=${TMP_DIR} ${JAVA_OPTS}

# Picard options
PICARD_ROOT=/tools/picard-tools         # Path of picard tools
MAX_RECORDS_IN_RAM=500000               # For enhance performance of picard
PICARD_OPTS=                            # For other extensions
_PICARD_OPTS=MAX_RECORDS_IN_RAM=${MAX_RECORDS_IN_RAM} ${PICARD_OPTS}

# GATK options
GATK_ROOT=/tools/gatk                                          # Path of GATK tools
GATK_BUNDLE_ROOT=/data/gatk_bundle/1.5/b37
GATK_VCF_DBSNP=${GATK_BUNDLE_ROOT}/dbsnp_135.b37.vcf           # Path of GATK dbSNP VCF file
GATK_VCF_HAPMAP=${GATK_BUNDLE_ROOT}/hapmap_3.3.b37.sites.vcf   # Path of GATK HapMap VCF file
GATK_VCF_OMNI=${GATK_BUNDLE_ROOT}/1000G_omni2.5.b37.sites.vcf  # Path of GATK 1000G OMNI VCF file
GATK_OPTS=
_GATK_OPTS=${GATK_OPTS}

###########################################################################

