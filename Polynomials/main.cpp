#include <SFML/Graphics.hpp>
#include <iostream>

int main() {

    int screenDiameter = 1000;

    sf::RenderWindow window(
        sf::VideoMode(screenDiameter, screenDiameter),
        "polynomial visualisation"
    );
    window.setFramerateLimit(60);

    // Load shader
    sf::Shader shader;
    if (!shader.loadFromFile("polynomial.frag", sf::Shader::Fragment)) {
        std::cout << "Failed to load shader!\n";
        return -1;
    }

    // Fullscreen quad for shader rendering
    sf::RectangleShape quad(sf::Vector2f((float)screenDiameter, (float)screenDiameter));
    // quad.setScale(2.0f, 2.0f);  // scale to fill the window

    int degree = 14;  // number of coefficients
    float sigma = 0.1f;

    shader.setUniform("u_coeff1", sf::Glsl::Vec2(-2.f, 0.f));
    shader.setUniform("u_coeff2", sf::Glsl::Vec2(2.f, 0.f));

    shader.setUniform("u_degree", degree);
    shader.setUniform("u_sigma", sigma);

    shader.setUniform("u_maxReal", 0.8f);
    shader.setUniform("u_maxImag", 0.8f);

    shader.setUniform("u_resolution", sf::Glsl::Vec2((float)screenDiameter, (float)screenDiameter));

    window.clear();
    window.draw(quad, &shader);
    window.display();

    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    return 0;
}
