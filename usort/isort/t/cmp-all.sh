#!/bin/bash -e

apps="s2"

for app in $apps ; do
    export app
    ./cmp.sh
done

