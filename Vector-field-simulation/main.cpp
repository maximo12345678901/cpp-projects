#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

sf::Vector2f normalized(sf::Vector2f vec) {
	return (vec / sqrtf(vec.x*vec.x + vec.y*vec.y)); // Pythagoras
}

sf::Vector2f vectorFieldVector(float a, float b) {
	float da = b; // derivative of the value on the x axis
	float db = a; // derivative of the value on the y axis

	return sf::Vector2f(da, db);
}

sf::Vector2f pixelToUnit(sf::Vector2i pixelCoord, int screenPixelSize, float screenUnitSize) {
	sf::Vector2f coordinate;

	coordinate.x = pixelCoord.x; // 0  to  pixelSize
	coordinate.y = pixelCoord.y;

	coordinate.x -= (screenPixelSize / 2.0f); // -1/2 pixel size  to  1/2 pixel size
	coordinate.y -= (screenPixelSize / 2.0f);
	coordinate.y *= -1;

	coordinate /= (float) screenPixelSize; // -1/2  to  1/2

	coordinate *= (float) screenUnitSize; // -1/2 unit size  to  1/2 unit size (length of 1 unit size)
	return coordinate;
}

sf::Vector2f unitToPixel(sf::Vector2f unitCoord, int screenPixelSize, float screenUnitSize) {
	sf::Vector2f coordinate = unitCoord; // -1/2 unit size  to  1/2 unit size (length of 1 unit size)

	coordinate /= (float) screenUnitSize; // -1/2  to  1/2

	coordinate *= (float) screenPixelSize; // -1/2 pixel size  to  1/2 pixel size
	
	coordinate.y *= -1;
	coordinate.x += (screenPixelSize / 2.0f); // 0  to  pixelSize
	coordinate.y += (screenPixelSize / 2.0f);
	return coordinate;
}

int main () {
	int screenPixelSize = 1000; // Pixel length of the screen
	float screenUnitSize = 20; // Length of the simulation window in simulation units;
	sf::RenderWindow window(sf::VideoMode(screenPixelSize, screenPixelSize), "Vector field");

	int pixelsPerUnit = screenPixelSize / screenUnitSize; // ratio of the pixel length and the unit length
	int vectorPixelDistance = 20; // This value is arbitrary

	int vectorAmount = screenPixelSize / vectorPixelDistance;

	while (window.isOpen()) {
		window.clear();

		for (int i = 0; i < vectorAmount; i++) {
			for (int j = 0; j < vectorAmount; j++) {
				float unitX = (float)i; // 0  to  vectorAmount
				float unitY = (float)j;

				unitX /= (float)vectorAmount; // 0  to  1
				unitY /= (float)vectorAmount;

				unitX *= screenUnitSize; // 0  to  unit size
				unitY *= screenUnitSize;

				unitX -= screenUnitSize/2.0f; // -1/2 unit size  to  1/2 unit size
				unitY -= screenUnitSize/2.0f;


				sf::Vector2f vector = vectorFieldVector(unitX, unitY); // Sample phase space direction vector from current point
				// sf::Vector2f drawingVector = normalized(vector); // Normalize for drawing purposes

				float scale = 0.2f;
				sf::Vector2f unitOrigin(unitX, unitY);
				sf::Vector2f unitEnd   (unitX + vector.x * scale, unitY + vector.y * scale);

				sf::Vertex lineSegment[] =
				{
					sf::Vertex(unitToPixel(unitOrigin, screenPixelSize, screenUnitSize), sf::Color::White), // origin
					sf::Vertex(unitToPixel(unitEnd, screenPixelSize, screenUnitSize), sf::Color::Black)  // direction
				};

				window.draw(lineSegment, 2, sf::Lines);
			}
		}

		window.display();
	}
}
