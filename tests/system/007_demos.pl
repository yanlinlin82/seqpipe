#!/usr/bin/perl
use strict;

print STDERR "$0 - ";

my $REGEX_UNIQUE_ID = '\[[0-9]{6}\.[0-9]{4}\.[0-9]+\.[^\]]+\]';
my $REGEX_TIME = '[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}';
my $REGEX_ELAPSE = '\(elapsed: [^)]+\)';
my $REGEX_OK = 'Pipeline finished successfully!';
my $REGEX_FAILED = 'Pipeline finished abnormally with exit value:';

#==========================================================#

sub test_001 # Demo(1) - Hello, world!
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
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '  (2) [shell] echo "Hello, world!"';
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
	echo \"Hello, world!\"
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.echo.cmd` ne "echo \"Hello, world!\"\n";
	die if `cat .seqpipe/last/2.echo.log` ne "Hello, world!\n";
	die if `cat .seqpipe/last/2.echo.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_001;

#==========================================================#

sub test_002 # Demo(2) - More Commands
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
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '  (2) [shell] echo "Write to stderr" >/dev/stderr';
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] ne '  (3) [shell] pwd; date';
	die if $lines[7] !~ /^  \(3\) starts at $REGEX_TIME$/;
	die if $lines[8] !~ /^  \(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[9] ne '  (4) [shell] cat foo.pipe | grep function | wc -l';
	die if $lines[10] !~ /^  \(4\) starts at $REGEX_TIME$/;
	die if $lines[11] !~ /^  \(4\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[12] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[13] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

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

sub test_003 # Demo(3) - Return Value
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
	wc -l foo.pipe; false
	echo "This command will never be run!"
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 9;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '  (2) [shell] wc -l foo.pipe; false';
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] ne '  (2) returns 1';
	die if $lines[7] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[8] !~ /^$REGEX_UNIQUE_ID $REGEX_FAILED 1! $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
	wc -l foo.pipe; false
	echo \"This command will never be run!\"
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.wc.cmd` ne "wc -l foo.pipe; false\n";
	die if `cat .seqpipe/last/2.wc.log` ne "5 foo.pipe\n";
	die if `cat .seqpipe/last/2.wc.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_003;

#==========================================================#

sub test_004 # Demo(4) - Logical Operations
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
	true && echo "This will be executed!"
	true || echo "This will not be executed!"
	false || echo "This works fine!"
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 14;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '  (2) [shell] true && echo "This will be executed!"';
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] ne '  (3) [shell] true || echo "This will not be executed!"';
	die if $lines[7] !~ /^  \(3\) starts at $REGEX_TIME$/;
	die if $lines[8] !~ /^  \(3\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[9] ne '  (4) [shell] false || echo "This works fine!"';
	die if $lines[10] !~ /^  \(4\) starts at $REGEX_TIME$/;
	die if $lines[11] !~ /^  \(4\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[12] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[13] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
	true && echo \"This will be executed!\"
	true || echo \"This will not be executed!\"
	false || echo \"This works fine!\"
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.true.cmd` ne "true && echo \"This will be executed!\"\n";
	die if `cat .seqpipe/last/2.true.log` ne "This will be executed!\n";
	die if `cat .seqpipe/last/2.true.err` ne "";
	die if `cat .seqpipe/last/3.true.cmd` ne "true || echo \"This will not be executed!\"\n";
	die if `cat .seqpipe/last/3.true.log` ne "";
	die if `cat .seqpipe/last/3.true.err` ne "";
	die if `cat .seqpipe/last/4.false.cmd` ne "false || echo \"This works fine!\"\n";
	die if `cat .seqpipe/last/4.false.log` ne "This works fine!\n";
	die if `cat .seqpipe/last/4.false.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_004;

#==========================================================#

sub test_005 # Demo(5) - Complex Bash
{
	# prepare input
	open my $fh, '>', 'foo.pipe' or die;
	print $fh '
function demo {
	for c in {1..22} X Y M; do \
		if [ "$c" == "X" -o $c == "M" ]; then \
			echo $c; \
		fi; \
	done
}
';
	close $fh;

	# run command
	my $output = `seqpipe foo.pipe demo` or die;

	# check results
	my @lines = split("\n", $output);
	die if scalar @lines != 8;
	die if $lines[0] !~ /^$REGEX_UNIQUE_ID seqpipe foo.pipe demo$/;
	die if $lines[1] ne '(1) [pipeline] demo';
	die if $lines[2] !~ /^\(1\) starts at $REGEX_TIME$/;
	die if $lines[3] ne '  (2) [shell] for c in {1..22} X Y M; do if [ "$c" == "X" -o $c == "M" ]; then echo $c; fi; done';
	die if $lines[4] !~ /^  \(2\) starts at $REGEX_TIME$/;
	die if $lines[5] !~ /^  \(2\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[6] !~ /^\(1\) ends at $REGEX_TIME $REGEX_ELAPSE$/;
	die if $lines[7] !~ /^$REGEX_UNIQUE_ID $REGEX_OK $REGEX_ELAPSE$/;

	die if `cat .seqpipe/last/log` ne $output;
	die if `cat .seqpipe/last/pipeline` ne "demo() {
	for c in {1..22} X Y M; do if [ \"\$c\" == \"X\" -o \$c == \"M\" ]; then echo \$c; fi; done
}

demo
";
	die if `cat .seqpipe/last/1.demo.call` ne "demo\n";
	die if `cat .seqpipe/last/2.for.cmd` ne 'for c in {1..22} X Y M; do if [ "$c" == "X" -o $c == "M" ]; then echo $c; fi; done' . "\n";
	die if `cat .seqpipe/last/2.for.log` ne "X\nM\n";
	die if `cat .seqpipe/last/2.for.err` ne "";

	# clean up
	unlink "foo.pipe";
}
test_005;

#==========================================================#
print "OK!\n";
exit 0;
