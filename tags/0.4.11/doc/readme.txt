SeqPipe (a SEQuencing data analsysis PIPEline framework)

Version: 0.4.0 ($Rev$)
Author: Linlin Yan (yanll@mail.cbi.pku.edu.cn)
Copyright: 2012, Centre for Bioinformatics, Peking University, China

$Id$

---------------------------------------------------------------------------

Table of Content

1. Goal & Philosiphy
2. Directories & Files
3. Basic Syntex of Pipeline Declaration
4. Evaluation in Run-time

---------------------------------------------------------------------------

1. Goal & Philosophy

   SeqPipe is designed for establishment of bioinformatics sequencing data
analysis pipeline. Some of the features list as follows:
   (1) Data analysis steps should be declared clearly and easy to re-use.
   (2) Every executed step should be recorded precisely, including all the
programs' version and options, executing date and time, successful or not,
etc.
   (3) In addition to presenting the standard pipelines, it should be easy
to change some of its parameters to form new pipelines.
   (4) When pipeline was somehow aborted, it should be easy to continue the
procedures by skipping already finished steps automatically.
   (5) Parallel mode are easily to be used.

   GNU bash is chosen for pipeline itself, while Perl is used to create a
shell explainer for the pipeline.

---------------------------------------------------------------------------

2. Directories & Files

   "${SP_ROOT}" stands for the root directory of project SeqPipe.

   ${SP_ROOT}/seqpipe         Main and the only program, written by Perl.

   ${SP_ROOT}/doc/            Directory for documents, including this file.

   ${SP_ROOT}/default.pipe    Default pipeline, to record system basic info.

   ${SP_ROOT}/bioseq.pipe     Pipeline declaration.

   ${SP_ROOT}/config.inc.tpl  Template for config.inc, in which user can
                                customize global options.

   ${SP_ROOT}/tests/          Test demo script and data files.

---------------------------------------------------------------------------

3. Basic Syntex of Pipeline Declaration

3.1. Pipeline Module File

   A pipeline module file consists of one or more procedures, which are
written as GNU bash functions. The module files could be loaded by '-m'
option. By default, SeqPipe will load all '*.pipe' in ${SP_ROOT} directory
automatically.

   Procedures could be listed by SeqPipe when type command 'seqpipe -l'.

3.2. Procedure Declaration

   Comments (format as '#[name="value" ...]') could be added right
before the function declarations or each command in the functions to
decleare attributes (such as requires, inputs and outputs).

#[...]
function procedure_name_XXX
{
	#[...]
	command_to_run options
	...
}

   All those comments are optional. You may write an procedure directly
like this (in a file named test.pipe, e.g.):

function bwa_read_mapping
{
	bwa index MT.fa
	bwa aln MT.fa 1.fq.gz > 1.fq.gz.sai
	bwa aln MT.fa 2.fq.gz > 2.fq.gz.sai
	bwa sampe MT.fa 1.fq.gz.sai 2.fq.gz.sai 1.fq.gz 2.fq.gz \
		| samtools view -Sb - > out.bam
}

just as in GNU bash script. Then you can found it in the procedure list
when you type command 'seqpipe -m test.pipe -l'.

   You can run it by command 'seqpipe -m test.pipe bwa_read_mapping',
which should be the same as you run 'bwa_read_mapping' in the bash script.
In addition, Seqpipe will record everything for you automatically.

   To be noticed, for the command line which has more than one line, a
tailing '\' should be added to tell SeqPipe to append next line to the
command.

3.3. Attributes (Requires, Inputs and Outputs)

   Supported attributes for procedures and commands are: "require",
"input", "output". For example:

#[require="hg19.fa"]
#[input="1.fq" input="2.fq"]
#[output="out.bam"]
function bwa_read_mapping
{
	#[input="MT.fa" output="MT.fa.bwt"]
	bwa index MT.fa

	#[require="MT.fa" input="1.fq.gz" output="1.fq.gz.sai"]
	bwa aln MT.fa 1.fq.gz > 1.fq.gz.sai
	#[require="MT.fa" input="2.fq.gz" output="2.fq.gz.sai"]
	bwa aln MT.fa 2.fq.gz > 2.fq.gz.sai

	#[require="MT.fa"]
	#[input="1.fq.gz" input="1.fq.gz.sai"]
	#[input="2.fq.gz" input="2.fq.gz.sai"]
	#[output="out.bam"]
	bwa sampe MT.fa 1.fq.gz.sai 2.fq.gz.sai 1.fq.gz 2.fq.gz \
		| samtools view -Sb - > out.bam
}

   Those comments (attributes) could be written together in a single line,
like this:
  #[input="1.fq" input="2.fq"]
or written each attribute in separated line, like this:
  #[input="1.fq"]
  #[input="2.fq"]
The meanings are exact the same.

Then, SeqPipe will skip previous finished steps according to those
attributes. Before running a procedure or a command, seqpipe will ensure
that every require file and input file exists and check if all output files
exist and are news than every input file. if any of the output files missed
or out-of-date, the procedure or command will be executed.

   If any command failed (with non-zero return value, $? in GNU bash),
SeqPipe will remove its output files to avoid those unfinished files
mislead the judgement of command skipping when run SeqPipe again. However,
output attributes of procedure

3.4. Variables

   Variables are the format '${XXXX}'. You can change some characters to
variables in you procedure to make it easy to re-use. For example:

function bwa_read_mapping
{
	bwa index ${REFERENCE}
	bwa aln ${REFERENCE} ${FASTQ_1} > ${FASTQ_1}.sai
	bwa aln ${REFERENCE} ${FASTQ_2} > ${FASTQ_2}.sai
	bwa sampe ${REFERENCE} ${FASTQ_1}.sai ${FASTQ_2}.sai \
		${FASTQ_1} ${FASTQ_2} \
		| samtools view -Sb - > ${OUTPUT}
}

   Then you can run command like this:

$ seqpipe -m test.pipe bwa_read_mapping bwa_read_mapping \
	REFERENCE=MT.fa FASTQ_1=1.fq.gz FASTQ_2=2.fq.gz OUTPUT=out.bam

3.5. Global Variables

   Outside those functions, you can declaration global variables as the
format: NAME=VALUE (no space beside the equal mark).

3.6. Primitives

   There are a few primitives in SeqPipe:
      SP_set
      SP_run
      SP_if / SP_else
      SP_for
      SP_while

---------------------------------------------------------------------------

4. Evaluation in Run-time

Embeded bash command in SP_set are supported, for example:

#[procedure type="stage"]
function foo
{
	SP_set TMP_FILE=$(mktemp)
	ls > ${TMP_FILE}
	cat ${TMP_FILE}
	wc ${TMP_FILE}
	rm ${TMP_FILE}
}

---------------------------------------------------------------------------
