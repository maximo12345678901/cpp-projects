#!/usr/bin/env bash

echo "Building app..."

g++ main.cpp -o raytracing-engine \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system

if [ $? -eq 0 ]; then
  echo "Build successful"
  ./raytracing-engine
else
  echo "Build failed"
fi
