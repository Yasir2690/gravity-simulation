#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>
#include <deque>
#include <algorithm>

using namespace sf;
using namespace std;

const float G = 0.8f;
float DT = 0.01f;
const float SOFTENING = 5.0f;

bool paused = false;
bool showTrails = true;
bool rainbowTrails = false;
bool ghostMode = false;
bool followMode = false;
int followIndex = 0;
int trailLength = 50;
float speedMultiplier = 1.0f;
float globalTime = 0.0f;

// Rewind system
deque<vector<pair<Vector2f, Vector2f>>> history;
const int MAX_HISTORY = 600;

// FPS counter
float fps = 0;
int frameCount = 0;
float fpsTimer = 0;

// Color picker mode
bool colorPickMode = false;
int selectedPlanet = -1;

// Graphs
vector<float> energyHistory;
vector<float> velocityHistory;

struct Trail {
    Vector2f position;
    float life;
    Color color;
};

struct Comet {
    Vector2f position;
    Vector2f velocity;
    float life;
    Color color;
};

class Body {
public:
    Vector2f position;
    Vector2f velocity;
    Vector2f acceleration;
    float mass;
    float radius;
    Color color;
    deque<Trail> trails;
    string name;
    float lastCollisionTime = 0;
    
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
            Trail newTrail;
            newTrail.position = position;
            newTrail.life = 1.0f;
            if (rainbowTrails) {
                float hue = fmod(globalTime * 2 + trails.size(), 360);
                newTrail.color = Color(
                    (int)(sin(hue * 3.14159f/180.0f) * 127 + 128),
                    (int)(sin((hue + 120) * 3.14159f/180.0f) * 127 + 128),
                    (int)(sin((hue + 240) * 3.14159f/180.0f) * 127 + 128)
                );
            } else {
                newTrail.color = color;
            }
            trails.push_front(newTrail);
            while (trails.size() > trailLength) {
                trails.pop_back();
            }
            
            for (auto& trail : trails) {
                trail.life -= 0.02f;
            }
        }
    }
    
    void draw(RenderWindow& window, Font& font, bool isSelected) {
        if (showTrails) {
            for (const auto& trail : trails) {
                if (trail.life > 0) {
                    CircleShape trailCircle(radius * 0.4f);
                    trailCircle.setPosition(Vector2f(trail.position.x - radius * 0.4f, trail.position.y - radius * 0.4f));
                    Color trailColor = trail.color;
                    trailColor.a = (uint8_t)(trail.life * 100);
                    trailCircle.setFillColor(trailColor);
                    window.draw(trailCircle);
                }
            }
        }
        
        CircleShape circle(radius);
        circle.setPosition(Vector2f(position.x - radius, position.y - radius));
        
        if (ghostMode && !isSelected) {
            Color ghostColor = color;
            ghostColor.a = 100;
            circle.setFillColor(ghostColor);
        } else {
            circle.setFillColor(color);
        }
        
        if (isSelected) {
            circle.setOutlineThickness(3);
            circle.setOutlineColor(Color::White);
        }
        
        window.draw(circle);
        
        Text label(font, name, 12);
        label.setFillColor(Color::White);
        label.setPosition(Vector2f(position.x - 15, position.y - radius - 15));
        window.draw(label);
    }
    
    float getSpeed() {
        return sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    }
    
    float getEnergy() {
        return 0.5f * mass * getSpeed() * getSpeed();
    }
};

vector<Body> bodies;
vector<Comet> comets;
Font font;
View view;
Clock gameClock;  // Renamed from 'clock' to avoid conflict
float lastTime = 0;
bool dragging = false;
Vector2f lastMousePos;
bool showUI = true;
float totalEnergy = 0;
bool renamingMode = false;
int renamingIndex = -1;
string newName = "";

void addComet() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> posDis(-400, 400);
    uniform_real_distribution<> velDis(-80, 80);
    
    Comet comet;
    comet.position = Vector2f(posDis(gen), posDis(gen));
    comet.velocity = Vector2f(velDis(gen), velDis(gen));
    comet.life = 1.0f;
    comet.color = Color(255, 200, 100);
    comets.push_back(comet);
}

void saveState() {
    vector<pair<Vector2f, Vector2f>> state;
    for (const auto& body : bodies) {
        state.push_back({body.position, body.velocity});
    }
    history.push_back(state);
    if (history.size() > MAX_HISTORY) {
        history.pop_front();
    }
}

void rewind() {
    if (!history.empty()) {
        auto state = history.back();
        history.pop_back();
        for (size_t i = 0; i < bodies.size() && i < state.size(); i++) {
            bodies[i].position = state[i].first;
            bodies[i].velocity = state[i].second;
        }
    }
}

void checkCollisions() {
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            float dist = sqrt(pow(bodies[i].position.x - bodies[j].position.x, 2) + 
                             pow(bodies[i].position.y - bodies[j].position.y, 2));
            if (dist < bodies[i].radius + bodies[j].radius) {
                float newMass = bodies[i].mass + bodies[j].mass;
                Vector2f newPos = (bodies[i].position * bodies[i].mass + bodies[j].position * bodies[j].mass) / newMass;
                Vector2f newVel = (bodies[i].velocity * bodies[i].mass + bodies[j].velocity * bodies[j].mass) / newMass;
                float newRadius = sqrt(newMass) * 1.5f;
                Color newColor = Color(
                    (bodies[i].color.r + bodies[j].color.r) / 2,
                    (bodies[i].color.g + bodies[j].color.g) / 2,
                    (bodies[i].color.b + bodies[j].color.b) / 2
                );
                string newName = bodies[i].name + "+" + bodies[j].name;
                
                bodies.erase(bodies.begin() + j);
                bodies.erase(bodies.begin() + i);
                bodies.push_back(Body(newPos.x, newPos.y, newVel.x, newVel.y, newMass, newRadius, newColor, newName));
                return;
            }
        }
    }
}

int main() {
    RenderWindow window(VideoMode(Vector2u(1600, 1000)), "ULTIMATE GRAVITY SIMULATION - Better than YouTube!");
    window.setFramerateLimit(60);
    
    view = View(FloatRect(Vector2f(-600, -450), Vector2f(1200, 900)));
    window.setView(view);
    
    if (!font.openFromFile("C:/Windows/Fonts/Arial.ttf")) {
        // Font loading failed, continue without text
        cout << "Font not found" << endl;
    }
    
    bodies.push_back(Body(0, 0, 0, 0, 500.0f, 30.0f, Color::Yellow, "SUN"));
    bodies.push_back(Body(220, 0, 0, 50, 1.5f, 10.0f, Color::Blue, "EARTH"));
    bodies.push_back(Body(-190, 0, 0, -44, 1.3f, 9.0f, Color::Red, "MARS"));
    bodies.push_back(Body(0, 210, -48, 0, 1.4f, 9.5f, Color::Green, "VENUS"));
    bodies.push_back(Body(280, 280, -40, 40, 3.0f, 16.0f, Color(255, 165, 0), "JUPITER"));
    bodies.push_back(Body(-270, 160, 38, -38, 2.0f, 11.0f, Color(100, 200, 255), "NEPTUNE"));
    bodies.push_back(Body(150, -200, -35, -35, 1.2f, 8.0f, Color(255, 100, 255), "URANUS"));
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1600);
    
    lastTime = gameClock.getElapsedTime().asSeconds();
    
    while (window.isOpen()) {
        float currentTime = gameClock.getElapsedTime().asSeconds();
        float deltaTime = currentTime - lastTime;
        
        while (const optional<Event> event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
            
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->code == Keyboard::Key::Escape) window.close();
                if (keyPressed->code == Keyboard::Key::Space) paused = !paused;
                if (keyPressed->code == Keyboard::Key::R) view.setCenter(Vector2f(0, 0));
                if (keyPressed->code == Keyboard::Key::T) showTrails = !showTrails;
                if (keyPressed->code == Keyboard::Key::U) showUI = !showUI;
                if (keyPressed->code == Keyboard::Key::Up) speedMultiplier = min(5.0f, speedMultiplier + 0.1f);
                if (keyPressed->code == Keyboard::Key::Down) speedMultiplier = max(0.1f, speedMultiplier - 0.1f);
                if (keyPressed->code == Keyboard::Key::Q) addComet();
                if (keyPressed->code == Keyboard::Key::C) colorPickMode = !colorPickMode;
                if (keyPressed->code == Keyboard::Key::G) ghostMode = !ghostMode;
                if (keyPressed->code == Keyboard::Key::X) rainbowTrails = !rainbowTrails;
                if (keyPressed->code == Keyboard::Key::F) followMode = !followMode;
                if (keyPressed->code == Keyboard::Key::Z && !history.empty()) rewind();
                if (keyPressed->code == Keyboard::Key::N) {
                    renamingMode = true;
                    renamingIndex = followIndex;
                    newName = "";
                }
                if (keyPressed->code >= Keyboard::Key::Num0 && keyPressed->code <= Keyboard::Key::Num9 && followMode) {
                    int num = (int)keyPressed->code - (int)Keyboard::Key::Num0;
                    if (num < (int)bodies.size()) followIndex = num;
                }
            }
            
            if (const auto* textEntered = event->getIf<Event::TextEntered>()) {
                if (renamingMode && textEntered->unicode < 128) {
                    if (textEntered->unicode == 13) {
                        if (renamingIndex >= 0 && renamingIndex < (int)bodies.size()) {
                            bodies[renamingIndex].name = newName;
                        }
                        renamingMode = false;
                        renamingIndex = -1;
                        newName = "";
                    } else if (textEntered->unicode == 8) {
                        if (!newName.empty()) newName.pop_back();
                    } else {
                        newName += (char)textEntered->unicode;
                    }
                }
            }
            
            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left && !dragging) {
                    Vector2f mouseWorld = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    if (colorPickMode) {
                        for (int i = 0; i < (int)bodies.size(); i++) {
                            float dist = sqrt(pow(bodies[i].position.x - mouseWorld.x, 2) + 
                                             pow(bodies[i].position.y - mouseWorld.y, 2));
                            if (dist < bodies[i].radius) {
                                selectedPlanet = i;
                                break;
                            }
                        }
                    } else {
                        float x = mouseWorld.x;
                        float y = mouseWorld.y;
                        float vx = -y * 0.15f;
                        float vy = x * 0.15f;
                        bodies.push_back(Body(x, y, vx, vy, 1.0f, 8.0f, 
                            Color(rand() % 255, rand() % 255, rand() % 255), "PLANET"));
                    }
                }
                if (mousePressed->button == Mouse::Button::Right) {
                    dragging = true;
                    lastMousePos = Vector2f(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
                }
            }
            
            if (const auto* mouseReleased = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseReleased->button == Mouse::Button::Right) dragging = false;
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
            view.move((lastMousePos - currentMousePos) * 0.5f);
            window.setView(view);
            lastMousePos = currentMousePos;
        }
        
        if (followMode && followIndex < (int)bodies.size()) {
            view.setCenter(bodies[followIndex].position);
            window.setView(view);
        }
        
        if (deltaTime >= 0.016f) {
            globalTime += deltaTime;
            
            if (!paused) {
                saveState();
                
                for (auto& body : bodies) body.acceleration = Vector2f(0, 0);
                for (size_t i = 0; i < bodies.size(); i++) {
                    Vector2f totalForce(0, 0);
                    for (size_t j = 0; j < bodies.size(); j++) {
                        if (i != j) bodies[i].calculateForce(bodies[j], totalForce);
                    }
                    bodies[i].acceleration = totalForce / bodies[i].mass;
                }
                for (auto& body : bodies) body.update();
                checkCollisions();
                
                totalEnergy = 0;
                for (auto& body : bodies) totalEnergy += body.getEnergy();
                energyHistory.push_back(totalEnergy);
                if (energyHistory.size() > 300) energyHistory.erase(energyHistory.begin());
                
                float avgVel = 0;
                for (auto& body : bodies) avgVel += body.getSpeed();
                if (bodies.size() > 0) avgVel /= bodies.size();
                velocityHistory.push_back(avgVel);
                if (velocityHistory.size() > 300) velocityHistory.erase(velocityHistory.begin());
            }
            
            for (auto& comet : comets) {
                comet.position += comet.velocity * DT;
                comet.life -= 0.01f;
            }
            comets.erase(remove_if(comets.begin(), comets.end(), [](const Comet& c) { return c.life <= 0; }), comets.end());
            
            lastTime = currentTime;
        }
        
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            fps = frameCount / fpsTimer;
            frameCount = 0;
            fpsTimer = 0;
        }
        
        window.clear(Color::Black);
        
        // Draw stars background
        for (int i = 0; i < 500; i++) {
            CircleShape star(1);
            star.setFillColor(Color(100 + rand() % 155, 100 + rand() % 155, 100 + rand() % 155));
            star.setPosition(Vector2f(rand() % 1600, rand() % 1000));
            window.draw(star);
        }
        
        for (auto& comet : comets) {
            CircleShape cometCircle(3);
            cometCircle.setPosition(comet.position);
            cometCircle.setFillColor(comet.color);
            window.draw(cometCircle);
        }
        
        for (int i = 0; i < (int)bodies.size(); i++) {
            bodies[i].draw(window, font, (followMode && i == followIndex));
        }
        
        if (showUI) {
            RectangleShape uiBg(Vector2f(340, 550));
            uiBg.setFillColor(Color(0, 0, 0, 220));
            uiBg.setPosition(Vector2f(10, 10));
            window.draw(uiBg);
            
            stringstream ss;
            ss << "=== ULTIMATE GRAVITY SIM ===\n";
            ss << "\n=== CONTROLS ===\n";
            ss << "Space: Pause | R: Reset View\n";
            ss << "T: Trails | G: Ghost Mode\n";
            ss << "X: Rainbow Trails\n";
            ss << "C: Color Pick | N: Rename\n";
            ss << "Q: Add Comet | F: Follow Mode\n";
            ss << "Z: Rewind | Up/Down: Speed\n";
            ss << "Left Click: Add Planet\n";
            ss << "Right Drag: Pan | Scroll: Zoom\n";
            ss << "0-9: Select Planet (Follow)\n";
            ss << "\n=== STATUS ===\n";
            ss << (paused ? "PAUSED" : "RUNNING") << " | FPS: " << (int)fps << "\n";
            ss << "Bodies: " << bodies.size() << "\n";
            ss << "Speed: x" << fixed << setprecision(1) << speedMultiplier << "\n";
            ss << "Total Energy: " << (int)totalEnergy << "\n";
            if (followMode && followIndex < (int)bodies.size()) ss << "Following: " << bodies[followIndex].name << "\n";
            if (colorPickMode) ss << "COLOR PICK MODE ACTIVE\n";
            if (renamingMode) ss << "Rename: " << newName << "_\n";
            
            Text uiText(font, ss.str(), 14);
            uiText.setFillColor(Color::White);
            uiText.setPosition(Vector2f(20, 20));
            window.draw(uiText);
            
            // Simple energy bar
            if (!energyHistory.empty()) {
                RectangleShape energyBarBg(Vector2f(300, 20));
                energyBarBg.setFillColor(Color(50, 50, 50));
                energyBarBg.setPosition(Vector2f(20, 480));
                window.draw(energyBarBg);
                
                float energyPercent = min(1.0f, energyHistory.back() / 10000.0f);
                RectangleShape energyBar(Vector2f(300 * energyPercent, 20));
                energyBar.setFillColor(Color::Yellow);
                energyBar.setPosition(Vector2f(20, 480));
                window.draw(energyBar);
                
                Text energyText(font, "Energy: " + to_string((int)energyHistory.back()), 12);
                energyText.setFillColor(Color::White);
                energyText.setPosition(Vector2f(20, 460));
                window.draw(energyText);
            }
        }
        
        if (colorPickMode && selectedPlanet >= 0 && selectedPlanet < (int)bodies.size()) {
            RectangleShape pickerBg(Vector2f(200, 150));
            pickerBg.setFillColor(Color(0, 0, 0, 200));
            pickerBg.setPosition(Vector2f(800, 400));
            window.draw(pickerBg);
            
            Text pickerText(font, "Pick Color for " + bodies[selectedPlanet].name, 16);
            pickerText.setFillColor(Color::White);
            pickerText.setPosition(Vector2f(810, 410));
            window.draw(pickerText);
            
            vector<Color> colors = {Color::Red, Color::Green, Color::Blue, Color::Yellow, 
                                     Color::Magenta, Color::Cyan, Color(255,165,0), Color(255,100,255)};
            for (int i = 0; i < 8; i++) {
                RectangleShape colorBox(Vector2f(40, 40));
                colorBox.setFillColor(colors[i]);
                colorBox.setPosition(Vector2f(810 + (i%4)*45, 450 + (i/4)*45));
                window.draw(colorBox);
            }
            
            Text hint(font, "Click on a color to select", 12);
            hint.setFillColor(Color::White);
            hint.setPosition(Vector2f(810, 540));
            window.draw(hint);
        }
        
        window.display();
    }
    
    return 0;
}