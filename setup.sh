#!/bin/bash

basedir=$(dirname $0)
sessionfile="$basedir/.cookie"
daydir="$basedir/$(date +"day%d")"
prevdir="$basedir/$(date -d yesterday +"day%d")"

daynr=$(date +"%d" | sed "s/^0//") # Strip prefix 0 from day number

if ! [ -d "$daydir" ]; then
    # Initialize only once
    mkdir "$daydir"

    if [ -d "$prevdir" ]; then
        # Copy files
        cp "$prevdir"/* "$daydir"

        # Rename filename to current day
        mv "$daydir/$prevdir.c" "$daydir/$daydir.c"
        sed -i "s/$prevdir/$daydir/g" "$daydir/Makefile"

        # Clear test_input.txt
        rm "$daydir/test_input.txt"
        touch "$daydir/test_input.txt"
    fi

    # Write test_expected.txt
    echo "Part one: _" > "$daydir/test_expected.txt"
    echo "Part two: _" >> "$daydir/test_expected.txt"

    # Write input.txt
    curl -sS --cookie "$sessionfile" "https://adventofcode.com/2021/day/$daynr/input" > "$daydir/input.txt"
fi

# Update description.md
curl -sS --cookie "$sessionfile" "https://adventofcode.com/2021/day/$daynr" | \
    pandoc -f html -t markdown | \
    awk '
        inmain          { print }
        /role="main"/   { inmain = 1 }
    ' > "$daydir/description.md"
