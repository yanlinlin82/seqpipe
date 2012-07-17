#!/bin/bash

function test
{
	bwa index chrM.fa
	bwa aln -t 2 chrM.fa 1.fq.gz > 1.fq.gz.sai
	bwa aln -t 2 chrM.fa 2.fq.gz > 2.fq.gz.sai
	bwa sampe chrM.fa 1.fq.gz.sai 2.fq.gz.sai 1.fq.gz 2.fq.gz | samtools view -Sb - > out.bam
}
