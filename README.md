# Night Highway Patrol (VS Code)

## Description
This is the VS Code adapted version of the Night Highway Patrol project using MSYS2 + FreeGLUT. It has a clean folder structure and includes .vscode configuration files for building and debugging (tasks.json, launch.json, c_cpp_properties.json).

## Features

- Works on Windows with MSYS2 installed
- Clean folder structure, no compiled binaries included
- Supports building via Ctrl+Shift+B and running/debugging via F5
- Implements advanced graphics algorithms: DDA Line, Bresenham Line, and Midpoint Circle
- Interactive police car chase game with realistic physics
- Night highway scene with dynamic lighting effects

## Usage

1. Clone or download the repository
2. Make sure MSYS2 is installed with:
   - `mingw-w64-x86_64-gcc`
   - `mingw-w64-x86_64-freeglut`
3. Open the folder in VS Code
4. Press `Ctrl+Shift+B` to build the project
5. Press `F5` to run

## Prerequisites

### Install MSYS2
Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)

### Install Required Packages
Open MSYS2 terminal and run:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-freeglut
pacman -S mingw-w64-x86_64-gdb
```

### Install VS Code Extensions
Recommended extensions:
- C/C++ Extension Pack
- C/C++ IntelliSense

## Game Controls

- **Arrow Keys**: Move police car left/right
- **S**: Toggle siren on/off  
- **P**: Pause/Resume game
- **R**: Restart game
- **ESC**: Exit game

## Project Structure

```
Night_Highway_Patrol_Glut_Project/
├── .vscode/
│   ├── c_cpp_properties.json    # IntelliSense configuration
│   ├── launch.json              # Debugging configuration
│   └── tasks.json               # Build tasks
├── .gitignore                   # Excludes binaries and temp files
├── main.cpp                     # Main game source code
└── README.md                    # This file
```

## Building and Running

### Quick Start
- **Build**: `Ctrl+Shift+B`
- **Run/Debug**: `F5`
- **Run without debugging**: `Ctrl+F5`

### Manual Build
```bash
g++ main.cpp -o main.exe -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib -lfreeglut -lopengl32 -lglu32
```

## Troubleshooting

If you encounter build issues:
1. Verify MSYS2 installation
2. Check that `C:/msys64/mingw64/bin` is in your PATH
3. Ensure all required packages are installed
4. Restart VS Code after installing MSYS2

## Notes

- `.gitignore` is used to exclude binaries (.exe) and temporary files
- Recommended for users who prefer VS Code over CodeBlocks
- All VS Code configuration files are included for seamless development experience
- Project uses modern C++ standards and best practices

## License

Open source project. Feel free to modify and distribute.
