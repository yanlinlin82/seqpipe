#!/usr/bin/perl
use strict;

print STDERR "$0 - ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';

#==========================================================#

sub test_001
{
	# run command
	my $output = `seqpipe -e "echo hello" -e "echo world"` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe -e 'echo hello' -e 'echo world'$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo hello$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^\(2\) \[shell\] echo world$/;
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "{\n\techo hello\n\techo world\n}\n";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo hello\n";
	die if `cat .seqpipe/last/1.echo.log` ne "hello\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo world\n";
	die if `cat .seqpipe/last/2.echo.log` ne "world\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";
}
test_001;

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

sub test_002
{
	# run command
	my $output = `seqpipe -E "echo hello" -E "echo world"` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	@lines[1..6] = sort by_log_id_and_type @lines[1..6];
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe -E 'echo hello' -E 'echo world'$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo hello$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^\(2\) \[shell\] echo world$/;
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "{{\n\techo hello\n\techo world\n}}\n";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo hello\n";
	die if `cat .seqpipe/last/1.echo.log` ne "hello\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo world\n";
	die if `cat .seqpipe/last/2.echo.log` ne "world\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";
}
test_002;

#==========================================================#
print "OK!\n";
exit 0;
