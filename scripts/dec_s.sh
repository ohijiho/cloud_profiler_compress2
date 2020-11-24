#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

"$enc" --decode < "test/$1" > "test/$1.-"
if ! diff "test/$1.-" "test/$2" > /dev/null; then
  cksum "test/$1.-" "test/$2"
fi
