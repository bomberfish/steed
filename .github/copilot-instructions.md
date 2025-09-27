## Project Overview
- `Steed` builds a single Raylib executable that simulates a rider/horse prototype; CMake `FetchContent` pulls the latest `raylib` sources during configure.
- Game logic lives in `sources/main.cpp`; Physac is vendored as `sources/physac.h` with `PHYSAC_IMPLEMENTATION` defined in `CMakeLists.txt`.
- Source files placed under `sources/*.cpp` are auto-included by the `file(GLOB_RECURSE ...)` rule, so new modules only need to be dropped into that folder.
- Assets load through the `ASSETS_PATH` compile definition; concatenate literals (`ASSETS_PATH "foo.png"`) and keep files under `assets/`.

## Build & Run
- Configure normally with CMake; this downloads raylib and configures Physac:

```fish
cmake -S . -B build
cmake --build build --target Steed
cd build
./Steed
```
- A successful build copies `compile_commands.json` into the repo root for tooling; rerun the build after editing CMake options.
- `SetConfigFlags(FLAG_VSYNC_HINT)` is set so frame pacing relies on vsync; avoid uncapped loops unless you handle physics dt manually.

## Gameplay & Physics Loop
- Each frame goes `BeginDrawing()` → `UpdatePhysics()` → input/velocity updates → rendering → `EndDrawing()`; call `UpdatePhysics()` only once per frame before mutating body state.
- `PhysicsBody` values are pointers from Physac; static colliders are created via `CreatePhysicsBodyRectangle()` and immediately `->enabled = false` to disable simulation while preserving collisions.
- Movement settings are centralized in the `MovementTuning` structs (`PLAYER_TUNING`, `HORSE_TUNING`); extend behavior by adjusting those constants or introducing new presets.
- `ConstructLevel()` destroys old bodies with `DestroyPhysicsBody()` before repopulating; maintain that pattern when adding new level geometry to prevent leaked bodies.
- Player vs horse control switches via the `onHorse` flag; mounted state mirrors the player position relative to the horse and shares velocity on dismount (`KEY_X`).

## Debugging & Extensions
- Physics debug rendering iterates `GetPhysicsBodiesCount()` and `DrawLineV()` to outline shapes; keep or expand that section when adding new bodies for quick visual checks.
- Runtime reset (`KEY_R`) reinitializes both bodies; when introducing new physics entities, wire them into the same reset pathway to avoid stale pointers.
- For different asset roots (e.g., packaging), adjust `ASSETS_PATH` in `CMakeLists.txt`; remember absolute paths are baked at configure time.

## Web Export
- Web builds are configured through Emscripten: `emcmake cmake -S . -B build-web` then `cmake --build build-web`; the executable becomes `Steed.html`.
- The emscripten target uses `webport/index.html` as the shell and preloads `assets/`; serve the generated folder (`python -m http.server`) to test in-browser input and timing.
