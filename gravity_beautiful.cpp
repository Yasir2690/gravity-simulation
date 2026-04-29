#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

const double G = 0.2;
const double DT = 0.5;
const double SOFTENING = 5.0;

class Body {
public:
    double x, y, vx, vy, mass;
    string name;
    char symbol;
    int r, g, b;
    
    Body(string n, double px, double py, double vx0, double vy0, double m, char sym, int red, int green, int blue) {
        name = n;
        x = px; y = py; vx = vx0; vy = vy0; mass = m;
        symbol = sym;
        r = red; g = green; b = blue;
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

void setColor(int r, int g, int b) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (r>128?4:0) | (g>128?2:0) | (b>128?1:0));
#endif
}

void resetColor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {
    cout << "\033[2J\033[1;1H";
    cout << "========================================" << endl;
    cout << "    N-BODY GRAVITY SIMULATION" << endl;
    cout << "========================================" << endl;
    cout << "Press Ctrl+C to exit" << endl;
    cout << endl;
    
    vector<Body> bodies;
    
    bodies.push_back(Body("Sun", 0, 0, 0, 0, 200, '@', 255, 255, 0));
    bodies.push_back(Body("Earth", 120, 0, 0, 28, 1, 'E', 0, 100, 255));
    bodies.push_back(Body("Mars", -100, 0, 0, -25, 0.8, 'M', 255, 50, 50));
    bodies.push_back(Body("Venus", 0, 110, -26, 0, 0.9, 'V', 50, 255, 50));
    bodies.push_back(Body("Jupiter", 160, 160, -22, 22, 2.5, 'J', 255, 165, 0));
    
    int width = 70;
    int height = 30;
    int step = 0;
    
    while (true) {
        vector<double> fx(bodies.size(), 0);
        vector<double> fy(bodies.size(), 0);
        
        for (size_t i = 0; i < bodies.size(); i++) {
            for (size_t j = i + 1; j < bodies.size(); j++) {
                bodies[i].computeForce(bodies[j], fx[i], fy[i]);
                bodies[j].computeForce(bodies[i], fx[j], fy[j]);
            }
        }
        
        for (size_t i = 0; i < bodies.size(); i++) {
            bodies[i].update(fx[i], fy[i]);
        }
        
        clearScreen();
        
        cout << "GRAVITY SIMULATION - Step: " << step++ << endl;
        cout << "Legend: @=Sun(Yellow) E=Earth(Blue) M=Mars(Red) V=Venus(Green) J=Jupiter(Orange)" << endl;
        cout << string(width + 2, '-') << endl;
        
        vector<string> screen(height, string(width, ' '));
        
        for (const auto& body : bodies) {
            int px = (int)((body.x + 200) / 400 * width);
            int py = (int)((150 - body.y) / 300 * height);
            
            if (px >= 0 && px < width && py >= 0 && py < height) {
                screen[py][px] = body.symbol;
            }
        }
        
        for (const auto& line : screen) {
            cout << "|" << line << "|" << endl;
        }
        
        cout << string(width + 2, '-') << endl;
        
        cout << "\nPositions:" << endl;
        for (const auto& body : bodies) {
            cout << "  " << body.symbol << " " << body.name << ": (" << (int)body.x << ", " << (int)body.y << ")" << endl;
        }
        
        this_thread::sleep_for(chrono::milliseconds(80));
    }
    
    return 0;
}