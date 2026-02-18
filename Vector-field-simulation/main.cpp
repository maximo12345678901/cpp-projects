#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "../../vec.h"


Vector2 vectorFieldVector(float x, float y) {
	float dx = y; // derivative of the value on the x axis
	float dy = -sin(x) - 0.1f*y; // derivative of the value on the y axis

	return Vector2(dx, dy);
}

Vector2 rungeKuttaStep(Vector2 current, double dt) {
	Vector2 k1 = vectorFieldVector(current.x, current.y);
	Vector2 k2 = vectorFieldVector(current.x + 0.5f * dt * k1.x, current.y + 0.5f * dt * k1.y);
	Vector2 k3 = vectorFieldVector(current.x + 0.5f * dt * k2.x, current.y + 0.5f * dt * k2.y);
	Vector2 k4 = vectorFieldVector(current.x + dt * k3.x, current.y + dt * k3.y);

	return (k1 + k2*2.0 + k3*2.0 + k4) * (dt/6.0) ;
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

	double weight = 2.0;
	double scale = 0.2;

	int pathLength = 100000;
	float dt = 0.1;

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
			Vector2 worldMousePosition = pixelToWorld(mousePosition, screenPixelSize, screenWorldSize);

			sf::Vertex eulerPathBad[pathLength];
			sf::Vertex eulerPathGood[pathLength];
			sf::Vertex rungeKuttaPath[pathLength];

			Vector2 currentValueEulerBad = worldMousePosition;
			Vector2 currentValueEulerGood = worldMousePosition;
			Vector2 currentValueRungeKutta = worldMousePosition;

			for (int i = 0; i < pathLength; i++) {
				eulerPathBad[i] = sf::Vertex(worldToPixel(currentValueEulerBad, screenPixelSize, screenWorldSize), sf::Color::Blue);
				eulerPathGood[i] = sf::Vertex(worldToPixel(currentValueEulerGood, screenPixelSize, screenWorldSize), sf::Color::Yellow);	
				rungeKuttaPath[i] = sf::Vertex(worldToPixel(currentValueRungeKutta, screenPixelSize, screenWorldSize), sf::Color::Green);

				currentValueEulerBad += vectorFieldVector(currentValueEulerBad.x, currentValueEulerBad.y) * dt;
				currentValueEulerGood += vectorFieldVector(currentValueEulerGood.x, currentValueEulerGood.y) * dt / 100.0;
				currentValueRungeKutta += rungeKuttaStep(currentValueRungeKutta, dt);
			}

			window.draw(eulerPathBad, pathLength, sf::LineStrip);
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


				Vector2 vector = vectorFieldVector(worldX, worldY); // Sample phase space direction vector from current point
				// Vector2 drawingVector = normalized(vector); // Normalize for drawing purposes
				Vector2 drawingVector = vector;

				float magnitude = std::sqrt(vector.x*vector.x + vector.y*vector.y);

				drawingVector /= weight;
				drawingVector += drawingVector.Normalized() * (weight-1 / weight);

				Vector2 worldOrigin(worldX, worldY);
				Vector2 worldEnd(worldX + drawingVector.x * scale, worldY + drawingVector.y * scale);

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
