#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

"$enc" --method="$1""${@:3}" < "test/$2" > "test/$2.$1"
