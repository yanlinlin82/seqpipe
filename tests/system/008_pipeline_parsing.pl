#!/usr/bin/perl
use strict;

print STDERR "$0 - ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';
my $REGEX_OK = 'Pipeline finished successfully!';
my $REGEX_FAILED = 'Pipeline finished abnormally with exit value:';

#==========================================================#

sub test_001 # empty function
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 5;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";

	# clean up
	unlink "foo.pipe";
}
test_001;

#==========================================================#

sub test_002 # empty function (with empty line)
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {

}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 5;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";

	# clean up
	unlink "foo.pipe";
}
test_001;

#==========================================================#
print "OK!\n";
exit 0;
