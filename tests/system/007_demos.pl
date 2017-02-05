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

sub test_002
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
	echo "Write to stderr" >/dev/stderr
	pwd; date
	cat foo.pipe | grep function \
		| wc -l
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 14;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] !~ /^\(1\) \[pipeline\] demo$/;
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^  \(2\) \[shell\] echo "Write to stderr" >\/dev\/stderr$/;
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] !~ /^  \(3\) \[shell\] pwd; date$/;
	die if $lines[7] !~ /^  \(3\) starts at $REGEX_TIME$/;
	die if $lines[8] !~ /^  \(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[9] !~ /^  \(4\) \[shell\] cat foo.pipe \| grep function \| wc -l$/;
	die if $lines[10] !~ /^  \(4\) starts at $REGEX_TIME$/;
	die if $lines[11] !~ /^  \(4\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[12] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[13] !~ /^$REGEX_UNIQUE_ID Pipeline finished successfully! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
	echo \"Write to stderr\" >/dev/stderr
	pwd; date
	cat foo.pipe | grep function | wc -l
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo \"Write to stderr\" >/dev/stderr\n";
	die if `cat .seqpipe/last/2.echo.log` ne "";
	die if `cat .seqpipe/last/2.echo.err` ne "Write to stderr\n";
	die if `cat .seqpipe/last/3.pwd.cmd` ne "pwd; date\n";
	die if `cat .seqpipe/last/3.pwd.log` eq ""; # no test here, since the result is changing
	die if `cat .seqpipe/last/3.pwd.err` ne "";
	die if `cat .seqpipe/last/4.cat.cmd` ne "cat foo.pipe | grep function | wc -l\n";
	die if `cat .seqpipe/last/4.cat.log` ne "2\n";
	die if `cat .seqpipe/last/4.cat.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_002;

#==========================================================#
print "OK!\n";
exit 0;
