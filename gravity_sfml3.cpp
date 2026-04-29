#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace sf;
using namespace std;

const float G = 0.5f;
const float DT = 0.01f;
const float SOFTENING = 5.0f;

class Body {
public:
    Vector2f position;
    Vector2f velocity;
    Vector2f acceleration;
    float mass;
    float radius;
    Color color;
    
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
    }
    
    void draw(RenderWindow& window) {
        CircleShape circle(radius);
        circle.setPosition(Vector2f(position.x - radius, position.y - radius));
        circle.setFillColor(color);
        window.draw(circle);
    }
};

int main() {
    RenderWindow window(VideoMode(Vector2u(800, 600)), "N-Body Gravity Simulation");
    window.setFramerateLimit(60);
    
    View view(FloatRect(Vector2f(0, 0), Vector2f(800, 600)));
    view.setCenter(Vector2f(0, 0));
    window.setView(view);
    
    vector<Body> bodies;
    
    bodies.push_back(Body(0, 0, 0, 0, 200.0f, 15.0f, Color::Yellow));
    bodies.push_back(Body(150, 0, 0, 35, 1.0f, 6.0f, Color::Blue));
    bodies.push_back(Body(-120, 0, 0, -30, 0.8f, 5.0f, Color::Red));
    bodies.push_back(Body(0, 130, -32, 0, 0.9f, 5.5f, Color::Green));
    bodies.push_back(Body(200, 200, -28, 28, 2.5f, 10.0f, Color(255, 165, 0)));
    
    Clock clock;
    float lastTime = 0;
    
    while (window.isOpen()) {
        float currentTime = clock.getElapsedTime().asSeconds();
        float deltaTime = currentTime - lastTime;
        
        while (const optional<Event> event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
        }
        
        if (deltaTime >= 0.016f) {
            for (auto& body : bodies) {
                body.acceleration = Vector2f(0, 0);
            }
            
            for (size_t i = 0; i < bodies.size(); i++) {
                Vector2f totalForce(0, 0);
                for (size_t j = 0; j < bodies.size(); j++) {
                    if (i != j) {
                        bodies[i].calculateForce(bodies[j], totalForce);
                    }
                }
                bodies[i].acceleration = totalForce / bodies[i].mass;
            }
            
            for (auto& body : bodies) {
                body.update();
            }
            
            lastTime = currentTime;
        }
        
        window.clear(Color::Black);
        
        for (auto& body : bodies) {
            body.draw(window);
        }
        
        window.display();
    }
    
    return 0;
}