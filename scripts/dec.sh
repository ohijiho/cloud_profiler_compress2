#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

"$enc" --decode < "test/$1" > "test/$1.-"
