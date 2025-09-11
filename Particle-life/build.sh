echo "Building app..."

g++ main.cpp -o particle-life \
  -IC:/SFML-install/include \
  -LC:/SFML-install/lib \
  -lsfml-graphics -lsfml-window -lsfml-system

if [ $? -eq 0 ]; then
  echo "Build successful"
  ./particle-life
else
  echo "Build failed"
fi
