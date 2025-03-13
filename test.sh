#!/usr/bin/env bash

./build.sh && ./run.sh "$1" test.ppm && python3 diff.py test.ppm "$2"