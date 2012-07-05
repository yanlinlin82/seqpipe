SeqPipe (a SEQuencing data analsysis PIPEline framework)

Version: 0.1.0 ($Rev$)
Copyright: 2012, Centre for Bioinformatics, Peking University, China

$Id$

---------------------------------------------------------------------------

Table of Content

1. Goal & Philosiphy
2. Directories & Files
3. Demo Pipeline
4. Others
5. Problems

---------------------------------------------------------------------------

1. Goal & Philosiphy

   SeqPipe is designed for establishment of bioinformatics sequencing  data
analysis pipeline. Some of the features list as follows:
   (1) Data analysis  steps  should be  declared  clearly,  as well as  for
re-using easily.
   (2) Every step that executed should be recorded precisely, including all
programs' version, executing date and time, successful or not, etc.
   (3) In addition to presenting the standard pipelines,  it should be easy
to change some of its parameters to form new pipelines.

   Therefore, GNU bash is chosen for pipeline itself, while Perl is used to
create a shell explainer for the pipeline.

---------------------------------------------------------------------------

2. Directories & Files

   "$" stands for the root directory of project SeqPipe.

   $/doc/                Directory for documents, including this file.

   $/seqpipe             Main and the only program, written by Perl.

   $/default.pipe        Default pipeline declaration, written by GNU Bash.

   $/tests/              Test demo script and data files.

---------------------------------------------------------------------------

3. Demo Pipeline

   See as $/test/test_*.sh

---------------------------------------------------------------------------

4. Others

Variables are supported, for example:

#[procedure type="evaluator"]
function get_temp_file
{
	mktemp
}

#[procedure type="stage"]
function foo
{
	SP_eval TMP_FILE get_temp_file
	ls > ${TMP_FILE}
	cat ${TMP_FILE}
	wc ${TMP_FILE}
	rm ${TMP_FILE}
}

---------------------------------------------------------------------------

5. Problems

   If your perl does not support threads, and you do not want to re-install
perl, you may use 'forks' packages instead.

