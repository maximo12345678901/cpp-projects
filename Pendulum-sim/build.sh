#!/usr/bin/env bash

echo "Building app..."

g++ main.cpp -o pendulum-simulation \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system -fopenmp

if [ $? -eq 0 ]; then
  echo "Build successful"
  ./pendulum-simulation
else
  echo "Build failed"
fi
