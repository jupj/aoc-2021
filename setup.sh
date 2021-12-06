#!/bin/bash

daydir=$(date +"day%d")
prevdir=$(date -d yesterday +"day%d")

if [ -d "$daydir" ]; then
    echo "Error: $daydir exists already"
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

go run get-input.go > "$daydir/input.txt"
