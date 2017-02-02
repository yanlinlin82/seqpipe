# User Guide of SeqPipe

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


## Requirements

The following lists software required in SeqPipe:

* [GNU bash](https://www.gnu.org/software/bash/): including commands 'which', 'date' and 'pwd'.
* [GNU core utilities](https://www.gnu.org/software/coreutils/coreutils.html): including commands 'tee' and 'uname'.
* [procps](https://sourceforge.net/projects/procps-ng/): including command 'free'
* [util-linux](https://www.kernel.org/pub/linux/utils/util-linux/): including command 'lscpu'

## Installation

There are different ways to install SeqPipe. Choose any one you like:

* (Method 1): Install from GitHub source:

        $ git clone https://github.com/yanlinlin82/seqpipe -b cpp-v0.5 --single-branch
        $ make -C seqpipe/
        $ sudo cp -av seqpipe/seqpipe /usr/bin/   # or other directory set in PATH
        $ rm -rf seqpipe/                         # (optionally) clean up

* (Method 2): Install by pre-compiled executable files:

        <<TODO>>: To be completed.


## Quick Start

1. The easiest way to use `seqpipe` is run shell command with prefix `seqpipe run`:

        $ seqpipe run echo 'Hello, world!'

    Log files can be checked in `.seqpipe` directory:

        $ ls -lR .seqpipe/

2. Pipeline can also be written into file as:

        $ cat <<EOF> hello.pipe
        echo "Hello, world!"
        sleep 1
        echo "Goodbye!"
        EOF

    Then run:

        $ seqpipe run hello.pipe

    Check log files:

        $ ls -l .seqpipe/last/

    The pipeline file can be passed by pipe like:

        $ seqpipe run - < hello.pipe
        $ seqpipe run <(cat hello.pipe)
        $ cat hello.pipe | seqpipe run -  # all these three commands are the same as above


## Usage Examples

1. Write workflow pipeline as bash function:

        $ cat <<EOF> foo.pipe
        hello() {
            echo "Hello, world!"
            sleep 1
            echo "Goodbye!"
        }
        EOF

        $ seqpipe foo.pipe hello     # specify 'hello', the name of procedure to run

    Or:

        $ echo hello >> foo.pipe
        $ seqpipe foo.pipe           # now no need to specify the procedure name

2. Try inline mode (run in shell command line, with two commands):

        $ seqpipe -e date -e pwd
        $ head .seqpipe/last/*.log   # check outputs

    Use '-E' (instead of '-e') for parallel running:

        $ seqpipe -E 'sleep 2' -E 'sleep 1'

3. Use `seqpipe` to record commands:

        $ seq 1 3 | awk '{print "echo Hello - "$1}' | seqpipe run -

    Or simply:

        $ seq 1 3 | awk '{print "echo Hello - "$1}' | seqpipe -

4. Parallel running:

        $ cat <<EOF> foo.pipe
        sleep 2
        sleep 1
        EOF

        $ seqpipe parallel foo.pipe

    Or using pipe:

        $ cat foo.pipe | seqpipe parallel -

5. More complex example for nested blocks:

        $ cat <<EOF> foo.pipe
        {{
            {
                echo A
                sleep 3
                echo B
            }
            {
                {{
                    echo C; sleep 2; echo D
                    echo E; sleep 1; echo F
                }}
                echo G
            }
        }}
        EOF

        $ seqpipe foo.pipe

6. With variable:

        $ seqpipe -e 'echo "Hello, ${NAME}!"' NAME=world

7. Variable in procedure, and mixture commands in single line:

        $ cat <<EOF> foo.pipe
        foo() {
            echo ${A}
            echo ${B}
        }
        echo 1; foo A=hello B=world; echo 2
        echo 3

        $ seqpipe foo.pipe


## Reference

    <<TODO>>: To be completed.


## Questions & Answers

1. Why give up Perl? Why C++(11)?

    Package 'thread' (both 'native' and 'fork' implementation) in Perl is suck. It is desperate for me to struggle the thread related problems in Perl. In contrast, C++(11) excites me to change brilliant ideas to reality.

2. What is the philosophy in UI design of SeqPipe?

   **Write and run pipelines just as you think.** Therefore, it is worth breaking rules to improve usability and conciseness by syntactic sugars.
