#!/bin/bash
if [ $# != 1 ]; then
    >&2 echo "Usage: $0 <day>"
    exit 1;
fi
daynr=$1

curl -sS --cookie .cookie "https://adventofcode.com/2021/day/$daynr" | \
    pandoc -f html -t markdown | \
    awk '
        inmain          { print }
        /role="main"/   { inmain = 1 }
    '
