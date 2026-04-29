#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

using namespace sf;
using namespace std;

const float G = 0.8f;
const float DT = 0.01f;
const float SOFTENING = 5.0f;

struct Trail {
    Vector2f position;
    float life;
};

class Body {
public:
    Vector2f position;
    Vector2f velocity;
    Vector2f acceleration;
    float mass;
    float radius;
    Color color;
    vector<Trail> trails;
    
    Body(float x, float y, float vx, float vy, float m, float r, Color c) {
        position = Vector2f(x, y);
        velocity = Vector2f(vx, vy);
        acceleration = Vector2f(0, 0);
        mass = m;
        radius = r;
        color = c;
    }
    
    void calculateForce(const Body& other, Vector2f& force) {
        Vector2f direction = other.position - position;
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y) + SOFTENING;
        float forceMagnitude = (G * mass * other.mass) / (distance * distance);
        force += direction / distance * forceMagnitude;
    }
    
    void update() {
        velocity += acceleration * DT;
        position += velocity * DT;
        acceleration = Vector2f(0, 0);
        
        // Add trail
        trails.push_back({position, 1.0f});
        if (trails.size() > 50) {
            trails.erase(trails.begin());
        }
        
        // Fade trails
        for (auto& trail : trails) {
            trail.life -= 0.02f;
        }
    }
    
    void draw(RenderWindow& window, View& view) {
        // Draw trails
        for (const auto& trail : trails) {
            if (trail.life > 0) {
                CircleShape trailCircle(radius * 0.5f);
                trailCircle.setPosition(Vector2f(trail.position.x - radius * 0.5f, trail.position.y - radius * 0.5f));
                trailCircle.setFillColor(Color(color.r, color.g, color.b, 50));
                window.draw(trailCircle);
            }
        }
        
        // Draw body
        CircleShape circle(radius);
        circle.setPosition(Vector2f(position.x - radius, position.y - radius));
        circle.setFillColor(color);
        window.draw(circle);
    }
};

int main() {
    RenderWindow window(VideoMode(Vector2u(1200, 800)), "N-Body Gravity Simulation");
    window.setFramerateLimit(60);
    
    View view(FloatRect(Vector2f(-400, -300), Vector2f(800, 600)));
    window.setView(view);
    
    vector<Body> bodies;
    
    // Sun
    bodies.push_back(Body(0, 0, 0, 0, 300.0f, 20.0f, Color::Yellow));
    
    // 4 colorful planets
    bodies.push_back(Body(180, 0, 0, 45, 1.2f, 8.0f, Color::Blue));
    bodies.push_back(Body(-150, 0, 0, -38, 1.0f, 7.0f, Color::Red));
    bodies.push_back(Body(0, 160, -42, 0, 1.1f, 7.5f, Color::Green));
    bodies.push_back(Body(220, 220, -35, 35, 2.0f, 12.0f, Color(255, 165, 0)));
    
    // Stars background
    vector<CircleShape> stars;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1200);
    for (int i = 0; i < 500; i++) {
        CircleShape star(1);
        star.setPosition(Vector2f(dis(gen), dis(gen)));
        star.setFillColor(Color(100, 100, 100));
        stars.push_back(star);
    }
    
    Clock clock;
    float lastTime = 0;
    bool dragging = false;
    Vector2f lastMousePos;
    
    while (window.isOpen()) {
        float currentTime = clock.getElapsedTime().asSeconds();
        float deltaTime = currentTime - lastTime;
        
        while (const optional<Event> event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
            
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->code == Keyboard::Key::Escape)
                    window.close();
            }
            
            // Mouse wheel zoom
            if (const auto* wheelScrolled = event->getIf<Event::MouseWheelScrolled>()) {
                if (wheelScrolled->wheel == Mouse::Wheel::Vertical) {
                    if (wheelScrolled->delta > 0)
                        view.zoom(0.9f);
                    else
                        view.zoom(1.1f);
                    window.setView(view);
                }
            }
            
            // Mouse drag pan
            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left) {
                    dragging = true;
                    lastMousePos = Vector2f(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
                }
            }
            
            if (const auto* mouseReleased = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseReleased->button == Mouse::Button::Left) {
                    dragging = false;
                }
            }
        }
        
        if (dragging) {
            Vector2f currentMousePos = Vector2f(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
            Vector2f delta = currentMousePos - lastMousePos;
            view.move(-delta * 0.5f);
            window.setView(view);
            lastMousePos = currentMousePos;
        }
        
        if (deltaTime >= 0.016f) {
            // Reset accelerations
            for (auto& body : bodies) {
                body.acceleration = Vector2f(0, 0);
            }
            
            // Calculate forces
            for (size_t i = 0; i < bodies.size(); i++) {
                Vector2f totalForce(0, 0);
                for (size_t j = 0; j < bodies.size(); j++) {
                    if (i != j) {
                        bodies[i].calculateForce(bodies[j], totalForce);
                    }
                }
                bodies[i].acceleration = totalForce / bodies[i].mass;
            }
            
            // Update positions
            for (auto& body : bodies) {
                body.update();
            }
            
            lastTime = currentTime;
        }
        
        window.clear(Color::Black);
        
        // Draw stars
        for (auto& star : stars) {
            window.draw(star);
        }
        
        // Draw bodies
        for (auto& body : bodies) {
            body.draw(window, view);
        }
        
        window.display();
    }
    
    return 0;
}