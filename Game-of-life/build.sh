#!/usr/bin/env bash

echo "Building app..."

g++ main.cpp -o game-of-life \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system -fopenmp 

if [ $? -eq 0 ]; then
  echo "Build successful"
  cp ./game-of-life /c/msys64/mingw64/bin/life
  ./game-of-life
else
  echo "Build failed"
fi
