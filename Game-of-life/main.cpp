// main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <string>

struct Rule {
    int currentState;
    int nextState;
    int requiredNeighbors;
    int neighborType;
};

int main() {
    int width, height, cellSize;
    std::cout << "Enter grid width: ";
    std::cin >> width;
    std::cout << "Enter grid height: ";
    std::cin >> height;
    std::cout << "Enter cell pixel size: ";
    std::cin >> cellSize;

    std::vector<Rule> rules;
    std::cout << "Enter rules (4 digits each, e.g. 0123), type 'break' to finish:\n";
    while (true) {
        std::string input;
        std::cin >> input;
        if (input == "break") break;
        if (input.size() != 4) {
            std::cerr << "Error: input must be exactly 4 chars.\n";
            continue;
        }
        rules.push_back(Rule{
            input[0] - '0',
            input[1] - '0',
            input[2] - '0',
            input[3] - '0'
        });
    }

    sf::RenderWindow window(
        sf::VideoMode(width * cellSize, height * cellSize),
        "GPU Cellular Automaton",
        sf::Style::Fullscreen,
        sf::ContextSettings(24, 8, 0, 3, 3)
    );
    window.setFramerateLimit(60);

    sf::VertexArray quad(sf::TrianglesStrip, 4);
    quad[0].position = { 0.f, 0.f };
    quad[1].position = { float(width * cellSize), 0.f };
    quad[2].position = { 0.f, float(height * cellSize) };
    quad[3].position = { float(width * cellSize), float(height * cellSize) };

    quad[0].texCoords = { 0.f, 0.f };
    quad[1].texCoords = { 1.f, 0.f };
    quad[2].texCoords = { 0.f, 1.f };
    quad[3].texCoords = { 1.f, 1.f };

    sf::RenderTexture texA, texB;
    if (!texA.create(width, height) || !texB.create(width, height)) {
        std::cerr << "Failed to create RenderTexture (check grid size)\n";
        return 1;
    }

    // disable smoothing on RenderTextures
    texA.setSmooth(false);
    texB.setSmooth(false);

    // initialize to black
    texA.clear(sf::Color::Black); texA.display();
    texB.clear(sf::Color::Black); texB.display();

    sf::Shader updateShader, renderShader;
    if (!updateShader.loadFromFile("update.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load update.frag\n";
        return 1;
    }
    if (!renderShader.loadFromFile("render.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load render.frag\n";
        return 1;
    }

    updateShader.setUniform("ruleCount", static_cast<int>(rules.size()));
    for (size_t i = 0; i < rules.size(); ++i) {
        std::string base = "rules[" + std::to_string(i) + "].";
        updateShader.setUniform(base + "currentState", rules[i].currentState);
        updateShader.setUniform(base + "nextState", rules[i].nextState);
        updateShader.setUniform(base + "requiredNeighbors", rules[i].requiredNeighbors);
        updateShader.setUniform(base + "neighborType", rules[i].neighborType);
    }

    updateShader.setUniform("gridSize", sf::Glsl::Vec2(float(width), float(height)));
    renderShader.setUniform("gridSize", sf::Glsl::Vec2(float(width), float(height)));

    bool useA = true;

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
        }

        if (useA) {
            updateShader.setUniform("stateTex", texA.getTexture());
            texB.clear(sf::Color::Black);
            texB.draw(quad, &updateShader);
            texB.display();
        } else {
            updateShader.setUniform("stateTex", texB.getTexture());
            texA.clear(sf::Color::Black);
            texA.draw(quad, &updateShader);
            texA.display();
        }
        useA = !useA;

        window.clear();
        if (useA) {
            renderShader.setUniform("stateTex", texA.getTexture());
            window.draw(quad, &renderShader);
        } else {
            renderShader.setUniform("stateTex", texB.getTexture());
            window.draw(quad, &renderShader);
        }
        window.display();
    }

    return 0;
}
