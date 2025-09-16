echo "Building app..."

g++ main.cpp -o black-hole-sim \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system -fopenmp

if [ $? -eq 0 ]; then
  echo "Build successful"
  ./black-hole-sim
else
  echo "Build failed"
fi
