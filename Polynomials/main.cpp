#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <omp.h>

// hsv to rgb
sf::Color hsv(double hue_double, float sat, float val)
{
	hue_double *= 360.0;
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

// Definition for a complex number
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
// Polynomial function definition
ComplexNumber Polynomial(ComplexNumber input, std::vector<ComplexNumber>& coefficients) {
	ComplexNumber result(0.0, 0.0);

	for (int i = 0; i < coefficients.size(); i++) {
		result = result + coefficients.at(i) * (input^i);
	}

	return result;
}

// Pixel to complex number mapping
ComplexNumber pixelToComplex(int x, int y, int maxX, int maxY) {
	double real = x - maxX / 2.0;
	double imaginary = y - maxY / 2.0;
	real /= ((maxX / 2.0)) / 10.0;
	imaginary /= ((maxY / 2.0)) / 10.0;

	return{real, imaginary};
}

// Complex number to pixel mapping
sf::Vector2f complexToPixel(ComplexNumber complex, double maxReal, double maxImaginary, int screenDiameter) {
	double x = complex.real * ((screenDiameter / 2.0))/maxReal;
	double y = complex.imaginary * ((screenDiameter / 2.0))/maxImaginary;
	x += screenDiameter;
	y += screenDiameter;
	return sf::Vector2f((float)x, (float)y);
}

// Main
int main() {
	// Store coefficients ---------------------------------------------------------------------------------------
	std::vector<ComplexNumber> coefficients = {
		ComplexNumber(2.0, 0.0),
		ComplexNumber(4.0, 0.0),
		ComplexNumber(3.0, 0.0)
	};

	sf::RenderWindow window;

	int screenDiameter = 700;
	window.create(sf::VideoMode(screenDiameter*2, screenDiameter*2), "Polynomial visualision");
	window.setFramerateLimit(30);
	sf::Image image;
	image.create(screenDiameter, screenDiameter, sf::Color::Black);


	while (window.isOpen()){
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed){
				window.close();
			}
		}

		#pragma omp parallel for collapse(2)
		for (int x = 0; x < screenDiameter; x++) {
			for (int y = 0; y < screenDiameter; y++) {

				ComplexNumber input = pixelToComplex(x, y, screenDiameter, screenDiameter);

				ComplexNumber result = Polynomial(input, coefficients);
				// ComplexNumber result = input^0.5;

				double distance = hypot(result.real, result.imaginary);
				double angle = atan2(result.imaginary, result.real);

				double brightnessDecay = 10.0;
				double globalBrightness = 0.1;
				double closeToZeroValue;
				closeToZeroValue = 2.0/distance + globalBrightness + (brightnessDecay * globalBrightness)/(hypot(result.real, result.imaginary)+brightnessDecay);
				image.setPixel(x, y, hsv((angle+M_PI) * (0.5/M_PI), 1.0, closeToZeroValue));
			}
		}

		sf::Texture texture;
		texture.loadFromImage(image);
		sf::Sprite sprite(texture);
		sprite.setScale(2.0, 2.0);

		window.clear();
		window.draw(sprite);

		// Draw all coefficient dots
		for (int i = 0; i < coefficients.size(); i++) {
			sf::CircleShape dot(5);
			double hue = (double)i / (double)coefficients.size();
			dot.setFillColor(hsv(hue, 1.0, 1.0));
			dot.setPosition(complexToPixel(coefficients.at(i), 10.0, 10.0, screenDiameter));
			window.draw(dot);
		}

		window.display();
	}

	return 0;
}
