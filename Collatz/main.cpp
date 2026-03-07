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

int main () {
	int screenPixelSize = 1300; // Pixel length of the screen
	sf::RenderWindow window(sf::VideoMode(screenPixelSize, screenPixelSize), "colalz");
	window.setFramerateLimit(60);

	// Camera variables
	Vector3 cameraPos(0.0, 0.0, -20.0);
	Vector3 cameraCenter(0.0, 0.0, 20.0);
	double cameraDistanceFromCenter = 40.0;
	Vector2 cameraRot;
	cameraRot.x = -(M_PI/1.5);
	cameraRot.y = 3.7;
	double cameraPanSpeed = 0.05;
	double cameraMoveSpeed = 0.5;

    // Graph variables
    int maxStartValues = 1000;
    int maxIterations = 999;

    std::vector<std::vector<Vector3>> graphPoints(
    maxStartValues,
    std::vector<Vector3>(maxIterations)
    );

    for (int i = 0; i < maxStartValues; i++) {
        int value = i + 1;

        for (int j = 0; j < maxIterations; j++) {

            if (value % 2 == 1) value = value * 3 + 1;
            else value /= 2;

            graphPoints[i][j] = Vector3(i * 5, value, j * 2);
        }
    }

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear();

        for (int i = 0; i < maxStartValues; i++) {

        std::vector<sf::Vertex> projected(maxIterations);

        for (int j = 0; j < maxIterations; j++) {

            sf::Vector2f pixel = worldToPixel3D(
                graphPoints[i][j],
                cameraPos,
                cameraRot,
                screenPixelSize,
                90.0
            );

            projected[j] = sf::Vertex(pixel, sf::Color::White);
        }

        window.draw(&projected[0], maxIterations, sf::LineStrip);
    }

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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			cameraCenter.y += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			cameraCenter.y -= cameraMoveSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			cameraDistanceFromCenter += cameraMoveSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && cameraDistanceFromCenter > 5.0) {
			cameraDistanceFromCenter -= cameraMoveSpeed;
		}
		cameraPos = cameraCenter + Vector3::getForward(cameraRot) * -cameraDistanceFromCenter;

        drawAxes(window, screenPixelSize, cameraPos, cameraCenter, cameraRot, cameraDistanceFromCenter);


		window.display();
	}
}
