SeqPipe (a SEQuencing data analysis PIPEline framework)
=======================================================

- Version: 0.4.13 ($Rev$)
- Author: Linlin Yan (yanll@mail.cbi.pku.edu.cn)
- Copyright: 2012-2014, Centre for Bioinformatics, Peking University, China

Thank you very much for choosing SeqPipe! Please do not hesitate to email me if
you have _any_ question or suggestion about SeqPipe.

---------------------------------------------------------------------------

Table of Content
----------------

1. Introduction
2. Installation & Files
3. A Very Quick Start
4. A Real Example
5. Supported Statements
6. File Dependency
7. Pre-defined Pipelines
8. Known Limitations

---------------------------------------------------------------------------

1. Introduction
---------------

SeqPipe is a command line-based pipeline framework for bioinformatics research.
It has predefined many common useful pipelines for high throughput sequencing
data analysis and is very easy for both bioinformaticians and biology
researchers to launch different tools.

More importantly, SeqPipe could record as many related information as possible
to ensure the analysis procedure is _reproducible_, which is essential in
scientific research.

There are some features of SeqPipe, for which you may like to use it as your
handy framework in your daily data analysis.

1. **GNU bash-like syntax** - Defining a pipeline in SeqPipe is almost the same
   as writing a function in GNU bash. Most your pre-existed bash scripts may be
   very easy to migrate to SeqPipe framework, from which you will benefit a
   lot, such as logs and re-use, while keep the scripts as clear as possible.

2. **Logging automatically** - When running pipeline with SeqPipe, it will
   automatically record command lines, parameters, program versions, running
   time and other log files. All of those are useful and also important for you
   to track every step of your analysis, which could help you to make research
   results be reproducible.

3. **Run in parallel easily** - It is very easy to define which steps in a
   pipeline should be run in parallel, without adding any complexity to the
   scripts.

4. **File dependency checking** - SeqPipe could check input/output file
   dependency for each step, therefore those already finished steps could be
   skipped automatically, especially when you restart a pipeline after it was
   somehow aborted.

5. **Predefined pipelines** - SeqPipe predefined many common pipelines for high
   throughput sequencing data analysis, including read mapping and variant
   calling. They are easy-to-use for experienced bioinformaticians and also
   useful for newbie to start learning the workflows.

---------------------------------------------------------------------------

2. Installation & Files
-----------------------

SeqPipe is written in Perl and almost self-dependent, therefore it should be
easy and compatible to run on most Unix-like systems (both Linux & MacOS are
tested so far).

The installation is also very easy. Since SeqPipe is open source and hosted on
Google Code, you can use subversion to download and update it as you like.
Here goes the command line for subversion checkout:

    $ svn checkout http://seqpipe.googlecode.com/svn/trunk seqpipe

This will create a new directory 'seqpipe' and download all source code and
documents in it. After that, you can run it immediately as:

    $ cd seqpipe/
    $ ./seqpipe

Or you may add the directory to your bash environment variable $PATH for
convenience, and then use 'seqpipe' command directly.

Using "${SP_ROOT}" as this root directory of SeqPipe you just downloaded, The
main directories and files are introduced as following:

    ${SP_ROOT}/seqpipe           Main program of SeqPipe, written in Perl.

    ${SP_ROOT}/doc/              Documents including this file.
    ${SP_ROOT}/examples/         A quick start for you to learn.

    ${SP_ROOT}/default.pipe      Default pipeline, to record system basic info.
    ${SP_ROOT}/bioseq.pipe       Pre-defined pipelines for HTS data analysis.

    ${SP_ROOT}/seqpipe.history   Utility tool for manipulating log files.
    ${SP_ROOT}/uxcat             A wrapper script for cat/zcat/gzcat.
    ${SP_ROOT}/qsub_sync         Attempt for using SeqPipe in Torque cluster.

---------------------------------------------------------------------------

3. A Very Quick Start
---------------------

Read and run procedures in ${SP_ROOT}/examples/demo.pipe step by step would be
a wise choice to learn SeqPipe.

More details will be comming soon!

---------------------------------------------------------------------------

4. A Real Example
-----------------

Comming soon!

---------------------------------------------------------------------------

5. Supported Statements
-----------------------

Comming soon!

---------------------------------------------------------------------------

6. File Dependency
------------------

Comming soon!

---------------------------------------------------------------------------

7. Pre-defined Pipelines
------------------------

Comming soon!

---------------------------------------------------------------------------

8. Known Limitations
--------------------

Comming soon!

---------------------------------------------------------------------------
