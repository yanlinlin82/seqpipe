#!/usr/bin/perl
use strict;

print STDERR "$0 ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';
my $REGEX_OK = 'Pipeline finished successfully!';
my $REGEX_FAILED = 'Pipeline finished abnormally with exit value:';

#==========================================================#

sub test_001
{
	# run command
	my $output = `seqpipe run echo 'Hello, world!'` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 5;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run echo 'Hello, world!'$/;
	die if $lines[1] ne '(1) [shell] echo "Hello, world!"';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/1.echo.log` ne "Hello, world!\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
}
test_001; print '.';

#==========================================================#

sub test_002
{
	# prepare input
	open my $fh, '>', 'hello.pipe' or die;
	print $fh 'echo "Hello, world!"
echo "Goodbye!"
';
	close $fh;

	# run command
	my $output = `seqpipe run hello.pipe`;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe run hello.pipe$/;
	die if $lines[1] ne '(1) [shell] echo "Hello, world!"';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[4] ne '(2) [shell] echo "Goodbye!"';
	die if $lines[5] !~ /^\(2\) starts at $REGEX_TIME$/;
	die if $lines[6] !~ /^\(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "echo \"Hello, world!\"
echo \"Goodbye!\"
";
	die if `cat .seqpipe/last/1.echo.cmd` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/1.echo.log` ne "Hello, world!\n";
	die if `cat .seqpipe/last/1.echo.err` ne "";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo \"Goodbye!\"\n";
	die if `cat .seqpipe/last/2.echo.log` ne "Goodbye!\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";

	# clean up
	unlink "hello.pipe";
}
test_002; print '.';

#==========================================================#
print " OK!\n";
exit 0;
