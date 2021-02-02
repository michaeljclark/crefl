#!/bin/bash
find test -name '*.h' | sed 's#\(.*\)#./scripts/run-test.sh \1#'
