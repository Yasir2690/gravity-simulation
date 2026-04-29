#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;
using namespace sf;

// Constants
const double G = 6.67430e-11;
const double DT = 100.0;
const double SOFTENING = 10.0;
const double SCALE = 2.0;  // Zoom factor
const double OFFSET_X = 400;  // Center screen X
const double OFFSET_Y = 300;  // Center screen Y

class Body {
public:
    double x, y, vx, vy, mass;
    string name;
    Color color;
    double radius;
    
    Body(string n, double px, double py, double vx0, double vy0, double m, Color c, double r) {
        name = n;
        x = px;
        y = py;
        vx = vx0;
        vy = vy0;
        mass = m;
        color = c;
        radius = r;
    }
    
    void computeForce(const Body& other, double& fx, double& fy) {
        double dx = other.x - x;
        double dy = other.y - y;
        double dist = sqrt(dx*dx + dy*dy) + SOFTENING;
        double force = (G * mass * other.mass) / (dist * dist);
        fx += force * dx / dist;
        fy += force * dy / dist;
    }
    
    void update(double fx, double fy) {
        vx += fx / mass * DT;
        vy += fy / mass * DT;
        x += vx * DT;
        y += vy * DT;
    }
    
    void draw(RenderWindow& window) {
        CircleShape circle(radius);
        circle.setFillColor(color);
        circle.setPosition(x * SCALE + OFFSET_X - radius, y * SCALE + OFFSET_Y - radius);
        window.draw(circle);
    }
};

int main() {
    cout << "========================================" << endl;
    cout << "  N-BODY GRAVITY SIMULATION - GRAPHICS" << endl;
    cout << "========================================" << endl;
    
    // Create window
    RenderWindow window(VideoMode(800, 600), "Gravity Simulation");
    window.setFramerateLimit(60);
    
    // Create bodies
    vector<Body> bodies;
    
    // Sun (yellow, large)
    bodies.push_back(Body("Sun", 0.0, 0.0, 0.0, 0.0, 2000.0, Color::Yellow, 15.0));
    
    // Earth (blue)
    bodies.push_back(Body("Earth", 100.0, 0.0, 0.0, 50.0, 1.0, Color::Blue, 5.0));
    
    // Mars (red)
    bodies.push_back(Body("Mars", -80.0, 0.0, 0.0, -40.0, 0.8, Color::Red, 4.0));
    
    // Venus (green)
    bodies.push_back(Body("Venus", 0.0, 90.0, -45.0, 0.0, 0.9, Color::Green, 4.5));
    
    // Jupiter (orange, large)
    bodies.push_back(Body("Jupiter", 150.0, 150.0, -35.0, 35.0, 2.5, Color(255, 165, 0), 8.0));
    
    // Font for text
    Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
        cout << "Font loading failed, but simulation will continue" << endl;
    }
    
    Text info;
    info.setFont(font);
    info.setCharacterSize(16);
    info.setFillColor(Color::White);
    info.setPosition(10, 10);
    
    Clock clock;
    float lastTime = 0;
    
    // Main loop
    while (window.isOpen()) {
        float currentTime = clock.getElapsedTime().asSeconds();
        float deltaTime = currentTime - lastTime;
        
        // Handle events
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                window.close();
        }
        
        // Update physics (30 times per second)
        if (deltaTime >= 0.033f) {
            // Reset forces
            vector<double> fx(bodies.size(), 0.0);
            vector<double> fy(bodies.size(), 0.0);
            
            // Compute forces
            for (size_t i = 0; i < bodies.size(); i++) {
                for (size_t j = i + 1; j < bodies.size(); j++) {
                    bodies[i].computeForce(bodies[j], fx[i], fy[i]);
                    bodies[j].computeForce(bodies[i], fx[j], fy[j]);
                }
            }
            
            // Update positions
            for (size_t i = 0; i < bodies.size(); i++) {
                bodies[i].update(fx[i], fy[i]);
            }
            
            lastTime = currentTime;
        }
        
        // Draw
        window.clear(Color::Black);
        
        // Draw bodies
        for (auto& body : bodies) {
            body.draw(window);
        }
        
        // Draw info text
        info.setString("Gravity Simulation - Planets orbiting Sun\n"
                       "Yellow: Sun | Blue: Earth | Red: Mars | Green: Venus | Orange: Jupiter\n"
                       "Press ESC to exit");
        window.draw(info);
        
        window.display();
    }
    
    cout << "\nSimulation Closed" << endl;
    return 0;
}