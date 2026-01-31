#include <SFML/Graphics.hpp>
#include <iostream>

int main() {

    int screenDiameter = 1000;

    sf::RenderWindow window(
        sf::VideoMode(screenDiameter, screenDiameter),
        "polenomal visualison"
    );
    window.setFramerateLimit(1);

    sf::Shader shader;
    if (!shader.loadFromFile("polynomial.frag", sf::Shader::Fragment)) {
        std::cout << "failed to load hsader\n";
        return -1;
    }

    sf::RectangleShape quad(sf::Vector2f((float)screenDiameter, (float)screenDiameter));
    // quad.setScale(1.0f, 2.0f);

    int degree = 13;
    float dotRadius = 0.006f;

    shader.setUniform("u_coeff1", sf::Glsl::Vec2(1.8f, 1.0f));
    shader.setUniform("u_coeff2", sf::Glsl::Vec2(-1.3f, 0.0f));

    shader.setUniform("u_degree", degree);
    shader.setUniform("u_sigma", dotRadius);

    shader.setUniform("u_maxReal", 2.0f);
    shader.setUniform("u_maxImag", 2.0f);

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
