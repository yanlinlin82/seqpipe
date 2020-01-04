# SeqPipe

## Introduction

SeqPipe is a command-line based, lightweight and easy-to-use workflow for reproducible data analysis. Since it is a single Perl script with barely no any third-party dependency, SeqPipe could be run on almost all Linux or Unix-like system after download/copy it. SeqPipe tries to keep all your old shell habits, meanwhile, improves the procedure by recording all running logs automatically, supporting parallel machinism, checking file generation dependency and allowing to predefine pipelines.

## Quick Start

1. Clone repo (this 'dev-6' branch):

    ```
    git clone https://github.com/yanlinlin82/seqpipe --depth 1 -b dev-6
    ```

    Or, download the single script, put it into any directory you want:

    ```
    wget https://raw.githubusercontent.com/yanlinlin82/seqpipe/dev-6/seqpipe
    chmod +x seqpipe
    ```
    
    I suggest to add the directory to PATH environment variable:
    
    ```
    export PATH=$PATH:/path/of/seqpipe
    ```

2. Run shell command with prefix 'seqpipe':

    ```
    $ seqpipe echo hello, world
    [200104.2130.149769.yanll-laptop] seqpipe echo hello, world
    (1) [shell] echo hello, world
    (1) starts at 2020-01-04 21:30:09
    (1) ends at 2020-01-04 21:30:09 (elapsed: 0s)
    [200104.2130.149769.yanll-laptop] Pipeline finished successfully! (elapsed: 0s)
    ```
    
    List the log files:
    
    ```
    $ ls -1 .seqpipe/last/
    1.shell.cmd
    1.shell.err
    1.shell.log
    log
    sysinfo
    ```
    
    Show log file:
    
    ```
    $ cat .seqpipe/last/sysinfo 
    # seqpipe sysinfo log (version=1.0)
    
    system:
      uname   : Linux 4.19.86-gentoo x86_64
      date    : 2020-01-04 21:30:09
      pwd     : /home/yanll/foo
      cpu     : 8 core(s) (Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz)
      memory  : 15.3 GB
      user    : yanll
      uid     : 1027
      gid     : 1027
      login   : 
      hostname: yanll-laptop
      shell   : /bin/bash
    
    seqpipe:
      version : 0.6.0-beta (5b33ea6)
      path    : /home/yanll/foo/seqpipe/seqpipe
    ```
