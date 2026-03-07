#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <omp.h>
#include <iomanip>
#include <sstream>
#include "../../vec.h"

float clamp(float v, float minVal, float maxVal) {
    return std::max(minVal, std::min(v, maxVal));
}

float wrap(float min, float max, float value) {
    float newValue = value;
    while (newValue < min) newValue += max - min;
    while (newValue > max) newValue -= max - min;
    return newValue;
}

sf::Color hsvToRgb(float h, float s, float v) {
    h = fmod(h, 360.f);
    if (h < 0) h += 360.f;
    float c = v * s;
    float x = c * (1 - std::fabs(fmod(h / 60.f, 2) - 1));
    float m = v - c;
    float rP, gP, bP;
    if      (h < 60)  { rP = c; gP = x; bP = 0; }
    else if (h < 120) { rP = x; gP = c; bP = 0; }
    else if (h < 180) { rP = 0; gP = c; bP = x; }
    else if (h < 240) { rP = 0; gP = x; bP = c; }
    else if (h < 300) { rP = x; gP = 0; bP = c; }
    else              { rP = c; gP = 0; bP = x; }
    return sf::Color(
        static_cast<sf::Uint8>((rP + m) * 255),
        static_cast<sf::Uint8>((gP + m) * 255),
        static_cast<sf::Uint8>((bP + m) * 255)
    );
}

double angularAccel1(float l1, float l2,
                     double m1, double m2,
                     double th1, double th2,
                     double w1, double w2,
                     double g = 100)
{
    double delta = th1 - th2;
    double denom = l1 * (3.0 - std::cos(2.0 * delta));
    return (
        -g * 3 * std::sin(th1)
        - g * std::sin(th1 - 2.0 * th2)
        - 2.0 * std::sin(delta) * (w2*w2 * l2 + w1*w1 * l1 * std::cos(delta))
    ) / denom;
}

double angularAccel2(float l1, float l2,
                     double m1, double m2,
                     double th1, double th2,
                     double w1, double w2,
                     double g = 100)
{
    double delta = th1 - th2;
    double denom = l2 * (3.0 - std::cos(2.0 * delta));
    return (
        2.0 * std::sin(delta) * (
            w1*w1 * l1 * 2
            + g * 2 * std::cos(th1)
            + w2*w2 * l2 * std::cos(delta))
    ) / denom;
}

struct Derivs {
    double dth1, dw1, dth2, dw2;
};

class Pendulum {
    public:
        double dt;
        sf::Vector2f origin;
        sf::Vector2f firstDotPos;
        sf::Vector2f secondDotPos;

        sf::Color color;
        float l1;
        float l2;
        double m1;
        double m2;

        double th1;
        double th2;
        double w1;
        double w2;

        int dotRadius;
        int rodWidth;

        void UpdatePendulumRK4(double g, double dt) {
            auto derivs = [&](double th1, double w1, double th2, double w2) {
                double a1 = angularAccel1(l1, l2, m1, m2, th1, th2, w1, w2, g);
                double a2 = angularAccel2(l1, l2, m1, m2, th1, th2, w1, w2, g);
                return Derivs{ w1, a1, w2, a2 };
            };

            // k1
            auto k1 = derivs(th1, w1, th2, w2);

            // k2
            auto k2 = derivs(
                th1 + 0.5*dt*k1.dth1,
                w1 + 0.5*dt*k1.dw1,
                th2 + 0.5*dt*k1.dth2,
                w2 + 0.5*dt*k1.dw2);

            // k3
            auto k3 = derivs(
                th1 + 0.5*dt*k2.dth1,
                w1 + 0.5*dt*k2.dw1,
                th2 + 0.5*dt*k2.dth2,
                w2 + 0.5*dt*k2.dw2);

            // k4
            auto k4 = derivs(
                th1 + dt*k3.dth1,
                w1 + dt*k3.dw1,
                th2 + dt*k3.dth2,
                w2 + dt*k3.dw2);

            // Combine
            th1 += dt/6.0 * (k1.dth1 + 2*k2.dth1 + 2*k3.dth1 + k4.dth1);
            w1  += dt/6.0 * (k1.dw1  + 2*k2.dw1  + 2*k3.dw1  + k4.dw1);
            th2 += dt/6.0 * (k1.dth2 + 2*k2.dth2 + 2*k3.dth2 + k4.dth2);
            w2  += dt/6.0 * (k1.dw2  + 2*k2.dw2  + 2*k3.dw2  + k4.dw2);

            // Update positions
            firstDotPos = sf::Vector2f(origin.x + cos(th1 + (M_PI / 2)) * l1 * 100.0f,
                                    origin.y + sin(th1 + (M_PI / 2)) * l1 * 100.0f);
            secondDotPos = sf::Vector2f(firstDotPos.x + l2 * cos(th2 + (M_PI / 2)) * 100.0f,
                                        firstDotPos.y + l2 * sin(th2 + (M_PI / 2)) * 100.0f);
        }

        void DrawPendulum(sf::RenderWindow& window) {
            // Draw the origin dot
            sf::CircleShape originDot(dotRadius);
            originDot.setFillColor(color);
            originDot.setPosition(origin);
            originDot.setOrigin(sf::Vector2f(dotRadius, dotRadius));
            
            // Draw the first rod
            sf::VertexArray rod1(sf::Lines, 2);
            rod1[0].color = color;
            rod1[1].color = color;
            rod1[0].position = origin;
            rod1[1].position = firstDotPos;

            // Draw the first dot
            sf::CircleShape firstDot(dotRadius);
            firstDot.setFillColor(color);
            firstDot.setPosition(firstDotPos);
            firstDot.setOrigin(sf::Vector2f(dotRadius, dotRadius));

            // Draw the second rod
            sf::VertexArray rod2(sf::Lines, 2);
            rod2[0].color = color;
            rod2[1].color = color;
            rod2[0].position = firstDotPos;
            rod2[1].position = secondDotPos;
            
            // Draw the end dot
            sf::CircleShape secondDot(dotRadius);
            secondDot.setFillColor(color);
            secondDot.setPosition(secondDotPos);
            secondDot.setOrigin(sf::Vector2f(dotRadius, dotRadius));

            window.draw(originDot);
            window.draw(rod1);
            window.draw(firstDot);
            window.draw(rod2);
            window.draw(secondDot);
        }

        Pendulum() { // Constructor
            dt = 0.002;
            l1 = 1.8;
            l2 = 2.2;
            m1 = 1;
            m2 = 1;

            origin = sf::Vector2f(500, 500);

            color = sf::Color::White;

            th1 = 0;
            th2 = 0;
            w1 = 6.0;
            w2 = -6.0;

            // convert the angle to screen coordinates
            firstDotPos = sf::Vector2f(origin.x + l1 * cos(th1), origin.y + l1 * sin(th1));
            secondDotPos = sf::Vector2f(firstDotPos.x + l2 * cos(th2), firstDotPos.y + l2 * sin(th2));

            dotRadius = 10;
        }
};

int main() {
    bool running = true;
    bool doGenerateMap;

    std::cout << "\nstate space? (y/n)\n";
    std::string input;
    std::cin >> input;
    doGenerateMap = (input == "y" || input == "Y");

    int screenPixelSize;
    std::cout << "\nscreen size: ";
    std::cin >> screenPixelSize;
    const float graphWorldSize  = 2.0f * M_PI;

    sf::RenderWindow simulationWindow;
    sf::RenderWindow graphWindow;
    sf::RenderWindow mapWindow;

    if (!doGenerateMap) {
        simulationWindow.create(sf::VideoMode(screenPixelSize, screenPixelSize), "pendulum sim");
        graphWindow.create(sf::VideoMode(screenPixelSize, screenPixelSize), "angle graph");

        simulationWindow.setFramerateLimit(120);
    }

    // Color map setup
    sf::Image originalColorMap;
    if (!originalColorMap.loadFromFile("./colormap.png")) return 1;
    unsigned int cmW = originalColorMap.getSize().x;
    unsigned int cmH = originalColorMap.getSize().y;

    int resolution = 1000;
    if (doGenerateMap) {
        std::cout << "\nresolution: ";
        std::cin >> resolution;
    }
    sf::Image currentColorMap;
    currentColorMap.create(cmW, cmH);

    sf::Texture texture;
    texture.loadFromImage(currentColorMap);
    sf::Sprite sprite(texture);
    sprite.setScale(sf::Vector2f(screenPixelSize / 1000.0f, screenPixelSize / 1000.0f));

    std::vector<std::vector<sf::Color>> originalColorValues(cmH, std::vector<sf::Color>(cmW));
    std::vector<std::vector<sf::Color>> currentColorValues (cmH, std::vector<sf::Color>(cmW));
    std::vector<std::vector<Pendulum>>  pendulums(resolution, std::vector<Pendulum>(resolution));

    Vector2 viewCenter(0.0, 0.0);
    double viewSideLength = 2 * M_PI;
    
    

    if (doGenerateMap) {
        mapWindow.create(sf::VideoMode(screenPixelSize, screenPixelSize), "colormap");
        mapWindow.setFramerateLimit(60);

        std::cout << "\nx center: ";
        std::cin >> viewCenter.x;
        std::cout << "\ny center: ";
        std::cin >> viewCenter.y;
        std::cout << "\nview diameter: ";
        std::cin >> viewSideLength;
        std::cout << "\n";

        if (viewSideLength == 0) {
            viewSideLength = 2 * M_PI;
        }

        for (unsigned int y = 0; y < cmH; ++y)
            for (unsigned int x = 0; x < cmW; ++x)
                originalColorValues[y][x] = currentColorValues[y][x] = originalColorMap.getPixel(x, y);

        for (int i = 0; i < resolution; ++i) {
            for (int j = 0; j < resolution; ++j) {
                Pendulum p;
                double th1 = viewCenter.x + ((double)i / (resolution - 1) - 0.5) * viewSideLength;
                double th2 = viewCenter.y + ((double)j / (resolution - 1) - 0.5) * viewSideLength;
                p.th1 = th1;
                p.th2 = th2;
                pendulums[i][j] = p;
            }
        }
    }

    sf::Clock clock;
    int fpsCounter = 0;

    double G  = 100;
    double dt = 0.007;

    Pendulum pendulum;
    std::vector<sf::Vector2f> graphTrail;
    graphTrail.reserve(2500);

    std::vector<sf::Vector2f> pendulumTrail;
    pendulumTrail.reserve(300);

    if (!doGenerateMap) {
        std::cout << "\nth1: ";
        std::cin >> pendulum.th1;
        std::cout << "\nth2: ";
        std::cin >> pendulum.th2;
    }

    while (running) {
        sf::Event event;

        if (!doGenerateMap) {
            while (simulationWindow.pollEvent(event))
                if (event.type == sf::Event::Closed) running = false;
            while (graphWindow.pollEvent(event))
                if (event.type == sf::Event::Closed) running = false;
        }
        if (doGenerateMap) {
            while (mapWindow.pollEvent(event))
                if (event.type == sf::Event::Closed) running = false;
        }

        if (!doGenerateMap) {
            simulationWindow.clear(sf::Color(10, 10, 10));
            graphWindow.clear(sf::Color(0, 0, 0));

            double th1W = wrap(-M_PI, M_PI, pendulum.th1);
            double th2W = wrap(-M_PI, M_PI, pendulum.th2);
            sf::Vector2f graphPx = worldToPixel(Vector2(th1W, th2W), screenPixelSize, graphWorldSize);
            graphTrail.push_back(graphPx);
            if (graphTrail.size() > 2500)
                graphTrail.erase(graphTrail.begin());

            // Smooth fading polyline on graph
            for (size_t i = 1; i < graphTrail.size(); ++i) {
                sf::Vector2f delta = graphTrail[i] - graphTrail[i-1];
                if (std::abs(delta.x) > screenPixelSize * 0.5f || std::abs(delta.y) > screenPixelSize * 0.5f)
                    continue;

                float alpha = static_cast<float>(i) / graphTrail.size() * 255.f;
                sf::Color c(255, 255, 255, static_cast<sf::Uint8>(alpha));
                sf::Vertex line[2] = { sf::Vertex(graphTrail[i-1], c), sf::Vertex(graphTrail[i], c) };
                graphWindow.draw(line, 2, sf::Lines);
            }

            // Fading trail on pendulum tip
            pendulumTrail.push_back(pendulum.secondDotPos);
            if (pendulumTrail.size() > 300)
                pendulumTrail.erase(pendulumTrail.begin());

            for (size_t i = 1; i < pendulumTrail.size(); ++i) {
                float t = static_cast<float>(i) / pendulumTrail.size();
                sf::Color c(
                    255,
                    255,
                    255,
                    static_cast<sf::Uint8>(t * 255.f)
                );
                sf::Vertex line[2] = { sf::Vertex(pendulumTrail[i-1], c), sf::Vertex(pendulumTrail[i], c) };
                simulationWindow.draw(line, 2, sf::Lines);
            }

            pendulum.UpdatePendulumRK4(G, dt);
            pendulum.DrawPendulum(simulationWindow);
            simulationWindow.display();
            graphWindow.display();
        }

        if (doGenerateMap) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2i mousePx = sf::Mouse::getPosition(mapWindow);
                double th1 = viewCenter.x + ((double)mousePx.x / (screenPixelSize - 1) - 0.5) * viewSideLength;
                double th2 = viewCenter.y + ((double)mousePx.y / (screenPixelSize - 1) - 0.5) * viewSideLength;
                Vector2 worldPosition(th1, th2);
                worldPosition.say();
            }

            #pragma omp parallel for collapse(2)
            for (int x = 0; x < resolution; ++x) {
                for (int y = 0; y < resolution; ++y) {
                    pendulums[x][y].UpdatePendulumRK4(G, dt);

                    float th1 = wrap(-M_PI, M_PI, pendulums[x][y].th1);
                    float th2 = wrap(-M_PI, M_PI, pendulums[x][y].th2);

                    // Map current (th1, th2) to source pixel in the color map
                    sf::Vector2f srcPx = worldToPixel(Vector2(th1, th2), cmW, graphWorldSize);
                    int srcX = std::clamp((int)srcPx.x, 0, (int)cmW - 1);
                    int srcY = std::clamp((int)srcPx.y, 0, (int)cmH - 1);

                    int dstX = (int)((double)x / (resolution - 1) * (cmW - 1));
                    int dstY = (int)((double)y / (resolution - 1) * (cmH - 1));
                    
                    sf::Color c = originalColorValues[srcY][srcX];

                    int fill = std::ceil(screenPixelSize / resolution);
                    for (int i = 0; i < fill; ++i) {
                        for (int j = 0; j < fill; ++j) {
                            int px = dstX + i, py = dstY + j;
                            if (px >= 0 && py >= 0 && px < (int)cmW && py < (int)cmH) {
                                currentColorValues[py][px] = c;
                                currentColorMap.setPixel(px, py, c);
                            }
                        }
                    }
                }
            }
            texture.update(currentColorMap);

            fpsCounter++;
            mapWindow.draw(sprite);
            mapWindow.display();

            float elapsed = clock.getElapsedTime().asSeconds();
            if (elapsed >= 1.0f) {
                std::cout << "FPS: " << fpsCounter / elapsed << std::endl;
                fpsCounter = 0;
                clock.restart();
            }
        }
    }
}   
