# Before we start #

> It is very easy to install [SeqPipe](SeqPipe.md): You can download the source code (including the test data) on your computer (I suppose you are using Linux), and then set PATH envionment variable (you may skip this step if your computer (or server) has already installed seqpipe:
```
$ svn co http://svn.cbi.pku.edu.cn/svn/seqpipe/trunk/ seqpipe/
$ export PATH=$(pwd)/seqpipe/:$PATH
```


# Show usage #

```
$ seqpipe

SeqPipe: a SEQuencing data analsysis PIPEline framework
Version: 0.4.3 ($Rev: 214 $)
Author : Linlin Yan (yanll<at>mail.cbi.pku.edu.cn)
Copyright: 2012, Centre for Bioinformatics, Peking University, China

Usage: seqpipe [options] <procedure> [NAME=VALUE ...]

Options:
   -h / -H     Show this or procedure usage. -H for more details.
   -m <file>   Load procedure module file, this option can be used many times.
   -l / -L     List current available proc_list. -L for all procedures (include internal ones).
   -T          Show the raw procedure declaration.
   -k          Keep intermediate files.
   -t <int>    Max thread number, 0 for unlimited. default: 2
   -e <cmd>    Inline mode, execute a bash command directly.
   -s <shell>  Send commands to another shell (such as "qsub_sync"), default: /bin/bash
```


# Run a test #

> Then goto the test subdirectory and run the demo:
```
$ cd seqpipe/tests/
$ seqpipe DNAseq_analysis NAME=r REF=MT.fa GATK_VCF_DBSNP=dbsnp_135.b37.MT.vcf
```

# Check log files #
```
$ ls .seqpipe/
121117.20233  history.log
```

```
$ ls .seqpipe/121117.20233/
11.eval.cmd         22.test_false.cmd        35.pindel2vcf.err  45.eval.cmd
11.eval.result      22.test_false.err        35.pindel2vcf.log  45.eval.result
12.test.cmd         22.test_false.log        36.pindel2vcf.cmd  46.java.cmd
12.test.err         23.test_SILENT.cmd       36.pindel2vcf.err  46.java.err
12.test.log         23.test_SILENT.err       36.pindel2vcf.log  46.java.log
13.bwa_index.cmd    23.test_SILENT.log       37.pindel2vcf.cmd  48.java.cmd
13.bwa_index.err    24.eval.cmd              37.pindel2vcf.err  48.java.err
13.bwa_index.log    24.eval.result           37.pindel2vcf.log  48.java.log
14.bwa_aln.cmd      25.java.cmd              38.pindel2vcf.cmd  49.eval.cmd
14.bwa_aln.err      25.java.err              38.pindel2vcf.err  49.eval.result
14.bwa_aln.log      25.java.log              38.pindel2vcf.log  50.java.cmd
15.bwa_aln.cmd      27.samtools_mpileup.cmd  39.java.cmd        50.java.err
15.bwa_aln.err      27.samtools_mpileup.err  39.java.err        50.java.log
15.bwa_aln.log      27.samtools_mpileup.log  39.java.log        52.eval.cmd
16.bwa_sampe.cmd    30.eval.cmd              40.pindel2vcf.cmd  52.eval.result
16.bwa_sampe.err    30.eval.result           40.pindel2vcf.err  53.java.cmd
16.bwa_sampe.log    32.eval.cmd              40.pindel2vcf.log  53.java.err
18.test_SILENT.cmd  32.eval.result           41.java.cmd        53.java.log
18.test_SILENT.err  33.pindel.cmd            41.java.err        55.java.cmd
18.test_SILENT.log  33.pindel.err            41.java.log        55.java.err
19.eval.cmd         33.pindel.log            42.eval.cmd        55.java.log
19.eval.result      34.pindel2vcf.cmd        42.eval.result     8.check.cmd
20.java.cmd         34.pindel2vcf.err        43.java.cmd        log
20.java.err         34.pindel2vcf.log        43.java.err        sysinfo
20.java.log         35.pindel2vcf.cmd        43.java.log
```