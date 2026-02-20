#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "../../vec.h"
#include <iostream>


void drawAxes(sf::RenderWindow& window, int screenPixelSize, Vector3 cameraPos, Vector3 cameraCenter, Vector2 cameraRot, double length) {
    sf::Vertex xAxis[] =
	{
		sf::Vertex(worldToPixel3D(cameraCenter - Vector3(length, 0.0, 0.0), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Red),
		sf::Vertex(worldToPixel3D(cameraCenter + Vector3(length, 0.0, 0.0), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Red)
	};
	sf::Vertex yAxis[] =
	{
		sf::Vertex(worldToPixel3D(cameraCenter - Vector3(0.0, length, 0.0), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Green),
		sf::Vertex(worldToPixel3D(cameraCenter + Vector3(0.0, length, 0.0), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Green)
	};
	sf::Vertex zAxis[] =
	{
		sf::Vertex(worldToPixel3D(cameraCenter - Vector3(0.0, 0.0, length), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Blue),
		sf::Vertex(worldToPixel3D(cameraCenter + Vector3(0.0, 0.0, length), cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Blue)
	};

	window.draw(xAxis, 2, sf::Lines);
	window.draw(yAxis, 2, sf::Lines);
	window.draw(zAxis, 2, sf::Lines);
}




Vector3 vectorFieldVector(double x, double y, double z) {
	double dx = 10.0*(y-x);
	double dy = x*(28.0-z)-y;
	double dz = x*y - 8.0*z/3.0;

	return Vector3(dx, dy, dz);
}

Vector3 rungeKuttaStep(Vector3 current, double dt) {
	Vector3 k1 = vectorFieldVector(current.x, current.y, current.z);
	Vector3 k2 = vectorFieldVector(current.x + 0.5f * dt * k1.x, current.y + 0.5f * dt * k1.y, current.z + 0.5f * dt * k1.z);
	Vector3 k3 = vectorFieldVector(current.x + 0.5f * dt * k2.x, current.y + 0.5f * dt * k2.y, current.z + 0.5f * dt * k2.z);
	Vector3 k4 = vectorFieldVector(current.x + dt * k3.x, current.y + dt * k3.y, current.z + dt * k3.z);

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
	int vectorPixelDistance = 60; // This value is arbitrary

	int vectorAmount = screenPixelSize / vectorPixelDistance;

	// Arrow variables
	double weight = 2.0;
	double scale = 0.2;

	// Path variables
	int pathLength = 100000;
	float dt = 0.001;
	Vector3 trajectoryStart(1.0, 1.0, 1.0);

	// Camera variables
	Vector3 cameraPos(0.0, 0.0, -20.0);
	Vector3 cameraCenter(0.0, 0.0, 20.0);
	double cameraDistanceFromCenter = 40.0;
	Vector2 cameraRot;
	cameraRot.x = -(M_PI/1.5);
	cameraRot.y = 3.7;
	double cameraPanSpeed = 0.05;
	double cameraMoveSpeed = 0.5;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			cameraRot.y -= cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			cameraRot.y += cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			cameraRot.x -= cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			cameraRot.x += cameraPanSpeed;
		}

		if (cameraRot.x < -M_PI) {cameraRot.x = -M_PI+0.01;}
		if (cameraRot.x > 0.0) {cameraRot.x = -0.01;}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			cameraCenter.x -= cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			cameraCenter.x += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			cameraCenter.z += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			cameraCenter.z -= cameraMoveSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			cameraDistanceFromCenter += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && cameraDistanceFromCenter > 5.0) {
			cameraDistanceFromCenter -= cameraMoveSpeed;
		}
		cameraPos = cameraCenter + Vector3::getForward(cameraRot) * -cameraDistanceFromCenter;

		// Trajectory controls
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) {
			trajectoryStart.z += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
			trajectoryStart.z -= cameraMoveSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
			trajectoryStart.x += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
			trajectoryStart.x -= cameraMoveSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {
			trajectoryStart.y += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) {
			trajectoryStart.y -= cameraMoveSpeed;
		}

		// sf::Vertex eulerPathBad[pathLength];
		// sf::Vertex eulerPathGood[pathLength];
		sf::Vertex rungeKuttaPath[pathLength];

		// Vector3 currentValueEulerBad = trajectoryStart;
		// Vector3 currentValueEulerGood = trajectoryStart;
		Vector3 currentValueRungeKutta = trajectoryStart;

		for (int i = 0; i < pathLength; i++) {
			// eulerPathBad[i] = sf::Vertex(worldToPixel3D(currentValueEulerBad, cameraPos, cameraRot, screenPixelSize, 90), sf::Color::Blue);
			// eulerPathGood[i] = sf::Vertex(worldToPixel3D(currentValueEulerGood, cameraPos, cameraRot, screenPixelSize, 90), sf::Color::Yellow);	
			rungeKuttaPath[i] = sf::Vertex(worldToPixel3D(currentValueRungeKutta, cameraPos, cameraRot, screenPixelSize, 90), sf::Color::White);

			// currentValueEulerBad += vectorFieldVector(currentValueEulerBad.x, currentValueEulerBad.y, currentValueEulerBad.z) * dt;
			// currentValueEulerGood += vectorFieldVector(currentValueEulerGood.x, currentValueEulerGood.y, currentValueEulerGood.z) * dt / 100.0;
			currentValueRungeKutta += rungeKuttaStep(currentValueRungeKutta, dt);
		}

		// window.draw(eulerPathBad, pathLength, sf::LineStrip);
		// window.draw(eulerPathGood, pathLength, sf::LineStrip);
		window.draw(rungeKuttaPath, pathLength, sf::LineStrip);


		// drawing arrows
		// for (int i = 0; i < vectorAmount; i++) {
		// 	for (int j = 0; j < vectorAmount; j++) {
		// 		for (int k = 0; k < vectorAmount; k++) {
		// 			float worldX = (float)i; // 0  to  vectorAmount
		// 			float worldY = (float)j;
		// 			float worldZ = (float)k;

		// 			worldX /= (float)vectorAmount; // 0  to  1
		// 			worldY /= (float)vectorAmount;
		// 			worldZ /= (float)vectorAmount;

		// 			worldX *= screenWorldSize; // 0  to  world size
		// 			worldY *= screenWorldSize;
		// 			worldZ *= screenWorldSize;

		// 			worldX -= screenWorldSize/2.0f; // -1/2 world size  to  1/2 world size
		// 			worldY -= screenWorldSize/2.0f;
		// 			worldZ -= screenWorldSize/2.0f;


		// 			Vector3 vector = vectorFieldVector(worldX, worldY, worldZ); // Sample phase space direction vector from current point
		// 			// Vector2 drawingVector = normalized(vector); // Normalize for drawing purposes
		// 			Vector3 drawingVector = vector;

		// 			double magnitude = Vector3::length(vector);

		// 			drawingVector /= weight;
		// 			drawingVector += drawingVector.Normalized() * (weight-1 / weight);

		// 			Vector3 worldOrigin(worldX, worldY, worldZ);
		// 			Vector3 worldEnd(worldX + drawingVector.x * scale, worldY + drawingVector.y * scale, worldZ + drawingVector.z * scale);

		// 			double dist = Vector3::distance(cameraPos, worldOrigin);
		// 			double fade = 1.0 / (1.0 + dist * 0.1); 

		// 			sf::Color base = logBlueCyanYellowRed(magnitude, 0.01f, 100.0f);

		// 			sf::Color finalColor(
		// 				static_cast<sf::Uint8>(base.r * fade),
		// 				static_cast<sf::Uint8>(base.g * fade),
		// 				static_cast<sf::Uint8>(base.b * fade)
		// 			);


		// 			sf::Vertex lineSegment[] =
		// 			{
		// 				sf::Vertex(worldToPixel3D(worldOrigin, cameraPos, cameraRot, screenPixelSize, 90.0), finalColor), // origin
		// 				sf::Vertex(worldToPixel3D(worldEnd, cameraPos, cameraRot, screenPixelSize, 90.0), sf::Color::Transparent)  // direction
		// 			};

		// 			window.draw(lineSegment, 2, sf::Lines);
		// 		}
		// 	}
		// }

		drawAxes(window, screenPixelSize, cameraPos, cameraCenter, cameraRot, cameraDistanceFromCenter);
		std::cout << "\033[2J" << "\n";
        std::cout << "Pitch: " << cameraRot.x << "\n";
		std::cout << "Yaw: " << cameraRot.y << "\n";

		window.display();
	}
}
