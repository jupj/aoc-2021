#!/bin/bash
if [ $# != 1 ]; then
    echo "Usage: $0 <day>"
    exit 1;
fi
daynr=$1

curl -s --cookie .cookie "https://adventofcode.com/2021/day/$daynr" | \
    pandoc -f html -t markdown | \
    awk '
        inmain          { print }
        /role="main"/   { inmain = 1 }
    '
