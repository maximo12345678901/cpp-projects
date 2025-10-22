#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <random>
#include <omp.h>

// Utility: flatten 2D indices into 1D
inline int idx(int x, int y, int w) {
    return y * w + x;
}

// Utility: set the quad color for a grid cell
inline void setCellColor(sf::VertexArray& va, int x, int y, int width, int cellSize, sf::Color color) {
    int i = (y * width + x) * 4;
    sf::Vector2f pos(x * cellSize, y * cellSize);

    va[i + 0].position = pos;
    va[i + 1].position = pos + sf::Vector2f(cellSize, 0);
    va[i + 2].position = pos + sf::Vector2f(cellSize, cellSize);
    va[i + 3].position = pos + sf::Vector2f(0, cellSize);

    va[i + 0].color = color;
    va[i + 1].color = color;
    va[i + 2].color = color;
    va[i + 3].color = color;
}

// Paint a region of cells with a given state
void Draw(std::vector<int>& cells, const sf::Vector2i& pos, int radius,
          int state, int width, int height) {
    const int xmin = std::max(0, pos.x - radius);
    const int xmax = std::min(width - 1, pos.x + radius);
    const int ymin = std::max(0, pos.y - radius);
    const int ymax = std::min(height - 1, pos.y + radius);

    for (int x = xmin; x <= xmax; ++x)
        for (int y = ymin; y <= ymax; ++y)
            cells[idx(x, y, width)] = state;
}

// Rule definition
struct Rule {
    int currentState;
    int nextState;
    int requiredNeighbors;
    int neighborType;
};

// Rule input helper (with random support)
void readRules(std::vector<Rule>& rules, std::mt19937& gen) {
    while (true) {
        std::string input;
        std::cin >> input;

        if (input == "break") {
            break;
        }

        if (input == "random") {
            int amountOfRules;
            std::cout << "how many? ";
            std::cin >> amountOfRules;

            std::uniform_int_distribution<> dist0_4(0, 4);
            std::uniform_int_distribution<> dist0_8(0, 8);

            for (int i = 0; i < amountOfRules; ++i) {
                int firstDigit = dist0_4(gen);
                int secondDigit;
                do {
                    secondDigit = dist0_4(gen);
                } while (secondDigit == firstDigit);

                int thirdDigit = dist0_8(gen);
                int fourthDigit = dist0_4(gen);

                Rule r{firstDigit, secondDigit, thirdDigit, fourthDigit};
                std::cout << r.currentState << r.nextState
                          << r.requiredNeighbors << r.neighborType << "\n";

                rules.push_back(r);
            }
            break;
        }

        if (input.size() != 4) {
            std::cerr << "Error: input must be exactly 4 characters long.\n";
            continue;
        }

        Rule r{
            input[0] - '0', // current
            input[1] - '0', // next
            input[2] - '0', // neighbors
            input[3] - '0'  // neighbor type
        };

        rules.push_back(r);
    }
}

int main() {
    int width, height, cellSize;

    // RNG setup
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Rule> rules;

    // Input parameters
    std::cout << "Enter the width of the map: ";
    std::cin >> width;

    std::cout << "Enter the height of the map: ";
    std::cin >> height;

    std::cout << "Enter the pixel size of each cell: ";
    std::cin >> cellSize;

    // Initial states
    std::vector<int> cellStates(width * height, 0);
    std::vector<int> newStates(width * height, 0);

    std::cout << "Add as many rules as you wish.\n"
              << "Format: 4 digits (e.g. 0123)\n"
              << "1st = current cell state\n"
              << "2nd = next state\n"
              << "3rd = number of neighbors\n"
              << "4th = neighbor state\n";

    readRules(rules, gen);

    // Setup window and grid
    sf::RenderWindow window(
        sf::VideoMode({width * cellSize, height * cellSize}),
        "Game of Life (Twist)");
    window.setFramerateLimit(60);

    sf::VertexArray grid(sf::Quads, width * height * 4);

    while (window.isOpen()) {
        sf::Vector2f windowMousePos =
            window.mapPixelToCoords(sf::Mouse::getPosition(window));

        sf::Vector2i hoveredPixel{
            std::clamp<int>(windowMousePos.x / cellSize, 0, width - 1),
            std::clamp<int>(windowMousePos.y / cellSize, 0, height - 1)
        };

        // Event loop
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        // Hotkeys for editing
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            rules.clear();
            readRules(rules, gen);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
            Draw(cellStates, hoveredPixel, 50, 0, width, height);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::U))
            Draw(cellStates, hoveredPixel, 2, 1, width, height);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
            Draw(cellStates, hoveredPixel, 2, 2, width, height);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
            Draw(cellStates, hoveredPixel, 2, 3, width, height);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            Draw(cellStates, hoveredPixel, 2, 4, width, height);

        // Simulation step
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                std::array<int, 5> neighborCounts{}; // states 0–4

                // Count neighbors
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        if (i == 0 && j == 0) continue;

                        int nx = x + i, ny = y + j;
                        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                            continue;

                        int s = cellStates[idx(nx, ny, width)];
                        if (s >= 0 && s <= 4)
                            neighborCounts[s]++;
                    }
                }

                int currentState = cellStates[idx(x, y, width)];
                int newState = currentState;

                // Apply rules
                for (const Rule& r : rules) {
                    if (currentState == r.currentState &&
                        neighborCounts[r.neighborType] == r.requiredNeighbors) {
                        newState = r.nextState;
                        break;
                    }
                }

                // Choose color
                sf::Color c = sf::Color::Black;
                switch (newState) {
                    case 1: c = sf::Color::White; break;
                    case 2: c = sf::Color::Red;   break;
                    case 3: c = sf::Color::Blue;  break;
                    case 4: c = sf::Color::Green; break;
                }

                setCellColor(grid, x, y, width, cellSize, c);
                newStates[idx(x, y, width)] = newState;
            }
        }

        // Swap buffers
        std::swap(cellStates, newStates);

        window.draw(grid);
        window.display();
    }

    return 0;
}
