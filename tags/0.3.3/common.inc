#!/bin/bash

###########################################################################

#[procedure type="checker"]
function if_has_arg
{
	test -n "${ARG}"
}

#[procedure type="checker"]
function if_no_arg
{
	test -z "${ARG}"
}

#[procedure type="evaluator"]
function get_file_name
{
	basename ${FILE} | sed 's,\.[^.]*$,,'
}

#[procedure type="evaluator"]
function get_file_date
{
	stat ${FILE} | grep Modify | awk '{print $2}' | sed 's,-,,g'
}

###########################################################################

#[procedure type="checker"]
#[procedure input="${INPUT_FASTA}"]
function if_fasta_is_long_genome
{
	# Here we assume a genome is long if its file is bigger than 100Mb
	test -n "$(ls -l ${INPUT_FASTA} | awk '$5>=1e8')"
}

#[procedure type="checker"]
#[procedure input="${INPUT_FASTQ}"]
function if_fastq_is_base_33
{
	# Pick up first ten reads's quality to see if it contains number.
	test -n "$(${SEQPIPE_ROOT}/uncompress_cat ${INPUT_FASTQ} | head -n4000 | sed -n '4~4p' | grep '[0-9]')"
}

#[procedure type="checker"]
#[procedure input="${INPUT_FASTQ}"]
function if_fastq_is_not_base_33
{
	# Pick up first ten reads's quality to see if it contains number.
	test -z "$(${SEQPIPE_ROOT}/uncompress_cat ${INPUT_FASTQ} | head -n4000 | sed -n '4~4p' | grep '[0-9]')"
}

###########################################################################
