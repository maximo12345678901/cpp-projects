#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

struct ComplexNumber {
	double real;
	double imaginary;

	ComplexNumber operator+(const ComplexNumber& other) const{
		return {real + other.real, imaginary + other.imaginary};
	}

	ComplexNumber operator-(const ComplexNumber& other) const{
		return {real - other.real, imaginary - other.imaginary};
	}

	ComplexNumber operator*(const ComplexNumber& other) const{
		return {real * other.real - imaginary * other.imaginary,
		real * other.imaginary + other.real * imaginary};
	}

	ComplexNumber operator^(const double exponent) const{
		double r = hypot(real, imaginary);
		double th = atan2(imaginary, real);
		return {pow(r, exponent) * cos(exponent*th), pow(r, exponent)*sin(exponent*th)};
	}

	ComplexNumber(double r, double i) {
		real = r;
		imaginary = i;
	}
};

ComplexNumber Polynomial(ComplexNumber x, ComplexNumber a, ComplexNumber b, ComplexNumber c, ComplexNumber d, ComplexNumber e) {
	ComplexNumber firstTerm = a*(x^4.0);
	ComplexNumber secondTerm = b*(x^3.0);
	ComplexNumber thirdTerm = c*(x^2.0);
	ComplexNumber fourthTerm = d*x;
	ComplexNumber fifthTerm = e;
	return firstTerm + secondTerm + thirdTerm + fourthTerm + fifthTerm;
}

ComplexNumber pixelToComplex(int x, int y, int maxX, int maxY) {
	double real = x - maxX / 2.0;
	double imaginary = y - maxY / 2.0;
	real /= ((maxX / 2.0)) / 10.0;
	imaginary /= ((maxY / 2.0)) / 10.0;

	return{real, imaginary};
}

sf::Vector2f complexToPixel(ComplexNumber complex, double maxReal, double maxImaginary, int screenDiameter) {
	double x = complex.real * ((screenDiameter / 2.0))/maxReal;
	double y = complex.imaginary * ((screenDiameter / 2.0))/maxImaginary;
	x += screenDiameter;
	y += screenDiameter;
	return sf::Vector2f((float)x, (float)y);
}

int main() {
	sf::RenderWindow window;

	int screenDiameter = 500;
	window.create(sf::VideoMode(screenDiameter*2, screenDiameter*2), "Polynomial visualision");
	window.setFramerateLimit(60);
	sf::Image image;
	image.create(screenDiameter, screenDiameter, sf::Color::Black);

	ComplexNumber a(0, 0);
	ComplexNumber b(0, 0);
	ComplexNumber c(0, 0);
	ComplexNumber d(0, 0);
	ComplexNumber e(0, 0);

	while (window.isOpen()){
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed){
				window.close();
			}
		}
		

		double changeSpeed = 0.1f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			a.real -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			a.real += changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			a.imaginary -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			a.imaginary += changeSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			b.real -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			b.real += changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			b.imaginary -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			b.imaginary += changeSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
			c.real -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {
			c.real += changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
			c.imaginary -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
			c.imaginary += changeSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) {
			d.real -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) {
			d.real += changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
			d.imaginary -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
			d.imaginary += changeSpeed;
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {
			e.real -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
			e.real += changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
			e.imaginary -= changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Semicolon)) {
			e.imaginary += changeSpeed;
		}


		for (int x = 0; x < screenDiameter; x++) {
			for (int y = 0; y < screenDiameter; y++) {

				ComplexNumber input = pixelToComplex(x, y, screenDiameter, screenDiameter);

				ComplexNumber result = Polynomial(input, a, b, c, d, e);
				// ComplexNumber result = input^0.5;

				double distance = hypot(result.real, result.imaginary);

				double brightnessDecay = 1.0;
				double closeToZeroValue = (255.0*brightnessDecay)/(hypot(result.real, result.imaginary)+brightnessDecay);
				image.setPixel(x, y, sf::Color(static_cast<sf::Uint8>(closeToZeroValue), static_cast<sf::Uint8>(closeToZeroValue), static_cast<sf::Uint8>(closeToZeroValue)));
			}
		}

		sf::Texture texture;
		texture.loadFromImage(image);
		sf::Sprite sprite(texture);
		sprite.setScale(2.0, 2.0);

		window.clear();
		window.draw(sprite);
		sf::CircleShape dotA(5);
		sf::CircleShape dotB(5);
		sf::CircleShape dotC(5);
		sf::CircleShape dotD(5);
		sf::CircleShape dotE(5);

		dotA.setFillColor(sf::Color::Red);
		dotB.setFillColor(sf::Color::Yellow);
		dotC.setFillColor(sf::Color::Green);
		dotD.setFillColor(sf::Color::Blue);
		dotE.setFillColor(sf::Color::Magenta);

		dotA.setPosition(complexToPixel(a, 10.0, 10.0, screenDiameter));
		dotB.setPosition(complexToPixel(b, 10.0, 10.0, screenDiameter));
		dotC.setPosition(complexToPixel(c, 10.0, 10.0, screenDiameter));
		dotD.setPosition(complexToPixel(d, 10.0, 10.0, screenDiameter));
		dotE.setPosition(complexToPixel(e, 10.0, 10.0, screenDiameter));

		window.draw(dotA);
		window.draw(dotB);
		window.draw(dotC);
		window.draw(dotD);
		window.draw(dotE);

		window.display();
	}

	return 0;
}
