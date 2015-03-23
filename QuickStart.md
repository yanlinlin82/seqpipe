# Installation #

SeqPipe is written in [Perl](http://www.perl.org/), so please make ensure perl has been installed before you start.

You can download SeqPipe from the [Downloads](https://code.google.com/p/seqpipe/downloads/list) page and then extract files from the compressed tarball:
```
wget http://seqpipe.googlecode.com/files/seqpipe-v0.4.10-r262.tar.gz
tar xvf seqpipe-v0.4.10-r262.tar.gz
```

Or you can directly checkout the source code from subversion (see the [Source](https://code.google.com/p/seqpipe/source/list) page).
```
svn checkout http://seqpipe.googlecode.com/svn/trunk seqpipe
```

Then, add the seqpipe directory to you PATH environment variable, for example:
```
export PATH=$(pwd)/seqpipe:$PATH
```

Now, you can just type "seqpipe" to run it.

To make it easy next time, you can edit your bash profile (such as ~/.bashrc) to add the PATH exporting line.

For more information, please go to the [Installation](Installation.md) page.


# Show usage #

When no any options is given, SeqPipe will prompt the usage messages as following:

```
$ seqpipe

SeqPipe: a SEQuencing data analsysis PIPEline framework
Version: 0.4.10 ($Rev: 262 $)
Author : Linlin Yan (yanll<at>mail.cbi.pku.edu.cn)
Copyright: 2012-2013, Centre for Bioinformatics, Peking University, China
Websites: http://seqpipe.googlecode.com
          http://www.cbi.pku.edu.cn

Usage: seqpipe [options] <procedure> [NAME=VALUE ...]

Options:
   -h / -H         Show help messages.
   -l [<pattern>]  List current available procedures.
   -m <file>       Load procedure module file, this option can be used many times.
   -t <int>        Max thread number in parallel. default: 1
   -e <cmd>        Inline mode, execute a bash command directly.
   -s <shell>      Send commands to another shell (such as "qsub_sync"), default: /bin/bash
   -k              Keep intermediate files.
   -R              Show the raw procedure declaration.
   -T              Test mode, show commands rather than execute them.
```


# Write first pipeline #

Create new text file (named as "hello.pipe") as following:
```
function hello
{
   echo "Hello SeqPipe!"
}
```

Then run the command:
```
seqpipe -m hello.pipe hello
```

If you see message "Pipeline finished successfully!", Congratulations! You have successfully run your first pipeline.

Now you can find a hidden directory .seqpipe in current working directory, in which log files are stored automatically:

```
$ ls .seqpipe
```