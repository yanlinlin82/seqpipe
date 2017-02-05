#!/usr/bin/perl
use strict;

print STDERR "$0 - ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';

#==========================================================#

sub test_001
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
	echo "Hello, world!"
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] !~ /^\(1\) \[pipeline\] demo$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^  \(2\) \[shell\] echo "Hello, world!"$/;
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {\n\techo \"Hello, world!\"\n}\n\ndemo\n";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/2.echo.log` ne "Hello, world!\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_001;

#==========================================================#
print "OK!\n";
exit 0;
