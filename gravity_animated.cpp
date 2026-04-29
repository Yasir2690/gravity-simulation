#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

const double G = 6.67430e-11;
const double DT = 500.0;
const double SOFTENING = 10.0;

class Body {
public:
    double x, y, vx, vy, mass;
    char symbol;
    
    Body(double px, double py, double vx0, double vy0, double m, char sym) {
        x = px; y = py; vx = vx0; vy = vy0; mass = m; symbol = sym;
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
};

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void drawBodies(const vector<Body>& bodies, int width, int height) {
    vector<vector<char>> screen(height, vector<char>(width, ' '));
    
    for (const auto& body : bodies) {
        int px = (int)((body.x + 200) / 400 * width);
        int py = (int)((200 - body.y) / 400 * height);
        
        if (px >= 0 && px < width && py >= 0 && py < height) {
            screen[py][px] = body.symbol;
        }
    }
    
    // Draw border
    cout << "+" << string(width, '-') << "+" << endl;
    for (int y = 0; y < height; y++) {
        cout << "|";
        for (int x = 0; x < width; x++) {
            cout << screen[y][x];
        }
        cout << "|" << endl;
    }
    cout << "+" << string(width, '-') << "+" << endl;
}

int main() {
    cout << "GRAVITY SIMULATION - Animated" << endl;
    cout << "=============================" << endl;
    cout << "Watch planets orbit around the Sun!" << endl;
    cout << "Close window to exit..." << endl;
    cout << endl;
    
    vector<Body> bodies;
    
    // Sun (center)
    bodies.push_back(Body(0.0, 0.0, 0.0, 0.0, 2000.0, '@'));
    
    // Earth-like planet
    bodies.push_back(Body(100.0, 0.0, 0.0, 50.0, 1.0, 'E'));
    
    // Mars-like planet  
    bodies.push_back(Body(-80.0, 0.0, 0.0, -40.0, 0.8, 'M'));
    
    // Venus-like planet
    bodies.push_back(Body(0.0, 90.0, -48.0, 0.0, 0.9, 'V'));
    
    // Jupiter-like planet (larger)
    bodies.push_back(Body(140.0, 140.0, -38.0, 38.0, 2.5, 'J'));
    
    int width = 70;
    int height = 30;
    int step = 0;
    
    while (true) {
        // Compute forces
        vector<double> fx(bodies.size(), 0.0);
        vector<double> fy(bodies.size(), 0.0);
        
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
        
        // Draw
        clearScreen();
        cout << "GRAVITY SIMULATION - Step: " << step << endl;
        cout << "@ = Sun, E = Earth, M = Mars, V = Venus, J = Jupiter" << endl;
        cout << endl;
        drawBodies(bodies, width, height);
        
        // Show positions
        cout << "\nPositions:" << endl;
        for (const auto& body : bodies) {
            cout << "  " << body.symbol << ": (" << (int)body.x << ", " << (int)body.y << ")" << endl;
        }
        
        step++;
        
        // Delay for animation
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    return 0;
}