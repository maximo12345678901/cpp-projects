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




Vector4 vectorFieldVector(double x, double y, double z, double w) {
	//double dx = (z-0.7)*x-3.5*y;
	//double dy = 3.5*x+(z-0.7)*y;
	//double dz = 0.6+0.95*z-((z*z*z)/3)-x*x+0.1*z*x*x*x;
	//double dx = 10*(y-x);
	//double dy = x*(28-z)-y;
	//double dz = x*y-(8/3)*z;

	double delta = x - z;
	double denom = (3.0 - std::cos(2.0*delta));

	double dx = y;
	double dy = (
		-3.0 * 10 * std::sin(x)
		- 10 * std::sin(x - 2.0*z)
		- 2.0 * std::sin(delta) * (w*w + y*y * std::cos(delta))
	) / denom;

	double dz = w;
	double dw = (
		2.0 * std::sin(delta) * (
			2.0*y*y + 2.0*10*std::cos(x)
			+ w*w * std::cos(delta))
	) / denom;

	return Vector4(dx, dy, dz, dw);
}

Vector4 rungeKuttaStep(Vector4 current, double dt) {
	Vector4 k1 = vectorFieldVector(current.x, current.y, current.z, current.w);
	Vector4 k2 = vectorFieldVector(current.x + 0.5f * dt * k1.x, current.y + 0.5f * dt * k1.y, current.z + 0.5f * dt * k1.z, current.w + 0.5f * dt * k1.w);
	Vector4 k3 = vectorFieldVector(current.x + 0.5f * dt * k2.x, current.y + 0.5f * dt * k2.y, current.z + 0.5f * dt * k2.z, current.w + 0.5f * dt * k2.w);
	Vector4 k4 = vectorFieldVector(current.x + dt * k3.x, current.y + dt * k3.y, current.z + dt * k3.z, current.w + dt * k3.w);

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
	float dt = 0.005;
	Vector4 trajectoryStart(-2.25223, 0.0, -0.746236, 0.0);


	// Camera variables
	Vector3 cameraPos(0.0, 0.0, 0.0);
	Vector3 cameraCenter(0.0, 0.0, 0.0);
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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
			cameraRot.y -= cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
			cameraRot.y += cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
			cameraRot.x -= cameraPanSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
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

		// sf::Vertex eulerPathBad[pathLength];
		// sf::Vertex eulerPathGood[pathLength];
		sf::Vertex rungeKuttaPath[pathLength];

		// Vector3 currentValueEulerBad = trajectoryStart;
		// Vector3 currentValueEulerGood = trajectoryStart;
		Vector4 currentValueRungeKutta = trajectoryStart;

		for (int i = 0; i < pathLength; i++) {
			// eulerPathBad[i] = sf::Vertex(worldToPixel3D(currentValueEulerBad, cameraPos, cameraRot, screenPixelSize, 90), sf::Color::Blue);
			// eulerPathGood[i] = sf::Vertex(worldToPixel3D(currentValueEulerGood, cameraPos, cameraRot, screenPixelSize, 90), sf::Color::Yellow);	
			rungeKuttaPath[i] = sf::Vertex(worldToPixel3D(Vector4::vec4tovec3(currentValueRungeKutta), cameraPos, cameraRot, screenPixelSize, 90), sf::Color(255, 255, 255 - std::abs(currentValueRungeKutta.w) * 10, 255));

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
