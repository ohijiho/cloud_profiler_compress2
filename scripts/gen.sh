#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

"$gen" "${@:2}" > "test/$1"
