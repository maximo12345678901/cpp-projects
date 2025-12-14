#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

int main() {
    sf::RenderWindow window(
        sf::VideoMode(1000, 1000),
        "rng"
    );
    window.setPosition(sf::Vector2i(300, 300));
    window.setFramerateLimit(20);

    sf::Image image;
    image.create(200, 200);
    sf::Texture texture;
    sf::Sprite sprite;

    int seed;
    int factor;
    int a;
    float cap;
    std::cout << "\nbegin waarde: ";
    std::cin >> seed;
    std::cout << "\nfactor: ";
    std::cin >> factor;
    std::cout << "\noptel waarde: ";
    std::cin >> a;
    std::cout << "\nmodulo waarde: ";
    std::cin >> cap;

    

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) seed++;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) seed--;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) factor++;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) factor--;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) a++;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) a--;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) cap += 1.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) cap -= 1.0;


        float lastValue = std::fmod(seed * factor + a, cap);

        for (int x = 0; x < 200; x++) {
            for (int y = 0; y < 200; y++) {
                sf::Uint8 c = static_cast<sf::Uint8>((lastValue / cap) * 255.f);
                image.setPixel(x, y, sf::Color(c, c, c));
                lastValue = std::fmod(lastValue * factor + a, cap);
            }
        }

        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setScale(5.0, 5.0);
        window.clear();
        window.draw(sprite);
        window.display();

        std::cout << "\033[2J" << "\n";
        std::cout << "Seed: " << seed << "\n";
        std::cout << "Factor: " << factor << "\n";
        std::cout << "Optel waarde: " << a << "\n";
        std::cout << "Modulo waarde: " << cap << "\n";
    }

    return 0;
}