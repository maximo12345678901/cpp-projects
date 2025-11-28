#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <omp.h>

sf::Color hsv(double hue_double, float sat, float val)
{
	double factor = 180.0 / M_PI;
	hue_double *= factor;
	hue_double;
	int hue = (int) hue_double;
	hue %= 360;
	while(hue<0) hue += 360;

	if(sat<0.f) sat = 0.f;
	if(sat>1.f) sat = 1.f;

	if(val<0.f) val = 0.f;
	if(val>1.f) val = 1.f;

	int h = hue/60;
	float f = float(hue)/60-h;
	float p = val*(1.f-sat);
	float q = val*(1.f-sat*f);
	float t = val*(1.f-sat*(1-f));

	switch(h)
	{
		default:
		case 0:
		case 6: return sf::Color(val*255, t*255, p*255);
		case 1: return sf::Color(q*255, val*255, p*255);
		case 2: return sf::Color(p*255, val*255, t*255);
		case 3: return sf::Color(p*255, q*255, val*255);
		case 4: return sf::Color(t*255, p*255, val*255);
		case 5: return sf::Color(val*255, p*255, q*255);
	}
}


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
	window.setFramerateLimit(30);
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

		ComplexNumber arrowDirection(0.0, 0.0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			arrowDirection.real = -changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			arrowDirection.real = changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			arrowDirection.imaginary = -changeSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			arrowDirection.imaginary = changeSpeed;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			a = a + arrowDirection;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
			b = b + arrowDirection;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
			c = c + arrowDirection;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			d = d + arrowDirection;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			e = e + arrowDirection;
		}

		#pragma omp parallel for collapse(2)
		for (int x = 0; x < screenDiameter; x++) {
			for (int y = 0; y < screenDiameter; y++) {

				ComplexNumber input = pixelToComplex(x, y, screenDiameter, screenDiameter);

				ComplexNumber result = Polynomial(input, a, b, c, d, e);
				// ComplexNumber result = input^0.5;

				double distance = hypot(result.real, result.imaginary);
				double angle = atan2(result.imaginary, result.real);

				double brightnessDecay = 10.0;
				double globalBrightness = 0.1;
				double closeToZeroValue = 1.0/distance + globalBrightness + (brightnessDecay * globalBrightness)/(hypot(result.real, result.imaginary)+brightnessDecay);
				image.setPixel(x, y, hsv(angle, 1.0, closeToZeroValue));
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
