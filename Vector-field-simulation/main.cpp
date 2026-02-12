#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

sf::Vector2f normalized(sf::Vector2f vec) {
	return (vec / sqrtf(vec.x*vec.x + vec.y*vec.y)); // Pythagoras
}

sf::Vector2f vectorFieldVector(float x, float y) {
	float dx = y; // derivative of the value on the x axis
	float dy = -sin(x) - 0.1f*y; // derivative of the value on the y axis

	return sf::Vector2f(dx, dy);
}

sf::Vector2f rungeKuttaStep(sf::Vector2f current, float dt) {
	sf::Vector2f k1 = vectorFieldVector(current.x, current.y);
	sf::Vector2f k2 = vectorFieldVector(current.x + 0.5f * dt * k1.x, current.y + 0.5f * dt * k1.y);
	sf::Vector2f k3 = vectorFieldVector(current.x + 0.5f * dt * k2.x, current.y + 0.5f * dt * k2.y);
	sf::Vector2f k4 = vectorFieldVector(current.x + dt * k3.x, current.y + dt * k3.y);

	return (dt/6.0f) * (k1 + 2.0f*k2 + 2.0f*k3 + k4);
}

sf::Vector2f pixelToWorld(sf::Vector2i pixelCoord, int screenPixelSize, float screenWorldSize) {
	sf::Vector2f coordinate;

	coordinate.x = pixelCoord.x; // 0  to  pixelSize
	coordinate.y = pixelCoord.y;

	coordinate.x -= (screenPixelSize / 2.0f); // -1/2 pixel size  to  1/2 pixel size
	coordinate.y -= (screenPixelSize / 2.0f);
	coordinate.y *= -1;

	coordinate /= (float) screenPixelSize; // -1/2  to  1/2

	coordinate *= (float) screenWorldSize; // -1/2 world size  to  1/2 world size (length of 1 world size)
	return coordinate;
}

sf::Vector2f worldToPixel(sf::Vector2f worldCoord, int screenPixelSize, float screenWorldSize) {
	sf::Vector2f coordinate = worldCoord; // -1/2 world size  to  1/2 world size (length of 1 world size)

	coordinate /= (float) screenWorldSize; // -1/2  to  1/2

	coordinate *= (float) screenPixelSize; // -1/2 pixel size  to  1/2 pixel size
	
	coordinate.y *= -1;
	coordinate.x += (screenPixelSize / 2.0f); // 0  to  pixelSize
	coordinate.y += (screenPixelSize / 2.0f);
	return coordinate;
}

float logNormalize(float v, float vMin, float vMax)
{
    v = std::max(v, vMin); // avoid log(0)

    float logMin = std::log(vMin);
    float logMax = std::log(vMax);
    float logV   = std::log(v);

    return (logV - logMin) / (logMax - logMin); // [0,1]
}

sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t)
{
    t = std::clamp(t, 0.f, 1.f);

    return sf::Color(
        static_cast<sf::Uint8>(a.r + t * (b.r - a.r)),
        static_cast<sf::Uint8>(a.g + t * (b.g - a.g)),
        static_cast<sf::Uint8>(a.b + t * (b.b - a.b))
    );
}

sf::Color blueCyanYellowRed(float t)
{
    t = std::clamp(t, 0.f, 1.f);

    const sf::Color blue   (  0,   0, 255);
    const sf::Color cyan   (  0, 255, 255);
    const sf::Color yellow (255, 255,   0);
    const sf::Color red    (255,   0,   0);

    if (t < 1.f / 3.f) {
        // Blue → Cyan
        float localT = t * 3.f;
        return lerpColor(blue, cyan, localT);
    }
    else if (t < 2.f / 3.f) {
        // Cyan to Yellow
        float localT = (t - 1.f / 3.f) * 3.f;
        return lerpColor(cyan, yellow, localT);
    }
    else {
        // Yellow direction Red
        float localT = (t - 2.f / 3.f) * 3.f;
        return lerpColor(yellow, red, localT);
    }
}

sf::Color logBlueCyanYellowRed(float value, float minValue, float maxValue)
{
    value = std::max(value, minValue);

    float logMin = std::log(minValue);
    float logMax = std::log(maxValue);
    float logV   = std::log(value);

    float t = (logV - logMin) / (logMax - logMin);
    return blueCyanYellowRed(t);
}

int main () {
	int screenPixelSize = 1300; // Pixel length of the screen
	float screenWorldSize = 18; // Length of the simulation window in world units;
	sf::RenderWindow window(sf::VideoMode(screenPixelSize, screenPixelSize), "vcetor fild");
	window.setFramerateLimit(60);

	int pixelsPerWorld = screenPixelSize / screenWorldSize; // ratio of the pixel length and the world length
	int vectorPixelDistance = 30; // This value is arbitrary

	int vectorAmount = screenPixelSize / vectorPixelDistance;

	float weight = 2.0f;
	float scale = 0.2f;

	int pathLength = 100000;
	float dt = 0.01f;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			weight += 0.01f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && weight > 0.0) {
			weight -= 0.01f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			scale += 0.005f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && scale > 0.0) {
			scale -= 0.005f;
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
			sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
			sf::Vector2f worldMousePosition = pixelToWorld(mousePosition, screenPixelSize, screenWorldSize);

			sf::Vertex eulerPathBad[pathLength];
			sf::Vertex eulerPathGood[pathLength];
			sf::Vertex rungeKuttaPath[pathLength];

			sf::Vector2f currentValueEulerBad = worldMousePosition;
			sf::Vector2f currentValueEulerGood = worldMousePosition;
			sf::Vector2f currentValueRungeKutta = worldMousePosition;

			for (int i = 0; i < pathLength; i++) {
				// eulerPathBad[i] = sf::Vertex(worldToPixel(currentValueEulerBad, screenPixelSize, screenWorldSize), sf::Color::Blue);
				eulerPathGood[i] = sf::Vertex(worldToPixel(currentValueEulerGood, screenPixelSize, screenWorldSize), sf::Color::Yellow);
				rungeKuttaPath[i] = sf::Vertex(worldToPixel(currentValueRungeKutta, screenPixelSize, screenWorldSize), sf::Color::Green);

				// currentValueEulerBad += vectorFieldVector(currentValueEulerBad.x, currentValueEulerBad.y) * dt;
				currentValueEulerGood += vectorFieldVector(currentValueEulerGood.x, currentValueEulerGood.y) * dt;
				currentValueRungeKutta += rungeKuttaStep(currentValueRungeKutta, dt);
			}

			// window.draw(eulerPathBad, pathLength, sf::LineStrip);
			window.draw(eulerPathGood, pathLength, sf::LineStrip);
			window.draw(rungeKuttaPath, pathLength, sf::LineStrip);
		}

		for (int i = 0; i < vectorAmount; i++) {
			for (int j = 0; j < vectorAmount; j++) {
				float worldX = (float)i; // 0  to  vectorAmount
				float worldY = (float)j;

				worldX /= (float)vectorAmount; // 0  to  1
				worldY /= (float)vectorAmount;

				worldX *= screenWorldSize; // 0  to  world size
				worldY *= screenWorldSize;

				worldX -= screenWorldSize/2.0f; // -1/2 world size  to  1/2 world size
				worldY -= screenWorldSize/2.0f;


				sf::Vector2f vector = vectorFieldVector(worldX, worldY); // Sample phase space direction vector from current point
				// sf::Vector2f drawingVector = normalized(vector); // Normalize for drawing purposes
				sf::Vector2f drawingVector = vector;

				float magnitude = std::sqrt(vector.x*vector.x + vector.y*vector.y);

				drawingVector /= weight;
				drawingVector += (weight-1 / weight) * normalized(drawingVector);

				sf::Vector2f worldOrigin(worldX, worldY);
				sf::Vector2f worldEnd(worldX + drawingVector.x * scale, worldY + drawingVector.y * scale);

				sf::Vertex lineSegment[] =
				{
					sf::Vertex(worldToPixel(worldOrigin, screenPixelSize, screenWorldSize), logBlueCyanYellowRed(magnitude, 0.01f, 50.0f)), // origin
					sf::Vertex(worldToPixel(worldEnd, screenPixelSize, screenWorldSize), sf::Color::Transparent)  // direction
				};

				window.draw(lineSegment, 2, sf::Lines);
			}
		}

		window.display();
	}
}
