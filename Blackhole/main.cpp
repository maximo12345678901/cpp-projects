#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

int main() {
    int width = 320;
    int height = 180;

    int displayScale = 8;

    float fov = 90;
    sf::RenderWindow window(sf::VideoMode(width * displayScale, height * displayScale), "blac hol");

    sf::Vector3f cameraPos;
    sf::Vector3f cameraDirPolar;

    float theta  = 0.0;       // angle from +Z axis
    float phi    = 0.0;        // angle in X–Y plane from +X

    cameraDirPolar = sf::Vector3f(1.0, theta, phi);
    cameraPos = sf::Vector3f(0.0, 0.0, 0.0);

    float rayStepSize = 0.1;
    int rayIterations = 500;
    sf::Vector3f blackholePos(0.0, -0.2, 10.0);
    float blackholeRadius = 0.1;
    float blackholeMass = 0.01;


    sf::Vector2i lastMousePos(width * displayScale / 2.0, height * displayScale / 2.0);
    window.setMouseCursorVisible(false);
    sf::Mouse::setPosition(lastMousePos, window);

    float sensitivity = 0.2f; // adjust to taste

    // Keep window open
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Image image;
        image.create(width, height, sf::Color::Black);


        // --- Mouse input ---
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float dx = static_cast<float>(mousePos.x - lastMousePos.x);
        float dy = static_cast<float>(mousePos.y - lastMousePos.y);

        // update yaw and pitch in cameraDirPolar
        cameraDirPolar.z += dx * sensitivity;   // yaw
        cameraDirPolar.y += dy * sensitivity;   // pitch

        // clamp pitch to avoid flipping
        if (cameraDirPolar.y > 89.0f) cameraDirPolar.y = 89.0f;
        if (cameraDirPolar.y < -89.0f) cameraDirPolar.y = -89.0f;

        // reset mouse to center for next frame
        sf::Mouse::setPosition(lastMousePos, window);

        // CAMERA MOVEMENT
        float moveSpeed = 0.1f;

        // convert to radians
        float yawRad = cameraDirPolar.z / 57.2958f;
        float pitchRad = cameraDirPolar.y / 57.2958f;

        // forward vector (based on yaw/pitch)
        sf::Vector3f forward(
            std::cos(pitchRad) * std::sin(yawRad),
            std::sin(pitchRad),
            std::cos(pitchRad) * std::cos(yawRad)
        );

        // right vector (perpendicular to forward)
        sf::Vector3f right(
            std::sin(yawRad - 3.14159f / 2.0f),
            0.0f,
            std::cos(yawRad - 3.14159f / 2.0f)
        );

        // up vector (world up)
        sf::Vector3f up(0.0f, 1.0f, 0.0f);

        // keyboard movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            cameraPos += forward * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            cameraPos -= forward * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            cameraPos += right * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            cameraPos -= right * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            cameraPos -= up * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            cameraPos += up * moveSpeed;
        }

        #pragma omp parallel for collapse(2)
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                float yaw = ((x - width / 2.0) * fov / width) + cameraDirPolar.z;
                float pitch = ((y - height / 2.0) * fov / height) +  + cameraDirPolar.y;

                yaw /= 57.2958;
                pitch /= 57.2958;

                sf::Vector3f rayDirection(
                    std::cos(pitch) * std::sin(yaw),
                    std::sin(pitch),
                    std::cos(pitch) * std::cos(yaw)
                );

                sf::Vector3f rayPosition(cameraPos);

                float r_x = rayDirection.x;
                float r_y = rayDirection.y;
                float r_z = rayDirection.z;

                float len = std::sqrt(
                    r_x * r_x +
                    r_y * r_y +
                    r_z * r_z);

                rayDirection.x /= len;
                rayDirection.y /= len;
                rayDirection.z /= len;

                for (int i = 0; i < rayIterations; i++) {
                    sf::Vector3f direction = blackholePos - rayPosition;
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
                    if (distance < blackholeRadius) {
                        image.setPixel(x, y, sf::Color::Black);
                        break;
                    }

                    // if (fabs(rayPosition.y - blackholePos.y) < 0.15 &&
                    //     distance > blackholeRadius && distance < 5.0)
                    // {
                    //     image.setPixel(x, y, sf::Color::White);
                    //     break;
                    // }
                    sf::Vector3f attractionForce((blackholeMass / (distance * distance)) * direction);
                    rayDirection += attractionForce;
                    float directionMagnitude = std::sqrt(rayDirection.x * rayDirection.x + 
                        rayDirection.y * rayDirection.y +
                        rayDirection.z * rayDirection.z);
                    rayDirection /= directionMagnitude;
                    rayPosition += rayDirection * rayStepSize;

                    if (i == rayIterations - 1) {
                        sf::Uint8 R = static_cast<sf::Uint8>((rayDirection.x) * 255);
                        sf::Uint8 G = static_cast<sf::Uint8>((rayDirection.y) * 255);
                        sf::Uint8 B = static_cast<sf::Uint8>((rayDirection.z) * 255);

                        image.setPixel(x, y, sf::Color(R, G, B));
                        break;
                    }
                }
            }
        }

        sf::Texture texture;
        texture.loadFromImage(image);
        
        sf::Sprite sprite(texture);
        sprite.setScale(displayScale, displayScale);

        window.clear();
        window.draw(sprite);
        window.display();

        sf::Vector3f cameraDirection(
                std::sin(cameraDirPolar.y) * std::cos(cameraDirPolar.z),
                std::sin(cameraDirPolar.y) * std::sin(cameraDirPolar.z),
                std::cos(cameraDirPolar.y)
                );
        std::cout << "\x1b[2J";
        std::cout << "Camera direction (cartesian): (" << cameraDirection.x << ", " << cameraDirection.y << ", " << cameraDirection.z << ")\n";
        std::cout << "Camera direction (polar): (" << cameraDirPolar.y << ", " << cameraDirPolar.z << ")\n";
    }

    return 0;
}