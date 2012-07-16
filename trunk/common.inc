#!/bin/bash

###########################################################################
# This part provide some SeqPipe primitives (SP_xxx), used only in run bash
#   scripts directly.

#[primitive]
# Usage:
#   SP_run <procedure> [NAME1=value1 ...]
function SP_run
{
	PROC_NAME=$1
	shift

	if [ -z "${PROC_NAME}" ]; then
		echo "Bad use of SP_run!" >/dev/stderr
		set -e
		exit 1
	fi

	if [ -z "$(type ${PROC_NAME} 2>/dev/null | grep 'function')" ]; then
		echo "Procedure '${PROC_NAME}' does not exist!" >/dev/stderr
		set -e
		exit 1
	fi

	( # Localize options to disable modifying options outside
		while [ $# -gt 0 ]; do
			if [ -z "$(echo $1 | egrep '^\w\w*=.*$')" ]; then
				echo "Bad option '$1' for procedure '${PROC_NAME}'!" >/dev/stderr
				set -e
				exit 1
			fi
			export $1
			shift
		done

		${PROC_NAME}
	)
}

#[primitive]
# Usage:
#   SP_set NAME=value [<checker> [NAME1=value1 ...]]
function SP_set
{
	SET_OPTION=$1
	shift

	if [ -z "${SET_OPTION}" ]; then
		echo "Bad use of SP_set!" >/dev/stderr
		set -e
		exit 1
	fi

	if [ $# -eq 0 ]; then
		export ${SET_OPTION}
	else
		SP_run $@ && export ${SET_OPTION}
	fi
}

#[primitive]
# Usage:
#   SP_eval NAME <evaluator> [NAME1=value1 ...]
function SP_eval
{
	OPTION_NAME=$1
	shift

	if [ -z "${OPTION_NAME}" -o $# -eq 0 ]; then
		echo "Bad use of SP_eval!" >/dev/stderr
		set -e
		exit 1
	fi

	export ${OPTION_NAME}=$(SP_run $@)
}

#[primitive]
function SP_parallel_begin
{
	# In running directly in bash, we do not support parallel mode
	:
}

#[primitive]
function SP_parallel_end
{
	# In running directly in bash, we do not support parallel mode
	:
}

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
	test -n "$(less ${INPUT_FASTQ} | sed -n '4~4p' | head -n1000 | grep '[0-9]')"
}

#[procedure type="checker"]
#[procedure input="${INPUT_FASTQ}"]
function if_fastq_is_not_base_33
{
	# Pick up first ten reads's quality to see if it contains number.
	test -z "$(less ${INPUT_FASTQ} | sed -n '4~4p' | head -n1000 | grep '[0-9]')"
}

###########################################################################

