#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <cmath>

int main() {
    // Base resolution
    int width = 400;
    int height = 300;
    int displayScale = 4;

    float fov = 90.0f;

    // Create window
    sf::RenderWindow window(sf::VideoMode(width * displayScale, height * displayScale), "blac hol");
    window.setFramerateLimit(60);

    // Camera
    sf::Vector3f cameraPos(0.0f, 0.0f, 0.0f);
    sf::Vector3f cameraDirPolar(0.0f, 0.0f, 0.0f); // yaw (z), pitch (y)

    // Black hole
    sf::Vector3f blackholePos(0.0f, -0.2f, 10.0f);
    float blackholeRadius = 0.1f;
    float blackholeMass = 0.05f;
    float rayStepSize = 0.1f;
    int rayIterations = 2000;
    float accretionDiskRadius = 3.0;

    // Mouse
    sf::Vector2i lastMousePos(width * displayScale / 2, height * displayScale / 2);
    window.setMouseCursorVisible(false);
    sf::Mouse::setPosition(lastMousePos, window);
    float sensitivity = 0.2f;

    // Load shader
    sf::Shader shader;
    if (!shader.loadFromFile("blackhole.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load shader\n";
        return -1;
    }

    // Render texture at base resolution
    sf::RenderTexture renderTexture;
    renderTexture.create(width, height);
    sf::RectangleShape screen(sf::Vector2f(width, height));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // --- Mouse input ---
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float dx = static_cast<float>(mousePos.x - lastMousePos.x);
        float dy = static_cast<float>(mousePos.y - lastMousePos.y);

        cameraDirPolar.z -= dx * sensitivity; // yaw
        cameraDirPolar.y -= dy * sensitivity; // pitch
        if (cameraDirPolar.y > 89.0f) cameraDirPolar.y = 89.0f;
        if (cameraDirPolar.y < -89.0f) cameraDirPolar.y = -89.0f;
        sf::Mouse::setPosition(lastMousePos, window);

        // --- Keyboard input ---
        float moveSpeed = 0.1f;
        float yawRad = cameraDirPolar.z * 3.14159f / 180.0f;
        float pitchRad = cameraDirPolar.y * 3.14159f / 180.0f;

        sf::Vector3f forward(cos(pitchRad) * sin(yawRad), sin(pitchRad), cos(pitchRad) * cos(yawRad));
        sf::Vector3f right(sin(yawRad - 3.14159f/2.0f), 0.0f, cos(yawRad - 3.14159f/2.0f));
        sf::Vector3f up(0.0f, 1.0f, 0.0f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) cameraPos += forward * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) cameraPos -= forward * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) cameraPos -= right * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) cameraPos += right * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) cameraPos += up * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) cameraPos -= up * moveSpeed;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) blackholeMass += 0.01;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) blackholeMass -= 0.01;


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) blackholeRadius += 0.01;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) blackholeRadius -= 0.01;

        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) accretionDiskRadius += 0.1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) accretionDiskRadius -= 0.1;

        // --- Shader uniforms ---
        
        shader.setUniform("cameraPos", sf::Glsl::Vec3(cameraPos.x, cameraPos.y, cameraPos.z));
        shader.setUniform("yaw", cameraDirPolar.z);
        shader.setUniform("pitch", cameraDirPolar.y);
        shader.setUniform("blackholePos", sf::Glsl::Vec3(blackholePos.x, blackholePos.y, blackholePos.z));
        shader.setUniform("blackholeRadius", blackholeRadius);
        shader.setUniform("blackholeMass", blackholeMass);
        shader.setUniform("rayStepSize", rayStepSize);
        shader.setUniform("rayIterations", rayIterations);
        shader.setUniform("screenWidth", static_cast<float>(width));
        shader.setUniform("screenHeight", static_cast<float>(height));
        shader.setUniform("fov", fov);
        shader.setUniform("accretionDiskRadius", accretionDiskRadius);

        // --- Render to texture ---
        renderTexture.clear();
        renderTexture.draw(screen, &shader);
        renderTexture.display();

        // --- Draw scaled to window ---
        sf::Sprite sprite(renderTexture.getTexture());
        sprite.setScale(displayScale, displayScale);

        window.clear();
        window.draw(sprite);
        window.display();
        std::cout << "\033[2J" << "\n";
        std::cout << "Blackhole mass: " << blackholeMass << "\n";
        std::cout << "Blackhole radius: " << blackholeRadius << "\n";
        std::cout << "Camera rotation (pitch, yaw): (" << cameraDirPolar.y << ", " << cameraDirPolar.z << ")" << "\n";
        std::cout << "Camera position (x, y, z): (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << "\n";
    }

    return 0;
}
