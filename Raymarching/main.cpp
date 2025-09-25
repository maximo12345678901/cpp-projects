#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

struct Camera {
    sf::Vector3f position {0.f, 0.f, 5.f};
    float yaw = -90.f;
    float pitch = 0.f;
    float speed = 50.f;
    float sensitivity = 0.1f;
};

sf::Vector3f getForward(const Camera& cam) {
    float cy = std::cos(cam.yaw * 3.14159f / 180.f);
    float sy = std::sin(cam.yaw * 3.14159f / 180.f);
    float cp = std::cos(cam.pitch * 3.14159f / 180.f);
    float sp = std::sin(cam.pitch * 3.14159f / 180.f);

    return sf::Vector3f(cy * cp, sp, sy * cp);
}

sf::Vector3f getRight(const Camera& cam) {
    sf::Vector3f f = getForward(cam);
    return sf::Vector3f(f.z, 0.f, -f.x);
}



int main() {

    float v = 10;
    const unsigned WIDTH = 1600;
    const unsigned HEIGHT = 900;

    bool controlling = true;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "raymarching yay");
    window.setPosition(sf::Vector2i(10, 10));
    // window.setFramerateLimit(60);
    window.setMouseCursorGrabbed(true);
    window.setMouseCursorVisible(false);

    sf::Shader shader;
    if (!shader.loadFromFile("raymarch.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load shader\n";
        return -1;
    }

    sf::RectangleShape screenQuad(sf::Vector2f(WIDTH, HEIGHT));
    screenQuad.setPosition(0, 0);

    Camera cam;
    cam.position.y = 3400000000.0;
    sf::Clock clock;

    sf::Vector2i center(WIDTH / 2, HEIGHT / 2);
    sf::Mouse::setPosition(center, window);

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.setMouseCursorGrabbed(false);
                    window.setMouseCursorVisible(true);
                    controlling = false;
                }
            }
        }

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        sf::Vector2i delta = mouse - center;
        
        if (controlling) {
            sf::Mouse::setPosition(center, window);
        }

        cam.yaw += delta.x * cam.sensitivity;
        cam.pitch -= delta.y * cam.sensitivity;
        cam.pitch = std::clamp(cam.pitch, -89.f, 89.f);

        // v += 5 * dt;
        // cam.position.y += v;


        sf::Vector3f forward = getForward(cam);
        sf::Vector3f right = getRight(cam);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            cam.position -= forward * (cam.speed * dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cam.position += forward * (cam.speed * dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cam.position -= right * (cam.speed * dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cam.position += right * (cam.speed * dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            cam.position.y -= cam.speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
            cam.position.y += cam.speed * dt;

        shader.setUniform("iResolution", sf::Vector2f(WIDTH, HEIGHT));
        shader.setUniform("camPos", cam.position);
        shader.setUniform("camYaw", cam.yaw);
        shader.setUniform("camPitch", cam.pitch);

        window.clear();
        window.draw(screenQuad, &shader);
        window.display();
    }

    return 0;
}
