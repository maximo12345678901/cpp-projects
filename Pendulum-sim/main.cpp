#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <omp.h>
#include <iomanip>
#include <sstream>

float clamp(float v, float minVal, float maxVal) {
    return std::max(minVal, std::min(v, maxVal));
}

float wrap(float min, float max, float value) {
    float newValue = value;
    while (newValue < min) {
        newValue += max - min;
    }
    while (newValue > max) {
        newValue -= max - min;
    }
    return newValue;
}

// HSV to RGB conversion
sf::Color hsvToRgb(float h, float s, float v) {
    h = fmod(h, 360.f);
    if (h < 0) h += 360.f;

    float c = v * s;
    float x = c * (1 - std::fabs(fmod(h / 60.f, 2) - 1));
    float m = v - c;

    float rPrime, gPrime, bPrime;
    if      (h < 60)  { rPrime = c; gPrime = x; bPrime = 0; }
    else if (h < 120) { rPrime = x; gPrime = c; bPrime = 0; }
    else if (h < 180) { rPrime = 0; gPrime = c; bPrime = x; }
    else if (h < 240) { rPrime = 0; gPrime = x; bPrime = c; }
    else if (h < 300) { rPrime = x; gPrime = 0; bPrime = c; }
    else              { rPrime = c; gPrime = 0; bPrime = x; }

    sf::Uint8 r = static_cast<sf::Uint8>((rPrime + m) * 255);
    sf::Uint8 g = static_cast<sf::Uint8>((gPrime + m) * 255);
    sf::Uint8 b = static_cast<sf::Uint8>((bPrime + m) * 255);

    return sf::Color(r, g, b);
}

// Get stretched hue-square color
sf::Color getHueSquareColor(float x, float y, float width, float height) {
    float cx = width / 2.f;
    float cy = height / 2.f;

    // Normalize to -1..1
    float dx = (x - cx) / (width / 2.f);
    float dy = (y - cy) / (height / 2.f);

    // Square radius (stretches to edges)
    float radius = std::max(std::fabs(dx), std::fabs(dy));
    radius = clamp(radius, 0.f, 1.f);

    // Hue from angle
    float angle = std::atan2(dy, dx); // -π..π
    float hue = angle / (2 * M_PI);   // -0.5..0.5
    if (hue < 0) hue += 1.f;          // 0..1

    // Saturation from distance, value fixed at 1
    float saturation = radius;
    float value = 1.f;

    return hsvToRgb(hue * 360.f, saturation, value);
}

double angularAccel1(float l1, float l2,
                    double m1, double m2,
                    double th1, double th2,
                    double w1, double w2,
                    double g = 9.80665)
{
    double delta = th1 - th2;
    double denom = l1 * (3.0 - std::cos(2.0*delta));

    return (
        -g * 3 * std::sin(th1)
        - g * std::sin(th1 - 2.0*th2)
        - 2.0 * std::sin(delta) * m2 *
        (w2*w2 * l2 + w1*w1 * l1 * std::cos(delta))
    ) / denom;
}

double angularAccel2(float l1, float l2,
                    double m1, double m2,
                    double th1, double th2,
                    double w1, double w2,
                    double g = 9.80665)
{
    double delta = th1 - th2;
    double denom = l2 * (3.0- std::cos(2.0*delta));

    return (
        2.0 * std::sin(delta) *
        (w1*w1 * l1 * 2
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
                th1 + 0.5*dt*k3.dth1,
                w1 + 0.5*dt*k3.dw1,
                th2 + 0.5*dt*k3.dth2,
                w2 + 0.5*dt*k3.dw2);

            // Combine
            th1 += dt/6.0 * (k1.dth1 + 2*k2.dth1 + 2*k3.dth1 + k4.dth1);
            w1  += dt/6.0 * (k1.dw1  + 2*k2.dw1  + 2*k3.dw1  + k4.dw1);
            th2 += dt/6.0 * (k1.dth2 + 2*k2.dth2 + 2*k3.dth2 + k4.dth2);
            w2  += dt/6.0 * (k1.dw2  + 2*k2.dw2  + 2*k3.dw2  + k4.dw2);

            // Update positions
            firstDotPos = sf::Vector2f(origin.x + cos(th1 + (M_PI / 2)) * l1,
                                    origin.y + sin(th1 + (M_PI / 2)) * l1);
            secondDotPos = sf::Vector2f(firstDotPos.x + l2 * cos(th2 + (M_PI / 2)),
                                        firstDotPos.y + l2 * sin(th2 + (M_PI / 2)));
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
            l1 = 200;
            l2 = 200;
            m1 = 1;
            m2 = 1;

            origin = sf::Vector2f(500, 500);

            color = sf::Color::White;

            th1 = 0;
            th2 = 0;
            w1 = 0.0;
            w2 = 0.0;

            // convert the angle to screen coordinates
            firstDotPos = sf::Vector2f(origin.x + l1 * cos(th1), origin.y + l1 * sin(th1));
            secondDotPos = sf::Vector2f(firstDotPos.x + l2 * cos(th2), firstDotPos.y + l2 * sin(th2));

            dotRadius = 10;
        }
};

int main() {
    bool running = true;
    bool doGenerateMap;
    std::cout << "\nstate space? (y/N)\n";
    std::string input;
    std::cin >> input;
    if (input == "y" || input == "Y") {
        doGenerateMap = true;
    }
    else {
        doGenerateMap = false;
    }

    sf::RenderWindow simulationWindow;
    sf::RenderWindow graphWindow;
    sf::RenderWindow mapWindow;

    // Set up all of the windows
    if (!doGenerateMap) {
        simulationWindow.create(sf::VideoMode(1000, 1000), "Pendulum simulation");
        graphWindow.create(sf::VideoMode(1000, 1000), "Pendulum angle graph");

        simulationWindow.setPosition(sf::Vector2i(100, 100));
        graphWindow.setPosition(sf::Vector2i(1200, 100));

        simulationWindow.setFramerateLimit(120);
    }

    // Set up the color map
    sf::Image originalColorMap;
    if (!originalColorMap.loadFromFile("better-colormap.png")) {
        return 1;
    }
    unsigned int originalColorMapWidth  = originalColorMap.getSize().x;
    unsigned int originalColorMapHeight = originalColorMap.getSize().y;

    int resolution = 200;
    sf::Image currentColorMap;
    currentColorMap.create(originalColorMapWidth, originalColorMapHeight);

    sf::Texture texture;
    texture.loadFromImage(currentColorMap);

    sf::Sprite sprite(texture);

    std::vector<std::vector<sf::Color>> originalColorValues(originalColorMapHeight, std::vector<sf::Color>(originalColorMapWidth));
    std::vector<std::vector<sf::Color>> currentColorValues(originalColorMapHeight, std::vector<sf::Color>(originalColorMapWidth));

    std::vector<std::vector<Pendulum>> pendulums(resolution, std::vector<Pendulum>(resolution));

    if (doGenerateMap) {
        mapWindow.create(sf::VideoMode(1000, 1000), "Map");

        mapWindow.setPosition(sf::Vector2i(100, 200));

        for (unsigned int y = 0; y < originalColorMapHeight; ++y) {
            for (unsigned int x = 0; x < originalColorMapWidth; ++x) {
                originalColorValues[y][x] = originalColorMap.getPixel(x, y);
                currentColorValues[y][x] = originalColorMap.getPixel(x, y);
            }
        }


        // Set up all pendulums
        for (int i = 0; i < resolution; ++i) {
            for (int j = 0; j < resolution; ++j) {
                Pendulum pendulum;

                float normX = ((float)i / (resolution - 1)) * 2 * M_PI - M_PI;
                float normY = ((float)j / (resolution - 1)) * 2 * M_PI - M_PI;

                pendulum.th1 = normX;  
                pendulum.th2 = normY;
                

                pendulums[i][j] = pendulum;
            }
        }
    }



    sf::Clock clock;
    int frameCount = 0;
    int fpsCounter = 0;


    // Declare some variables relating to the pendula
    double G = 10000;
    double dt = 0.002;

    Pendulum pendulum;
    
    std::vector<sf::CircleShape> graphTrail;

    if (!doGenerateMap) {


        std::cout << "\nth1: ";
        double firstAngle;
        std::cin >> firstAngle;
        std::cout << "\nth2: ";
        double secondAngle;
        std::cin >> secondAngle;

        // pendulum.th1 = ((float) 832 / (resolution - 1)) * 2 * M_PI - M_PI;
        // pendulum.th2 = ((float) 720 / (resolution - 1)) * 2 * M_PI - M_PI;
        pendulum.th1 = firstAngle;
        pendulum.th2 = secondAngle;
    
    // loop
    while (running) {
        sf::Event event;
        if (!doGenerateMap) {
            while (simulationWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    running = false;
                }
            }
            while (graphWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    running = false;
                }
            }
        }
        if (doGenerateMap) {
            while (mapWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    running = false;
                }
            }
        }
        if (!doGenerateMap) {
            // Update simulation window
            simulationWindow.clear(sf::Color(10, 10, 10));


            // // Update graph window
            float scale = 200; // radians per pixel
            int pointSize = 2;
            
            graphWindow.clear(sf::Color(0, 0, 0));

            sf::CircleShape graphPoint(pointSize);
            graphPoint.setFillColor(sf::Color(255, 255, 255));
            graphPoint.setOrigin(pointSize / 2, pointSize / 2);

            graphPoint.setPosition(pendulum.th1 * scale + 500, pendulum.th2 * scale + 500);

            graphTrail.push_back(graphPoint);

            // Limit the length of the graph
            if (graphTrail.size() > 2500) {
                graphTrail.erase(graphTrail.begin());
            }

            // Center all points so that the most recent one is in the middle
            for (sf::CircleShape point : graphTrail) {
                // sf::Vector2f newPosition;
                // newPosition.x = point.getPosition().x - graphTrail.back().getPosition().x + graphWindow.getSize().x / 2;
                // newPosition.y = point.getPosition().y - graphTrail.back().getPosition().y + graphWindow.getSize().y / 2;
                // point.setPosition(newPosition);
                graphWindow.draw(point);
            }
            graphPoint.setRadius(pointSize * 4);
            graphPoint.setOrigin(pointSize * 4 / 2, pointSize * 4 / 2);
            graphWindow.draw(graphPoint);
            pendulum.UpdatePendulumRK4(G, dt);
            pendulum.DrawPendulum(simulationWindow);

        }

        if (doGenerateMap) {
            // Update map window
            #pragma omp parallel for collapse(2)
            for (int x = 0; x < resolution; ++x) {
                for (int y = 0; y < resolution; ++y) {

                    pendulums[x][y].UpdatePendulumRK4(G, dt);

                    float th1 = wrap(-M_PI, M_PI, pendulums[x][y].th1);
                    float th2 = wrap(-M_PI, M_PI, pendulums[x][y].th2);

                    int srcX = (int)((th1 + M_PI) / (2 * M_PI) * (originalColorMapWidth  - 1));
                    int srcY = (int)((th2 + M_PI) / (2 * M_PI) * (originalColorMapHeight - 1));

                    int dstX = x * originalColorMapWidth  / resolution;
                    int dstY = y * originalColorMapHeight / resolution;

                    sf::Color c = originalColorValues[srcY][srcX]; 
                    int fill = std::ceil(1000 / resolution);
                    for (int i = 0; i < fill; ++i) {
                        for (int j = 0; j < fill; ++j) {
                            currentColorValues[dstY + i][dstX + j] = c;
                            currentColorMap.setPixel(dstX + i, dstY + j, c);
                        }
                    }
                }
            }
            texture.update(currentColorMap);

            // std::ostringstream filenameStream;
            // filenameStream << "frames/frame_" << std::setw(4) << std::setfill('0') << frameCount << ".png";
            // std::string filename = filenameStream.str();

            // currentColorMap.saveToFile(filename);
        }

        frameCount++;
        fpsCounter++;
        
        if (!doGenerateMap) {
            simulationWindow.display();
            graphWindow.display();
        }

        if (doGenerateMap) {
            mapWindow.draw(sprite);
            mapWindow.display();
            
            // FPS calculation
            float elapsed = clock.getElapsedTime().asSeconds();
            if (elapsed >= 1.0f) {
                float fps = fpsCounter / elapsed; 
                std::cout << "FPS: " << fps << std::endl;

                fpsCounter = 0;
                clock.restart();
            }
        }


    }
}
}
