// main.cpp
// Compile: g++ main.cpp -o bh_sim -lsfml-graphics -lsfml-window -lsfml-system -std=c++17
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>

struct Ray {
    sf::VertexArray path; // line strip
    sf::Vector2f pos;
    sf::Vector2f dir; // unit vector (velocity direction)
    bool alive;
};

int main() {
    const unsigned W = 1200, H = 800;
    sf::RenderWindow window(sf::VideoMode(W, H), "Black Hole Ray Bending - Approximation");
    window.setFramerateLimit(120);

    // Simulation parameters (tune these for look)
    const float c = 1.0f;            // speed-of-light in simulation units
    float K = 4000.0f;               // effective strength ~ GM (tune visually)
    float schwarzschild_r = 40.0f;   // "horizon" radius in screen units
    float dt = 0.02f;                // time step
    int raysCount = 600;
    int maxSteps = 2000;

    // Black hole center (screen coords)
    sf::Vector2f center(W*0.5f, H*0.5f);

    // Visual: draw BH as filled circle (horizon)
    sf::CircleShape bhHorizon(schwarzschild_r);
    bhHorizon.setOrigin(schwarzschild_r, schwarzschild_r);
    bhHorizon.setPosition(center);
    bhHorizon.setFillColor(sf::Color::Black);

    // Initialize rays: spawn from left edge with slight vertical spread
    std::vector<Ray> rays;
    rays.reserve(raysCount);
    std::mt19937 rng(12345);
    std::uniform_real_distribution<float> distY(0.1f*H, 0.9f*H);
    std::uniform_real_distribution<float> jitter(-0.003f, 0.003f);

    for (int i=0;i<raysCount;i++){
        Ray r;
        r.pos = sf::Vector2f(10.0f, distY(rng)); // left margin
        // initial direction pointing right
        float dy = ( (float)i / raysCount - 0.5f ) * 0.002f + jitter(rng);
        r.dir = sf::Vector2f(1.0f, dy);
        // normalize dir
        float len = std::sqrt(r.dir.x*r.dir.x + r.dir.y*r.dir.y);
        r.dir /= len;
        r.alive = true;
        r.path = sf::VertexArray(sf::LineStrip);
        r.path.append(sf::Vertex(r.pos, sf::Color::White));
        rays.push_back(std::move(r));
    }

    // Precompute capture circle visual
    sf::CircleShape captureOuter(schwarzschild_r*1.8f);
    captureOuter.setOrigin(schwarzschild_r*1.8f, schwarzschild_r*1.8f);
    captureOuter.setPosition(center);
    captureOuter.setFillColor(sf::Color(0,0,0,0));
    captureOuter.setOutlineColor(sf::Color(80,80,80));
    captureOuter.setOutlineThickness(1.0f);

    bool paused = false;
    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Space) paused = !paused;
                if (ev.key.code == sf::Keyboard::Up) K *= 1.2f;
                if (ev.key.code == sf::Keyboard::Down) K /= 1.2f;
                if (ev.key.code == sf::Keyboard::Right) schwarzschild_r += 4.0f;
                if (ev.key.code == sf::Keyboard::Left) schwarzschild_r = std::max(4.0f, schwarzschild_r - 4.0f);
            }
        }

        if (!paused) {
            // advance rays
            for (auto &r : rays) {
                if (!r.alive) continue;
                for (int step=0; step<1; ++step) {
                    // compute vector from ray to BH center
                    sf::Vector2f rel = center - r.pos;
                    float dist2 = rel.x*rel.x + rel.y*rel.y;
                    float dist = std::sqrt(dist2);
                    if (dist <= schwarzschild_r) {
                        r.alive = false;
                        break;
                    }
                    // acceleration magnitude: K / r^2, direction towards center
                    // vector form: a = K * rel / r^3
                    float invr3 = 1.0f / (dist2 * dist + 1e-6f);
                    sf::Vector2f a = rel * (K * invr3);

                    // simple RK4-ish step for direction (improves stability)
                    // integrate direction: v' = v + a*dt ; then renormalize to speed c
                    sf::Vector2f v1 = r.dir;
                    sf::Vector2f k1 = a * dt;

                    sf::Vector2f v2 = r.dir + k1*0.5f;
                    sf::Vector2f k2 = a * dt; // acceleration approx constant for short dt

                    sf::Vector2f v_new = r.dir + k2;
                    // renormalize direction to keep speed constant
                    float vlen = std::sqrt(v_new.x*v_new.x + v_new.y*v_new.y);
                    if (vlen > 1e-8f) v_new /= vlen;

                    // position update
                    r.pos += v_new * c * dt;
                    r.dir = v_new;

                    // push to path
                    r.path.append(sf::Vertex(r.pos, sf::Color::White));
                    // limit path length to avoid memory explosion
                    if (r.path.getVertexCount() > 1200) {
                        // remove oldest by shifting (cheap for small counts)
                        for (size_t i=1;i<r.path.getVertexCount();++i) r.path[i-1] = r.path[i];
                        r.path.resize(r.path.getVertexCount()-1);
                    }

                    // kill if off-screen
                    if (r.pos.x < -50 || r.pos.x > W+50 || r.pos.y < -50 || r.pos.y > H+50) {
                        r.alive = false;
                        break;
                    }
                }
            }
        }

        // draw
        window.clear(sf::Color(10,10,20));
        // optional faint background grid (skip for speed)
        window.draw(captureOuter);
        window.draw(bhHorizon);
        // draw rays
        for (auto &r : rays) {
            // color by alive/dead
            sf::Color col = r.alive ? sf::Color::White : sf::Color(120,120,120);
            for (size_t i=0;i<r.path.getVertexCount();++i) r.path[i].color = col;
            window.draw(r.path);
        }

        // Overlay text
        static sf::Font font;
        static bool loaded = false;
        if (!loaded) {
            if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                // fallback: don't show text
                loaded = true;
            } else loaded = true;
        }
        if (loaded) {
            sf::Text info;
            info.setFont(font);
            info.setCharacterSize(14);
            info.setFillColor(sf::Color::Green);
            info.setPosition(10, 10);
            char buf[256];
            std::snprintf(buf, sizeof(buf), "K=%.1f   r_s=%.1f   dt=%.3f   rays=%d   (Arrows: change, Space: pause)", K, schwarzschild_r, dt, raysCount);
            info.setString(buf);
            window.draw(info);
        }

        window.display();
    }

    return 0;
}
