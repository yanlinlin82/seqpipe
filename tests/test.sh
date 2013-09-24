#!/bin/bash
#
# This is a demo bash script for using pipeline directly.
#

. ../default.pipe

REFERENCE=chrM.fa
ALGORITHM=is
INPUT_1=1.fq.gz
INPUT_2=2.fq.gz
OUTPUT=bwa_mapping.bam
MAX_INSERT_SIZE=500

SP_run bwa_reads_mapping $@
