#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <random>
#include <omp.h>

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

void Draw(std::vector<std::vector<int>>& cells, const sf::Vector2i& pos, int radius,
          int state, int width, int height) {
    const int xmin = std::max(0, pos.x - radius);
    const int xmax = std::min(width - 1, pos.x + radius);
    const int ymin = std::max(0, pos.y - radius);
    const int ymax = std::min(height - 1, pos.y + radius);

    for (int x = xmin; x <= xmax; ++x)
        for (int y = ymin; y <= ymax; ++y)
            cells[x][y] = state;
}

struct Rule {
    int currentState;
    int nextState;
    int requiredNeighbors;
    int neighborType;
};

void readRules(std::vector<Rule>& rules, std::mt19937& gen) {
    std::string line;

    // Clear leftover newline if needed
    std::getline(std::cin, line);

    while (true) {
        std::getline(std::cin, line);

        if (!std::cin) break;          // EOF
        if (line.empty()) continue;    // skip empty lines

        if (line == "break") {
            break;
        }

        if (line == "random") {
            int amountOfRules;
            std::cout << "how many? ";
            std::cin >> amountOfRules;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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

                rules.push_back({firstDigit, secondDigit, thirdDigit, fourthDigit});

                std::cout << firstDigit << secondDigit << thirdDigit << fourthDigit << "\n";
            }
            continue;
        }

        if (line == "clear") {
            rules.clear();
            std::cout << "cleared \n";
            continue;
        }

        if (line.size() != 4) {
            std::cerr << "Error: each rule must be exactly 4 digits\n";
            continue;
        }

        Rule r{
            line[0] - '0',
            line[1] - '0',
            line[2] - '0',
            line[3] - '0'
        };

        rules.push_back(r);
    }
}


int main() {
    int width, height, cellSize;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Rule> rules;

//    std::cout << "width: ";
//    std::cin >> width;
      width = 300;
      height = 300;
      cellSize = 3;
//    std::cout << "height: ";
//    std::cin >> height;

//    std::cout << "pixel size: ";
//    std::cin >> cellSize;

    std::vector<std::vector<int>> cellStates(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> newStates(width, std::vector<int>(height, 0));

    readRules(rules, gen);

    
    sf::RenderWindow window(
        sf::VideoMode({width * cellSize, height * cellSize}), "game of life (with silly twist)");
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            cellStates = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
            newStates = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
        }

        #pragma omp parallel for collapse(2)
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                std::array<int, 5> neighborCounts{}; // states 0–4

                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        if (i == 0 && j == 0) continue;

                        int nx = x + i, ny = y + j;
                        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                            continue;

                        int s = cellStates[nx][ny];
                        if (s >= 0 && s <= 4)
                            neighborCounts[s]++;
                    }
                }

                int currentState = cellStates[x][y];
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
                newStates[x][y] = newState;
            }
        }

        // Swap buffers
        std::swap(cellStates, newStates);

        window.draw(grid);
        window.display();
    }

    return 0;
}
