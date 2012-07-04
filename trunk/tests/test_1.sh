#!/bin/bash
#
# This is a demo bash script for using pipeline directly.
#

. ../default.pipe

REFERENCE=chrM.fa
ALGORITHM=is
FASTQ_1=1.fq.gz
FASTQ_2=2.fq.gz
OUTPUT_BAM=bwa_mapping.bam
INSERT_SIZE=500

SP_run bwa_read_mapping $@
