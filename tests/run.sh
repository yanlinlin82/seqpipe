#!/bin/bash

# Set PATH to make current 'seqpipe' be default.
export PATH=.:$PATH

# Run every perl script in tests/
for TEST in tests/*.pl; do
	${TEST} || exit 1
done

echo 1>&2 "All tests passed."
exit 0
