# 🚔 Night Highway Patrol

> **High-speed police chase game** built with OpenGL/FreeGLUT featuring advanced graphics algorithms

![Language](https://img.shields.io/badge/Language-C++-blue.svg)
![Graphics](https://img.shields.io/badge/Graphics-OpenGL-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![IDE](https://img.shields.io/badge/IDE-VS%20Code-007ACC.svg)

## ✨ Features

🎮 **Interactive Gameplay**
- Real-time police car controls with physics simulation
- Dynamic civilian vehicle spawning based on speed
- High score tracking with persistent file storage

🎨 **Advanced Graphics**
- **DDA Line Algorithm** for road boundaries
- **Bresenham Line Algorithm** for vehicle outlines  
- **Midpoint Circle Algorithm** for wheels and lights
- Night scene with gradient sky and city lights

🚨 **Game Mechanics**
- Speed-based difficulty scaling
- Collision detection system
- Siren effects with visual indicators
- Criminal pursuit with zigzag AI patterns

## 🎯 Quick Start

```bash
# 1. Clone repository
git clone https://github.com/ar-sayeem/Night_Highway_Patrol_Glut_Project.git

# 2. Open in VS Code
code Night_Highway_Patrol_Glut_Project

# 3. Build & Run
Ctrl+Shift+B  # Build
F5            # Run with debugging
```

## 🛠️ Prerequisites

### MSYS2 Setup
```bash
# Install MSYS2 from: https://www.msys2.org/
# Then install packages:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-freeglut  
pacman -S mingw-w64-x86_64-gdb
```

### VS Code Extensions
- [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
- [C/C++ IntelliSense](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

## 🎮 Controls

| Key | Action |
|-----|--------|
| ⬅️➡️ | Steer police car |
| `S` | Toggle siren |
| `P` | Pause/Resume |
| `R` | Restart game |
| `ESC` | Exit |

## 📁 Project Structure

```
📦 Night_Highway_Patrol_Glut_Project
├── 🛠️ .vscode/
│   ├── c_cpp_properties.json    # IntelliSense config
│   ├── launch.json              # Debug config  
│   └── tasks.json               # Build automation
├── 🎯 main.cpp                  # Game source code
├── 📄 README.md                 # Documentation
├── 🚫 .gitignore               # Git ignore rules
└── 🏆 highscore.txt            # High score data
```

## 🔧 Build Commands

### VS Code (Recommended)
- **Build**: `Ctrl+Shift+B`
- **Debug**: `F5`
- **Run**: `Ctrl+F5`

### Manual Build
```bash
g++ main.cpp -o main.exe \
    -I C:/msys64/mingw64/include \
    -L C:/msys64/mingw64/lib \
    -lfreeglut -lopengl32 -lglu32 \
    -Wall -std=c++17
```

## 🎯 Game Objectives

- 🚔 **Chase criminals** - Catch zigzagging criminal vehicles for bonus points
- 🏃 **Avoid civilians** - Don't crash into innocent traffic
- 🏁 **Stay on road** - Don't hit the boundaries  
- 🏆 **Beat high score** - Challenge yourself to reach new heights

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| Build fails | ✅ Verify MSYS2 installation |
| Missing libraries | ✅ Install FreeGLUT packages |
| IntelliSense errors | ✅ Check `c_cpp_properties.json` paths |
| Game won't run | ✅ Ensure `mingw64/bin` in PATH |

## 🚀 Technical Highlights

- **Real-time rendering** at 60 FPS
- **Custom algorithms** implementation (DDA, Bresenham, Midpoint Circle)
- **Object-oriented design** with efficient collision detection
- **Dynamic memory management** for vehicle spawning
- **File I/O** for persistent high score storage

## 📜 License

**MIT License** - Feel free to use, modify, and distribute! 🚀

---

<div align="center">

**Made with ❤️ using OpenGL & C++**

[⭐ Star this repo](https://github.com/ar-sayeem/Night_Highway_Patrol_Glut_Project) • [🐛 Report bugs](https://github.com/ar-sayeem/Night_Highway_Patrol_Glut_Project/issues) • [✨ Request features](https://github.com/ar-sayeem/Night_Highway_Patrol_Glut_Project/issues)

</div>
