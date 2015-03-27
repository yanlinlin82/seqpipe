# SeqPipe

a SEQuencing data analysis PIPEline framework

## Introduction

SeqPipe is a command line-based pipeline framework for bioinformatics research. It has predefined many common useful pipelines for high throughput sequencing data analysis and is very easy for both bioinformaticians and biology researchers to launch different tools.

More importantly, SeqPipe could record as many related information as possible to ensure the analysis procedure is reproducible, which is essential in scientific research.

## Features

There are some features of SeqPipe, for which you may like to use it as your handy framework in your daily data analysis.

- **GNU bash-like syntax** - Defining a pipeline in SeqPipe is almost the same as writing a function in GNU bash. Most your pre-existed bash scripts may be very easy to migrate to SeqPipe framework, from which you will benefit a lot, such as logs and re-use, while keep the scripts as clear as possible.
- **Logging automatically** - When running pipeline with SeqPipe, it will automatically record command lines, parameters, program versions, running time and other log files. All of those are useful and also important for you to track every step of your analysis, which could help you to make research results be reproducible.
- **Run in parallel easily** - It is very easy to define which steps in a pipeline should be run in parallel, without adding any complexity to the scripts.
- **File dependency checking** - SeqPipe could check input/output file dependency for each step, therefore those already finished steps could be skipped automatically, especially when you restart a pipeline after it was somehow aborted.
- **Predefined pipelines** - SeqPipe predefined many common pipelines for high throughput sequencing data analysis, including read mapping and variant calling. They are easy-to-use for experienced bioinformaticians and also useful for newbie to start learning the workflows.

## Quick Start

1. Install by git (recommended, easy to update new version):

        git clone http://github.com/yanlinlin82/seqpipe /path/to/install/seqpipe/
        export PATH=$PATH:/path/to/install/seqpipe/
    
    or install by wget (or other downloader):
    
        wget -N http://github.com/yanlinlin82/seqpipe/archive/master.zip
        unzip master.zip
        mv seqpipe-master /path/to/install/seqpipe/
        export PATH=$PATH:/path/to/install/seqpipe/

2. Write a simple pipeline:

        cat <<EOF> foo.pipe
        foo() {
            echo "Hello, world!"
            date
        }
        EOF

3. Run the pipeline:

        seqpipe -m foo.pipe foo

4. Check the log files:

        ls -l -R .seqpipe/
    
