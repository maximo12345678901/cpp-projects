#!/usr/bin/env bash

filename="$1"
binaryname="$2"
echo "Building app..."

g++ $filename -o $binaryname \
  -I/opt/sfml2/include \
  -L/opt/sfml2/lib \
  -lsfml-graphics -lsfml-window -lsfml-system -fopenmp

if [ $? -eq 0 ]; then
  echo "Build successful"
  LD_LIBRARY_PATH=/opt/sfml2/lib:$LD_LIBRARY_PATH ./$binaryname
else
  echo "Build failed"
fi