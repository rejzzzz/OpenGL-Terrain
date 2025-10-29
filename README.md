### CGM Project - Terrain

Team:

- Daivik Wadhwani
- Sai Vivash Dara
- Rejwanul Hoque

---

## Overview
This project renders a simple rolling procedural terrain, a movable cube "player", and a third‑person orbit camera with mouse + scroll interaction. The codebase has been modularised into `core` (application + scene loop), `scenes` (game logic), `camera`, `objects`, and `terrain` modules to make future extension easier (e.g. adding new scenes, replacing the renderer, etc.).

## Current Features
- Procedural height function (simple sin/cos hills)
- Movable object constrained to terrain surface
- Third‑person orbit camera (RMB drag to orbit, scroll to zoom)
- Scene abstraction for future expansion
- A skybox

## Requirements (Ubuntu / WSL)
Install toolchain & libraries:
```
sudo apt update
sudo apt install -y build-essential libglfw3-dev libglew-dev libglm-dev
```

## Build & Run
Single command build (no CMake required):
```
g++ -Iinclude \
	src/main.cpp \
	src/core/Application.cpp \
	src/scenes/PlayScene.cpp \
	src/terrain.cpp \
	src/objects.cpp \
	src/utils.cpp \
	src/objects/MovableObject.cpp \
	src/camera/Camera.cpp \
	src/skybox/skybox.cpp \
	-lGL -lGLU -lGLEW -lglfw -o terrain
```

## Project Structure
```
include/
	core/            Application + Scene interfaces
	scenes/          Scene headers (PlayScene etc.)
	camera/          Orbit camera
	objects/         MovableObject + future entities
	terrain.h        Terrain API
	objects.h        Drawing helpers (buildings/trees)
src/
	core/            Application implementation
	scenes/          Scene implementations
	camera/          Camera implementation
	objects/         MovableObject implementation
	terrain.cpp      Height + terrain drawing
	objects.cpp      Static object drawing
	utils.cpp        (placeholder for helpers)
	main.cpp         Entry point (sets up Application + Scene)
```

## Runtime Controls
| Action | Input |
|--------|-------|
| Move   | WASD  |
| Orbit Camera | Hold Right Mouse + Move |
| Zoom | Mouse Scroll |
| Quit | ESC |

## Contributing
We welcome improvements—optimisation, modern OpenGL migration (VBO/VAO + shaders), new terrain generation techniques, physics, etc.

### 1. Fork & Clone
```
git clone https://github.com/<your-user>/OpenGL-Terrain.git
cd OpenGL-Terrain
git remote add upstream https://github.com/rejzzzz/OpenGL-Terrain.git
```

### 2. Create a Feature Branch
Use a descriptive name:
```
git checkout -b feature/orbit-damping
```

### 3. Coding Guidelines
- C++11 style (already used) – prefer modern constructs over raw C pointers where feasible.
- Keep rendering side-effects isolated inside scene or rendering modules.
- Avoid adding global state; pass references or pointers explicitly.
- Keep each new subsystem in its own folder under `include/` and `src/`.
- Use `#pragma once` in headers.
- Prefer small focused commits.

### 4. Commit Messages
Use imperative mood:
```
feat(camera): add smoothing to orbit rotation
fix(terrain): correct normal calculation for lighting
refactor(scene): extract input handling
```

### 5. Run & Test Before Pushing
```
g++ -Iinclude \
	src/main.cpp src/core/Application.cpp src/scenes/PlayScene.cpp \
	src/terrain.cpp src/objects.cpp src/utils.cpp \
	src/objects/MovableObject.cpp src/camera/Camera.cpp \
	-lGL -lGLU -lGLEW -lglfw -o terrain && ./terrain
```

### 6. Rebase (Keep History Clean)
```
git fetch upstream
git rebase upstream/main
```

### 7. Open a Pull Request
- Describe what & why (include screenshots / gifs if visual changes)
- List any follow-up tasks
- Mention performance considerations if relevant

### 8. Review Checklist (Before Opening PR)
- [ ] Builds without warnings (or warnings explained)
- [ ] No unused files / dead code
- [ ] Feature is scoped & documented in code comments where non-obvious
- [ ] Controls or user-facing changes reflected in README if needed

## Roadmap Ideas
- Replace immediate mode with VAO/VBO + GLSL shaders
- Add lighting & normals
- Procedural texture splatting
- Frustum culling / quadtree
- Config-driven terrain parameters
- Basic physics / collision
- Multiple scenes (menu, gameplay, debug)

## License
See `LICENSE` for details.

---
Feel free to open an issue if you want to discuss an idea before investing time.
