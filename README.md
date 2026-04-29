\# 🌌 N-Body Gravity Simulation



An \*\*ULTIMATE\*\* N-body gravity simulation built in \*\*C++ with SFML\*\* – \*\*BETTER than YouTube tutorials!\*\*



\[!\[C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)

\[!\[SFML](https://img.shields.io/badge/SFML-3.0-green.svg)](https://www.sfml-dev.org/)

\[!\[License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)



\## ✨ Features (YouTube doesn't have these!)



| Feature | Description |

|---------|-------------|

| 🌟 | \*\*7 Planets + Sun\*\* with realistic gravity |

| 🎨 | \*\*Rainbow Trails\*\* - Colorful planet paths |

| 👻 | \*\*Ghost Mode\*\* - See-through planets |

| 🎯 | \*\*Follow Camera\*\* - Track any planet |

| ⏪ | \*\*Rewind Time\*\* - Go back in time! |

| 💥 | \*\*Collision Detection\*\* - Planets merge when they hit |

| 🎮 | \*\*Add Planets\*\* - Left click anywhere |

| ☄️ | \*\*Add Comets\*\* - Press Q |

| 🏷️ | \*\*Rename Planets\*\* - Press N |

| 🎨 | \*\*Color Picker\*\* - Press C + click planet |

| ⚡ | \*\*Speed Control\*\* - Up/Down arrows |

| 🔍 | \*\*Zoom \& Pan\*\* - Mouse wheel + right drag |

| ⏸️ | \*\*Pause/Resume\*\* - Space bar |

| 📊 | \*\*Real-time FPS \& Energy display\*\* |



\## 🎮 Controls



| Key | Action |

|-----|--------|

| `Space` | Pause/Resume simulation |

| `R` | Reset camera to center |

| `T` | Toggle trails ON/OFF |

| `G` | Ghost mode (transparent planets) |

| `X` | Rainbow trails mode |

| `C` | Color picker mode |

| `N` | Rename selected planet |

| `Q` | Add a shooting comet |

| `F` | Follow mode (camera follows planet) |

| `Z` | Rewind time (undo last 10 seconds) |

| `↑` | Increase simulation speed |

| `↓` | Decrease simulation speed |

| `0-9` | Select planet to follow |

| `Left Click` | Add new planet at mouse position |

| `Right Drag` | Pan camera |

| `Mouse Wheel` | Zoom in/out |



\## 🚀 How to Run



\### Prerequisites

\- \*\*C++ Compiler\*\* (MinGW/GCC)

\- \*\*SFML 3.0\*\* graphics library



\### Compile \& Run

```bash

g++ gravity\_ultimate\_fixed.cpp -o gravity.exe -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system

./gravity.exe



🧠 Physics

This simulation uses Newton's Law of Universal Gravitation:





F = G \* (m₁ \* m₂) / r²

Real-time N-body physics (all planets attract all others)



Euler integration for smooth motion



Softening factor to prevent numerical explosions



Conservation of energy visualization



📁 Files

File	Description

gravity\_ultimate\_fixed.cpp	Complete working version with ALL features

gravity\_animated.cpp	Simple console animation

gravity\_exact.cpp	Basic SFML graphics version



📦 Dependencies

SFML 3.0 - Graphics, window, and input handling



MinGW-w64 - C++ compiler for Windows





