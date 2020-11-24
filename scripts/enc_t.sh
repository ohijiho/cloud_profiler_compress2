#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

#echo "$2.$1"
(time { "$enc" --method="$1" "${@:3}" < "test/$2" > "test/$2.$1"; }) 2>&1
du -h "test/$2.$1" "test/$2"
