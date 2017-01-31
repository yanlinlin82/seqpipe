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
	my $output = `seqpipe run echo 'Hello, world!'` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 5;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run echo 'Hello, world!'$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo 'Hello, world!'$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;
}
test_001;

#==========================================================#

sub test_002
{
	# prepare input
	open my $fh, ">hello.pipe" or die;
	print $fh 'echo "Hello, world!"
sleep 1
echo "Goodbye!"
';
	close $fh;

	# run command
	my $output = `seqpipe run hello.pipe`;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 11;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run hello.pipe$/;
	die if $lines[1] !~ /^\(1\) \[shell\] echo 'Hello, world!'$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^\(2\) \[shell\] sleep 1$/;
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^\(3\) \[shell\] echo 'Goodbye!'$/;
	die if $lines[8] !~ /^\(3\) starts at $REGEX_TIME$/;
	die if $lines[9] !~ /^\(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[10] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	# clean up
	unlink "hello.pipe";
}
test_002;

#==========================================================#
print "OK!\n";
exit 0;
