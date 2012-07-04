#!/bin/bash

. ../default.pipe

INPUT_BAM=abc.sorted.bam
OUTPUT_PREFIX=xyz_sv
REFERENCE=chrM.fa
SAMPLE_NAME=xyz
INSERT_SIZE=500

SP_run pindel_call_structure_variants $@
