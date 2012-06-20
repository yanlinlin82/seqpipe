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
MAX_INSERT_SIZE=200

SP_run bwa_reads_mapping $@


function bwa_reads_mapping
{
	SP_set QUAL_OPTION "-I" "${QUALITY_FORMAT_BASE}==64"
	SP_run bwa_build_index REFERENCE=${REFERENCE}
	SP_Parallel_Begin 2
	bwa aln -t ${THREAD_NUM} -i ${BWA_END_IND} -e ${BWA_GAP_EXT} \
		${REFERENCE} ${INPUT_2} -f ${INPUT_1}.sai ${QUAL_OPTION} &
	bwa aln -t ${THREAD_NUM} -i ${BWA_END_IND} -e ${BWA_GAP_EXT} \
		${REFERENCE} ${INPUT_2} -f ${INPUT_2}.sai ${QUAL_OPTION} &
	SP_Parallep_End
	bwa sampe -P ${REFERENCE} -a ${MAX_INSERT_SIZE} \
		${INPUT_2}.sai ${INPUT_2}.sai ${INPUT_1} ${INPUT_2} \
		| samtools view -Sb - \
		> ${OUTPUT}
}
