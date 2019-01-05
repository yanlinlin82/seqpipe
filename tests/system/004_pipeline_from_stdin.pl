#!/usr/bin/perl
use strict;

print STDERR "$0 ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';
my $REGEX_OK = 'Pipeline finished successfully!';
my $REGEX_FAILED = 'Pipeline finished abnormally with exit value:';

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
	# run command
	my $output = `seq 1 3 | awk '{print "echo Hello - "\$1}' | seqpipe run -` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 11;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run -$/;
	die if $lines[1] ne '(1) [shell] echo Hello - 1';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] ne '(2) [shell] echo Hello - 2';
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] ne '(3) [shell] echo Hello - 3';
	die if $lines[8] !~ /^\(3\) starts at $REGEX_TIME$/;
	die if $lines[9] !~ /^\(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[10] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "echo Hello - 1
echo Hello - 2
echo Hello - 3
";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo Hello - 1\n";
	die if `cat .seqpipe/last/1.echo.log` ne "Hello - 1\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo Hello - 2\n";
	die if `cat .seqpipe/last/2.echo.log` ne "Hello - 2\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";
	die if `cat .seqpipe/last/3.echo.cmd` ne "echo Hello - 3\n";
	die if `cat .seqpipe/last/3.echo.log` ne "Hello - 3\n";
	die if `cat .seqpipe/last/3.echo.err` ne "";
}
test_001; print '.';

#==========================================================#

sub test_002
{
	# run command
	my $output = `seq 1 3 | awk '{print "echo Hello - "\$1}' | seqpipe -` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 11;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe -$/;
	die if $lines[1] ne '(1) [shell] echo Hello - 1';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] ne '(2) [shell] echo Hello - 2';
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] ne '(3) [shell] echo Hello - 3';
	die if $lines[8] !~ /^\(3\) starts at $REGEX_TIME$/;
	die if $lines[9] !~ /^\(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[10] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "echo Hello - 1
echo Hello - 2
echo Hello - 3
";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo Hello - 1\n";
	die if `cat .seqpipe/last/1.echo.log` ne "Hello - 1\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo Hello - 2\n";
	die if `cat .seqpipe/last/2.echo.log` ne "Hello - 2\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";
	die if `cat .seqpipe/last/3.echo.cmd` ne "echo Hello - 3\n";
	die if `cat .seqpipe/last/3.echo.log` ne "Hello - 3\n";
	die if `cat .seqpipe/last/3.echo.err` ne "";
}
test_002; print '.';

#==========================================================#

sub test_003
{
	# run command
	my $output = `seq 2 -1 1 | awk '{print "sleep "\$1}' | seqpipe parallel -` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	@lines[1..4] = sort by_log_id_and_type @lines[1..4];
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe parallel -$/;
	die if $lines[1] ne '(1) [shell] sleep 2';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '(2) [shell] sleep 1';
	die if $lines[4] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "{{
	sleep 2
	sleep 1
}}
";
	die if `cat .seqpipe/last/1.sleep.cmd` ne "sleep 2\n";
	die if `cat .seqpipe/last/1.sleep.log` ne "";
	die if `cat .seqpipe/last/1.sleep.err` ne "";
	die if `cat .seqpipe/last/2.sleep.cmd` ne "sleep 1\n";
	die if `cat .seqpipe/last/2.sleep.log` ne "";
	die if `cat .seqpipe/last/2.sleep.err` ne "";
}
test_003; print '.';

#==========================================================#
print " OK!\n";
exit 0;
