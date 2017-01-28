# SeqPipe

A framework for SEQuencing data analysis PIPElines.

## Installation

    $ git clone https://github.com/yanlinlin82/seqpipe -b cpp-v0.5 --single-branch
    $ make -C seqpipe/
    $ sudo cp -av seqpipe/seqpipe /usr/bin/   # or other directory set in PATH
    $ rm -rf seqpipe/                         # (optionally) clean up

## Quick Start

1. The easiest way to use `seqpipe` is run shell command with prefix `seqpipe run`:

        $ seqpipe run echo 'Hello, world!'

    Log files can be checked in `.seqpipe` directory:

        $ ls -lR .seqpipe/

2. Pipeline can also be written into file as:

        $ cat <<EOF> hello.pipe
        echo "Hello, world!"
        sleep 1
        echo "Goodbye!"
        EOF

    Then run:

        $ seqpipe run hello.pipe

    Check log files:

        $ ls -lR .seqpipe/last/

## See Also

For more information, see [UserGuide.md](UserGuide.md).
