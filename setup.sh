#!/bin/bash

daydir=$(date +"day%d")
prevdir=$(date -d yesterday +"day%d")

if [ -d "$daydir" ]; then
    >&2 echo "Error: $daydir exists already"
    exit 1
fi

mkdir "$daydir"

if [ -d "$prevdir" ]; then
    # Copy files
    cp "$prevdir"/* "$daydir"

    # Rename filename to current day
    mv "$daydir/$prevdir.c" "$daydir/$daydir.c"
    sed -i "s/$prevdir/$daydir/g" "$daydir/Makefile"
fi

daynr=$(date +"%d" | sed "s/^0//") # Strip prefix 0 from day number
curl -sS --cookie .cookie "https://adventofcode.com/2021/day/$daynr/input" > "$daydir/input.txt"

./getdesc.sh "$daynr" > "$daydir/description.md"
