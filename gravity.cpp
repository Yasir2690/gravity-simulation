#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

const double G = 6.67430e-11;
const double DT = 100.0;
const double SOFTENING = 10.0;

class Body {
public:
    double x, y, vx, vy, mass;
    string name;
    
    Body(string n, double px, double py, double vx0, double vy0, double m) {
        name = n;
        x = px;
        y = py;
        vx = vx0;
        vy = vy0;
        mass = m;
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
    
    void print() {
        cout << name << ": (" << (int)x << ", " << (int)y << ")";
    }
};

int main() {
    cout << "========================================" << endl;
    cout << "     N-BODY GRAVITY SIMULATION" << endl;
    cout << "========================================" << endl;
    cout << "Simulating planets orbiting a sun..." << endl;
    cout << endl;
    
    vector<Body> bodies;
    
    // Sun (center, massive)
    bodies.push_back(Body("Sun   ", 0.0, 0.0, 0.0, 0.0, 1000.0));
    
    // Earth-like planet
    bodies.push_back(Body("Earth ", 80.0, 0.0, 0.0, 45.0, 1.0));
    
    // Mars-like planet
    bodies.push_back(Body("Mars  ", -60.0, 0.0, 0.0, -35.0, 0.8));
    
    // Venus-like planet
    bodies.push_back(Body("Venus ", 0.0, 70.0, -38.0, 0.0, 0.9));
    
    // Jupiter-like planet (large, far)
    bodies.push_back(Body("Jupiter", 120.0, 120.0, -30.0, 30.0, 2.0));
    
    cout << "Bodies in simulation:" << endl;
    for (auto& b : bodies) {
        cout << "  - " << b.name << " (mass: " << b.mass << ")" << endl;
    }
    cout << endl;
    
    cout << "Running simulation..." << endl;
    cout << "---------------------" << endl;
    
    for (int step = 0; step <= 500; step++) {
        // Reset forces
        vector<double> fx(bodies.size(), 0.0);
        vector<double> fy(bodies.size(), 0.0);
        
        // Compute all gravitational forces
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
        
        // Print progress every 50 steps
        if (step % 50 == 0) {
            cout << "Step " << setw(3) << step << ": ";
            for (auto& b : bodies) {
                cout << b.name << "(" << (int)b.x << "," << (int)b.y << ") ";
            }
            cout << endl;
        }
    }
    
    cout << "---------------------" << endl;
    cout << "\nSIMULATION COMPLETE!" << endl;
    cout << "\nFinal positions:" << endl;
    for (auto& b : bodies) {
        cout << "  " << b.name << ": (" << (int)b.x << ", " << (int)b.y << ")" << endl;
    }
    
    return 0;
}