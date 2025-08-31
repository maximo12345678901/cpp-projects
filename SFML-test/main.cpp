#include <SFML/Graphics.hpp>
#include <vector>

struct Entity {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::RectangleShape shape;

    Entity(sf::Vector2f startPos, sf::Vector2f startVel, sf::Color color) {
        position = startPos;
        velocity = startVel;

        shape.setSize({100.f, 100.f});
        shape.setFillColor(color);
        shape.setPosition(position);
    }

    void updateShape() {
        shape.setPosition(position);
    }
};

const sf::Vector2f screenSize(1500.f, 1000.f);
const float gravity = -1.0f;
const float acceleration = 1.2f;
const float bounciness = 0.5f;

void handleInput(Entity& entity);
void applyPhysics(Entity& entity);

int main()
{
    sf::RenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML Multiple Squares");
    window.setFramerateLimit(60);

    // Create multiple squares
    std::vector<Entity> squares;
    squares.emplace_back(sf::Vector2f(200, 300), sf::Vector2f(3, 5), sf::Color::Red);
    squares.emplace_back(sf::Vector2f(800, 600), sf::Vector2f(-4, -2), sf::Color::Blue);
    squares.emplace_back(sf::Vector2f(400, 100), sf::Vector2f(2, 3), sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed ||
               (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            {
                window.close();
            }
        }

        // Apply input and physics to each square
        for (auto& square : squares) {
            handleInput(square);   // Control first square with keyboard
            applyPhysics(square);
            square.updateShape();
        }

        // Render
        window.clear();
        for (const auto& square : squares)
            window.draw(square.shape);
        window.display();
    }

    return 0;
}

void handleInput(Entity& entity)
{
    // Only apply input to the first square (extendable)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        entity.velocity.x -= acceleration;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        entity.velocity.x += acceleration;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        entity.velocity.y -= acceleration;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        entity.velocity.y += acceleration;
    }
}

void applyPhysics(Entity& entity)
{
    // Apply movement
    entity.position += entity.velocity;

    // Apply screen collision and bounce
    if (entity.position.x < 0) {
        entity.velocity.x = std::abs(entity.velocity.x) * bounciness;
        entity.position.x = 0;
    }
    if (entity.position.x > screenSize.x - 100) {
        entity.velocity.x = -std::abs(entity.velocity.x) * bounciness;
        entity.position.x = screenSize.x - 100;
    }
    if (entity.position.y < 0) {
        entity.velocity.y = std::abs(entity.velocity.y) * bounciness;
        entity.position.y = 0;
    }
    if (entity.position.y > screenSize.y - 100) {
        entity.velocity.y = -std::abs(entity.velocity.y) * bounciness;
        entity.position.y = screenSize.y - 100;
    }

    // Apply gravity
    entity.velocity.y -= gravity;
}
