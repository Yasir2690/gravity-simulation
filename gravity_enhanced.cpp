#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>

using namespace sf;
using namespace std;

const float G = 0.8f;
float DT = 0.01f;
const float SOFTENING = 5.0f;

bool paused = false;
bool showTrails = true;
int trailLength = 50;
float speedMultiplier = 1.0f;

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
    string name;
    
    Body(float x, float y, float vx, float vy, float m, float r, Color c, string n) {
        position = Vector2f(x, y);
        velocity = Vector2f(vx, vy);
        acceleration = Vector2f(0, 0);
        mass = m;
        radius = r;
        color = c;
        name = n;
    }
    
    void calculateForce(const Body& other, Vector2f& force) {
        Vector2f direction = other.position - position;
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y) + SOFTENING;
        float forceMagnitude = (G * mass * other.mass) / (distance * distance);
        force += direction / distance * forceMagnitude;
    }
    
    void update() {
        if (!paused) {
            velocity += acceleration * DT * speedMultiplier;
            position += velocity * DT * speedMultiplier;
        }
        acceleration = Vector2f(0, 0);
        
        if (showTrails) {
            trails.push_back({position, 1.0f});
            while (trails.size() > trailLength) {
                trails.erase(trails.begin());
            }
            
            for (auto& trail : trails) {
                trail.life -= 0.02f;
            }
        }
    }
    
    void draw(RenderWindow& window, Font& font) {
        if (showTrails) {
            for (const auto& trail : trails) {
                if (trail.life > 0) {
                    CircleShape trailCircle(radius * 0.4f);
                    trailCircle.setPosition(Vector2f(trail.position.x - radius * 0.4f, trail.position.y - radius * 0.4f));
                    trailCircle.setFillColor(Color(color.r, color.g, color.b, (uint8_t)(trail.life * 100)));
                    window.draw(trailCircle);
                }
            }
        }
        
        CircleShape circle(radius);
        circle.setPosition(Vector2f(position.x - radius, position.y - radius));
        circle.setFillColor(color);
        window.draw(circle);
        
        // Draw name label - FIXED: Text requires font in constructor
        Text label(font, name, 12);
        label.setFillColor(Color::White);
        label.setPosition(Vector2f(position.x - 15, position.y - radius - 15));
        window.draw(label);
    }
    
    float getSpeed() {
        return sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    }
    
    float getDistanceFromCenter() {
        return sqrt(position.x * position.x + position.y * position.y);
    }
};

int main() {
    RenderWindow window(VideoMode(Vector2u(1400, 900)), "Enhanced Gravity Simulation - BETTER than YouTube!");
    window.setFramerateLimit(60);
    
    View view(FloatRect(Vector2f(-500, -400), Vector2f(1000, 800)));
    window.setView(view);
    
    Font font;
    font.openFromFile("C:/Windows/Fonts/Arial.ttf");
    
    vector<Body> bodies;
    
    // Sun
    bodies.push_back(Body(0, 0, 0, 0, 400.0f, 25.0f, Color::Yellow, "SUN"));
    
    // 5 colorful planets
    bodies.push_back(Body(200, 0, 0, 48, 1.5f, 9.0f, Color::Blue, "EARTH"));
    bodies.push_back(Body(-180, 0, 0, -42, 1.3f, 8.0f, Color::Red, "MARS"));
    bodies.push_back(Body(0, 190, -45, 0, 1.4f, 8.5f, Color::Green, "VENUS"));
    bodies.push_back(Body(260, 260, -38, 38, 2.5f, 14.0f, Color(255, 165, 0), "JUPITER"));
    bodies.push_back(Body(-250, 150, 35, -35, 1.8f, 10.0f, Color(100, 200, 255), "NEPTUNE"));
    
    // Stars background
    vector<CircleShape> stars;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1400);
    for (int i = 0; i < 800; i++) {
        CircleShape star(1);
        star.setPosition(Vector2f(dis(gen), dis(gen)));
        star.setFillColor(Color(100 + rand() % 155, 100 + rand() % 155, 100 + rand() % 155));
        stars.push_back(star);
    }
    
    Clock clock;
    float lastTime = 0;
    bool dragging = false;
    Vector2f lastMousePos;
    bool showUI = true;
    
    while (window.isOpen()) {
        float currentTime = clock.getElapsedTime().asSeconds();
        float deltaTime = currentTime - lastTime;
        
        while (const optional<Event> event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
            
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->code == Keyboard::Key::Escape)
                    window.close();
                if (keyPressed->code == Keyboard::Key::Space)
                    paused = !paused;
                if (keyPressed->code == Keyboard::Key::R)
                    view.setCenter(Vector2f(0, 0));
                if (keyPressed->code == Keyboard::Key::T)
                    showTrails = !showTrails;
                if (keyPressed->code == Keyboard::Key::U)
                    showUI = !showUI;
                if (keyPressed->code == Keyboard::Key::Up)
                    speedMultiplier = min(5.0f, speedMultiplier + 0.1f);
                if (keyPressed->code == Keyboard::Key::Down)
                    speedMultiplier = max(0.1f, speedMultiplier - 0.1f);
                if (keyPressed->code == Keyboard::Key::Q) {
                    random_device rd2;
                    mt19937 gen2(rd2());
                    uniform_real_distribution<> dis2(-300, 300);
                    float x = dis2(gen2);
                    float y = dis2(gen2);
                    float vx = -y * 0.2f;
                    float vy = x * 0.2f;
                    bodies.push_back(Body(x, y, vx, vy, 1.0f, 7.0f, 
                        Color(rand() % 255, rand() % 255, rand() % 255), 
                        "NEW"));
                }
            }
            
            // Add planet on left click
            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left && !dragging) {
                    Vector2f mouseWorld = window.mapPixelToCoords(Mouse::getPosition(window));
                    float x = mouseWorld.x;
                    float y = mouseWorld.y;
                    float vx = -y * 0.15f;
                    float vy = x * 0.15f;
                    bodies.push_back(Body(x, y, vx, vy, 1.0f, 7.0f, 
                        Color(rand() % 255, rand() % 255, rand() % 255), 
                        "NEW"));
                }
                if (mousePressed->button == Mouse::Button::Right) {
                    dragging = true;
                    lastMousePos = Vector2f(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
                }
            }
            
            if (const auto* mouseReleased = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseReleased->button == Mouse::Button::Right) {
                    dragging = false;
                }
            }
            
            if (const auto* wheelScrolled = event->getIf<Event::MouseWheelScrolled>()) {
                if (wheelScrolled->wheel == Mouse::Wheel::Vertical) {
                    if (wheelScrolled->delta > 0)
                        view.zoom(0.9f);
                    else
                        view.zoom(1.1f);
                    window.setView(view);
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
        
        for (auto& star : stars) {
            window.draw(star);
        }
        
        for (auto& body : bodies) {
            body.draw(window, font);
        }
        
        if (showUI) {
            RectangleShape uiBackground(Vector2f(280, 320));
            uiBackground.setFillColor(Color(0, 0, 0, 200));
            uiBackground.setPosition(Vector2f(10, 10));
            window.draw(uiBackground);
            
            stringstream ss;
            ss << "=== CONTROLS ===\n";
            ss << "Space: Pause\n";
            ss << "T: Trails\n";
            ss << "R: Reset View\n";
            ss << "U: Hide UI\n";
            ss << "Up/Down: Speed\n";
            ss << "Q: Random Planet\n";
            ss << "Left Click: Add\n";
            ss << "Right Drag: Pan\n";
            ss << "Scroll: Zoom\n\n";
            ss << "=== STATUS ===\n";
            ss << (paused ? "PAUSED" : "RUNNING") << "\n";
            ss << "Bodies: " << bodies.size() << "\n";
            ss << "Speed: x" << fixed << setprecision(1) << speedMultiplier << "\n";
            ss << "Trails: " << (showTrails ? "ON" : "OFF") << "\n";
            
            Text uiText(font, ss.str(), 14);
            uiText.setFillColor(Color::White);
            uiText.setPosition(Vector2f(20, 20));
            window.draw(uiText);
        }
        
        window.display();
    }
    
    return 0;
}