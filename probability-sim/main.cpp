#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <vector>
#include <cmath>

sf::Vector2f operator*(const sf::Vector2f& v, float scalar) {
    return sf::Vector2f(v.x * scalar, v.y * scalar);
}

double transformationFunction(double x, int scale) {
    return (std::sin(x * 2 * M_PI)*0.65+ x + (-0.01)/(x-1))*scale;
    // return (std::abs(sin(M_PI*x)))*scale;
    // return (std::pow(4, x) - 1) * scale;
    // return 3*std::tan(x/5)*scale;
}

sf::Vector2f normalizedSlopeVector(double x, double h) {
    double slope = (transformationFunction(x+h, 1) - transformationFunction(x, 1))/h;
    double magnitude = std::sqrt(slope*slope + 1);

    sf::Vector2f vector = sf::Vector2f(1/magnitude, slope/magnitude);
    return vector;
}
int main() {
    sf::RenderWindow window;

    int width = 800;
    int height= 800;
    window.create(sf::VideoMode(width, height), "probabilty");
    window.setFramerateLimit(120);


    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<> dist(0.0, 1.0); // Range [0, 1)

    std::vector<sf::CircleShape> circleShapes; // Store all dots

    int unit2pixel = 400;
    int slices = 1000;
    float pointDensity = 50.0f;

    float sliceWidth =  1.0f / slices;

    std::vector<sf::Vector2f> graphPoints;
    std::vector<sf::Vector2f> PDFpoints;

    graphPoints.push_back(sf::Vector2f(0, 0));

    int pointsGenerated = 1;

    while (true) {
        sf::Vector2f previousPoint = graphPoints.at(pointsGenerated - 1);

        if (previousPoint.y > 3.0 || previousPoint.x > 1.0) {
            break;
        }
        sf::Vector2f direction = normalizedSlopeVector(previousPoint.x, 0.0001)* sliceWidth / pointDensity;
        graphPoints.push_back(previousPoint + direction); 
        pointsGenerated++;
        // std::cout << direction.x << "     " << direction.y << "\n";
        
        std::cout << previousPoint.x << "\n";
        std::cout << pointsGenerated << "\n";

    }

    float pointValue = 1.0f / pointsGenerated;
    for (int i = 0; i < slices * 2; i++) {
        float totalValue = 0.0;
        for (sf::Vector2f point : graphPoints) {
            float lowerBound = i * sliceWidth;
            float upperBound = (i + 1) * sliceWidth;

            if (point.y >= lowerBound && point.y < upperBound) {
                totalValue += pointValue;
            }

        }
        PDFpoints.push_back(sf::Vector2f(i * sliceWidth, totalValue));
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        // Calculate x position

        double x = transformationFunction(dist(gen), unit2pixel);


        // Calculate y offset

        double randYoffset = dist(gen);
        double maxOffset = 50;
        randYoffset *= maxOffset * 2;
        randYoffset -= maxOffset;

        double y = randYoffset;

        // Draw

        sf::CircleShape dot(1);
        dot.setPosition(sf::Vector2f(x, y + height / 2));
        circleShapes.push_back(dot);

        window.clear();

        // Draw the random points
        for (sf::CircleShape& circle : circleShapes) {
            circle.setFillColor(sf::Color(150, 0, 255));
            window.draw(circle);
        }

        // Draw the transformation function
        for (sf::Vector2f point : graphPoints) {
            sf::CircleShape graphicalPoint(1);
            graphicalPoint.setPosition(sf::Vector2f(point.y * unit2pixel, point.x * -unit2pixel + height));
            graphicalPoint.setFillColor(sf::Color(0, 255, 0));
            window.draw(graphicalPoint);
        }

        // Draw the PDF as a continuous line
        sf::VertexArray pdfLine(sf::LineStrip, PDFpoints.size());

        for (std::size_t i = 0; i < PDFpoints.size(); ++i) {
            pdfLine[i].position = sf::Vector2f(
                PDFpoints[i].x * unit2pixel,
                PDFpoints[i].y * -unit2pixel * slices / 3 + height
            );
            pdfLine[i].color = sf::Color(0, 255, 255);
        }

        window.draw(pdfLine);

        window.display();
    }

    return 0;
}