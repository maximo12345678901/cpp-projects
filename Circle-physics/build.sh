#!/usr/bin/env bash

echo "Building app..."

g++ main.cpp -o circle-simulation \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system

if [ $? -eq 0 ]; then
  echo "Build successful"
  ./circle-simulation
else
  echo "Build failed"
fi
