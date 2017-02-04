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
	# run command
	my $output = `seqpipe -e 'echo "Hello, \${NAME}!"' NAME=world` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 5;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe -e \'echo \"Hello, \$\{NAME\}\!\"\' NAME=world$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo "Hello, world!"$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "echo \"Hello, \${NAME}!\"\n";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/1.echo.log` ne "Hello, world!\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
}
test_001;

#==========================================================#

sub test_002
{
my $code = 'foo() {
	echo ${A}
	echo ${B}
}
echo 1; foo A=hello B=world; echo 2
echo 3
';
	# prepare input
	open my $fh, ">foo.pipe" or die;
	print $fh $code;
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 20;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo 1$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^\(2\) \[pipeline\] foo A=hello B=world$/;
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^  \(3\) \[shell\] echo hello$/;
	die if $lines[7] !~ /^  \(3\) starts at $REGEX_TIME$/;
	die if $lines[8] !~ /^  \(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[9] !~ /^  \(4\) \[shell\] echo world$/;
	die if $lines[10] !~ /^  \(4\) starts at $REGEX_TIME$/;
	die if $lines[11] !~ /^  \(4\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[12] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[13] !~ /^\(5\) \[shell\] echo 2$/;
	die if $lines[14] !~ /^\(5\) starts at $REGEX_TIME$/;
	die if $lines[15] !~ /^\(5\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[16] !~ /^\(6\) \[shell\] echo 3$/;
	die if $lines[17] !~ /^\(6\) starts at $REGEX_TIME$/;
	die if $lines[18] !~ /^\(6\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[19] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne 'foo() {
	echo ${A}
	echo ${B}
}

{
	echo 1
	foo A=hello B=world
	echo 2
	echo 3
}
';
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo 1\n";
	die if `cat .seqpipe/last/1.echo.log` ne "1\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.foo.call` ne "foo\n";
	die if `cat .seqpipe/last/3.echo.cmd` ne "echo hello\n";
	die if `cat .seqpipe/last/3.echo.log` ne "hello\n";
	die if `cat .seqpipe/last/3.echo.err` ne "";
	die if `cat .seqpipe/last/4.echo.cmd` ne "echo world\n";
	die if `cat .seqpipe/last/4.echo.log` ne "world\n";
	die if `cat .seqpipe/last/4.echo.err` ne "";
	die if `cat .seqpipe/last/5.echo.cmd` ne "echo 2\n";
	die if `cat .seqpipe/last/5.echo.log` ne "2\n";
	die if `cat .seqpipe/last/5.echo.err` ne "";
	die if `cat .seqpipe/last/6.echo.cmd` ne "echo 3\n";
	die if `cat .seqpipe/last/6.echo.log` ne "3\n";
	die if `cat .seqpipe/last/6.echo.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_002;

#==========================================================#
print "OK!\n";
exit 0;
