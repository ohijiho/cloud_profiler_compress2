#!/bin/bash

cd "$(dirname "$0")/.."

gen=cmake-build-release/generator
enc=cmake-build-release/encoder

x_list=(ts n u e)
#x_list=(u)
#f_list=(i d b f df bf)
f_list=("${@:2}")

mkdir -p test
rm -f test/*

for x in "${x_list[@]}"; do
  scripts/gen.sh "$x" --method="$x" --mean=200 --stddev=50 --lowest=1 --highest=100 --blocks=4096
#  scripts/gen.sh "$x" --method="$x" --mean=200 --stddev=50 --lowest=1000 --highest=1000 --blocks=4096
  for f in "${f_list[@]}"; do
    scripts/enc_t.sh "$f" "$x" --block-size=65536 --deflate-level="$1"
    scripts/dec_s.sh "$x.$f" "$x"
    rm "test/$x.$f" "test/$x.$f.-"
  done
  rm "test/$x"
done
