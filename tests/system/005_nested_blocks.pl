#!/usr/bin/perl
use strict;

print STDERR "$0 - ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';

#==========================================================#

sub parse_log_id_and_type
{
	my $log_text = shift;
	my ($id, $type) = (0, 0);
	if ($log_text =~ /^\(([0-9]+)\) (\[|starts|ends)/) {
		$id = $1;
		if ($2 eq 'starts') {
			$type = 1;
		} elsif ($2 eq 'ends') {
			$type = 2;
		}
	}
	return ($id, $type);
}

sub by_log_id_and_type
{
	my ($a_id, $a_type) = parse_log_id_and_type($a);
	my ($b_id, $b_type) = parse_log_id_and_type($b);
	my $n = $a_id <=> $b_id;
	if ($n == 0) {
		$n = $a_type <=> $b_type;
	}
	return $n;
}

#==========================================================#

sub test_001
{
my $code = '{{
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
';
	# prepare input
	open my $fh, ">foo.pipe" or die;
	print $fh $code;
	close $fh;

	# run command
	my $output = `seqpipe run foo.pipe` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 32;
	@lines[1..30] = sort by_log_id_and_type @lines[1..30];
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run foo.pipe$/;
	die if $lines[31] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne '{{
	{
		echo A
		sleep 3
		echo B
	}
	{
		{{
			{
				echo C
				sleep 2
				echo D
			}
			{
				echo E
				sleep 1
				echo F
			}
		}}
		echo G
	}
}}
';

	# clean up
	unlink "foo.pipe";
}
test_001;

#==========================================================#
print "OK!\n";
exit 0;
