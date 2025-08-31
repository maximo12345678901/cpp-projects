#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

class GravityObject {
    public:
        // Members
        sf::Vector2f position;
        sf::Vector2f velocity;
        int radius;
        float mass;
        bool isGrabbed;

        // Constructor
        GravityObject() {
            position = sf::Vector2f(0, 0);
            velocity = sf::Vector2f(0, 0);
            radius = 50;
            mass = 1;
            isGrabbed = false;
        }
};

float magnitude(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f gravitationalAcceleration(GravityObject obj1, GravityObject obj2, float G) {
    sf::Vector2f direction = obj2.position - obj1.position;
    float distance = magnitude(direction);


    if (distance < 3) {
        return sf::Vector2f(0, 0);
    }

    sf::Vector2f normalizedDirection(direction.x / distance, direction.y / distance);
    float scalar = G * obj2.mass / (distance * distance);
    return sf::Vector2f(normalizedDirection.x * scalar, normalizedDirection.y * scalar);
}

int main() {
    // Initialize window
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Gravity Simulation");
    window.setFramerateLimit(60);

    // Initialize gravity object(s)
    std::vector<GravityObject> gravityObjects;
    sf::Vector2i mousePosition;

    int scrollSpeed = 30;

    // Loop
    while (window.isOpen()) {
        sf::Event event;
        mousePosition = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::E) {
                    
                    GravityObject newObject;
                    newObject.position.x = mousePosition.x;
                    newObject.position.y = mousePosition.y;

                    gravityObjects.push_back(newObject);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f clickPosition(event.mouseButton.x, event.mouseButton.y);
                    for (GravityObject &obj : gravityObjects) {
                        if (magnitude(clickPosition - obj.position) < obj.radius) {
                            obj.isGrabbed = true;
                        } 
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    for (GravityObject &obj : gravityObjects) {
                        obj.isGrabbed = false;
                    }
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            
            for (GravityObject &obj : gravityObjects) {
                obj.position.x += scrollSpeed;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            
            for (GravityObject &obj : gravityObjects) {
                obj.position.x -= scrollSpeed;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            
            for (GravityObject &obj : gravityObjects) {
                obj.position.y += scrollSpeed;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            
            for (GravityObject &obj : gravityObjects) {
                obj.position.y -= scrollSpeed;
            }
        }


        window.clear(sf::Color(10, 10, 10));
        // Update and draw all gravity objects
        for (GravityObject &gravityObject : gravityObjects) {
            if (gravityObject.isGrabbed) {
                gravityObject.velocity.x = (mousePosition.x - gravityObject.position.x) * 0.1f;
                gravityObject.velocity.y = (mousePosition.y - gravityObject.position.y) * 0.1f;
            }
            else {
                for (GravityObject &otherObject : gravityObjects) {
                    if (&gravityObject == &otherObject) {
                        continue;
                    }
                    sf::Vector2f acceleration = gravitationalAcceleration(gravityObject, otherObject, 10000);
                    gravityObject.velocity.x += acceleration.x;
                    gravityObject.velocity.y += acceleration.y;
                }
            }
            gravityObject.position.x += gravityObject.velocity.x;
            gravityObject.position.y += gravityObject.velocity.y;

            sf::CircleShape circleShape(gravityObject.radius);
            circleShape.setFillColor(sf::Color(0, 120, 255));
            circleShape.setOrigin(gravityObject.radius, gravityObject.radius);
            circleShape.setPosition(gravityObject.position);
            window.draw(circleShape);
        }
        window.display();
    }


    return 0;
}
