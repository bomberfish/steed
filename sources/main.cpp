#include "physac.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string>
#include <vector>

#define SCREEN_WIDTH (960)
#define SCREEN_HEIGHT (480)
#define WINDOW_TITLE "Steed"

// Player and horse size configuration
#define PLAYER_SIZE 12
#define HORSE_SIZE 12

#include <cmath>
#include <math.h>

#define SCREEN_WIDTH (960)
#define SCREEN_HEIGHT (480)
#define WINDOW_TITLE "Steed"

// Player and horse size configuration
#define PLAYER_SIZE 12
#define HORSE_SIZE 12

struct MovementTuning {
    float maxSpeed;
    float groundAcceleration;
    float airAcceleration;
    float groundDeceleration;
    float airDeceleration;
    float jumpVelocity;
};

// Movement tuning keeps rider and on-foot values centralized.
static const MovementTuning PLAYER_TUNING = {0.2f,  5.0f,  7.5f,
                                             10.0f, 15.0f, 0.3f};
static const MovementTuning HORSE_TUNING = {0.3f, 10.0f, 12.5f,
                                            7.5f, 10.0f, 0.8f};

static std::vector<PhysicsBody> levelSurfaces;

static const float GROUND_DITHER_DENSITY = 0.35f;
static const float HORSE_DITHER_DENSITY = 0.5f;

static float Clamp01(float value) {
    if (value < 0.0f)
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    return value;
}

static void DrawCheckerboardRectangleV(Vector2 position, Vector2 size,
                                       Color color, float density) {
    const float clampedDensity = Clamp01(density);
    if (clampedDensity <= 0.0f || size.x <= 0.0f || size.y <= 0.0f)
        return;

    if (clampedDensity >= 1.0f) {
        ::DrawRectangleV(position, size, color);
        return;
    }

    static const int PATTERN_SIZE = 4;
    static const unsigned char BAYER_MATRIX[PATTERN_SIZE][PATTERN_SIZE] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5},
    };

    const int maxThreshold = PATTERN_SIZE * PATTERN_SIZE;
    int threshold = (int)floorf(clampedDensity * (float)maxThreshold);
    if (threshold <= 0)
        return;
    if (threshold >= maxThreshold) {
        ::DrawRectangleV(position, size, color);
        return;
    }

    const float left = position.x;
    const float top = position.y;
    const float right = left + size.x;
    const float bottom = top + size.y;

    const int minX = (int)floorf(left);
    const int minY = (int)floorf(top);
    const int maxX = (int)ceilf(right);
    const int maxY = (int)ceilf(bottom);

    for (int y = minY; y < maxY; ++y) {
        const float py = (float)y + 0.5f;
        if (py < top || py >= bottom)
            continue;
        const int localY = (y - minY) % PATTERN_SIZE;
        for (int x = minX; x < maxX; ++x) {
            const float px = (float)x + 0.5f;
            if (px < left || px >= right)
                continue;
            const int localX = (x - minX) % PATTERN_SIZE;
            if (BAYER_MATRIX[localY][localX] < threshold) {
                DrawPixel(x, y, color);
            }
        }
    }
}

static void DrawCheckerboardRectangleV(Vector2 position, Vector2 size,
                                       Color color) {
    DrawCheckerboardRectangleV(position, size, color,
                               Clamp01(color.a / 255.0f));
}
void ConstructLevel(int level) {
    for (PhysicsBody body : levelSurfaces) {
        if (body) {
            DestroyPhysicsBody(body);
        }
    }
    levelSurfaces.clear();

    switch (level) {
    case 2:
        levelSurfaces = {
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10}, SCREEN_WIDTH,
                20, 10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 3, SCREEN_HEIGHT - 40}, 140, 40, 10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 2 + 120, SCREEN_HEIGHT - 70}, 120, 60,
                10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH - 180, SCREEN_HEIGHT - 45}, 160, 30,
                10)};
        break;
    case 3:
        levelSurfaces = {CreatePhysicsBodyRectangle(
                             (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10},
                             SCREEN_WIDTH, 20, 10),
                         CreatePhysicsBodyRectangle(
                             (Vector2){SCREEN_WIDTH / 3, SCREEN_HEIGHT - 140},
                             50, SCREEN_HEIGHT - 240, 10)};

        break;
    case 4:
        levelSurfaces = {CreatePhysicsBodyRectangle(
                             (Vector2){160, SCREEN_HEIGHT - 10}, 320, 20, 10),
                         CreatePhysicsBodyRectangle(
                             (Vector2){600, SCREEN_HEIGHT - 10}, 200, 20, 10),
                         CreatePhysicsBodyRectangle(
                             (Vector2){440, SCREEN_HEIGHT - 100}, 120, 20, 10),
                         CreatePhysicsBodyRectangle(
                             (Vector2){860, SCREEN_HEIGHT - 50}, 160, 20, 10)};
        break;
    case 5:
        levelSurfaces = {
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 4 + 32.5, SCREEN_HEIGHT - 10}, SCREEN_WIDTH / 2 + 65,
                20, 10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT - 80}, 200, 20,
                10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 2 + 130, SCREEN_HEIGHT - 150}, 100, 20,
                10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH - 140, SCREEN_HEIGHT - 80}, 200, 20, 10),
            CreatePhysicsBodyRectangle(
                (Vector2){SCREEN_WIDTH / 2 + 40, SCREEN_HEIGHT - 195}, 50, 350,
                10)};
        break;
    case 6:
        levelSurfaces = {
            CreatePhysicsBodyRectangle((Vector2){120, SCREEN_HEIGHT - 40}, 200,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){400, SCREEN_HEIGHT - 100}, 140,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){650, SCREEN_HEIGHT - 210}, 140,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){860, SCREEN_HEIGHT - 70}, 180,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){500, SCREEN_HEIGHT - 250}, 80,
                                       20, 10)};
        break;
    case 7:
        levelSurfaces = {
            CreatePhysicsBodyRectangle((Vector2){100, SCREEN_HEIGHT - 60}, 160,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){340, SCREEN_HEIGHT - 120}, 120,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){540, SCREEN_HEIGHT - 240}, 100,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){720, SCREEN_HEIGHT - 140}, 140,
                                       20, 10),
            CreatePhysicsBodyRectangle((Vector2){900, SCREEN_HEIGHT - 90}, 120,
                                       20, 10)};
        break;
    default:
        // add surfaces for level 1
        levelSurfaces = {CreatePhysicsBodyRectangle(
            (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10}, SCREEN_WIDTH, 20,
            10)};
        break;
    }

    for (PhysicsBody body : levelSurfaces) {
        if (body) {
            body->enabled = false; // Static bodies
        }
    }
}

void Hint(int level) {
    std::string thanks = "That's all for now. Thanks for playing!\nFrom "
                         "Canada with love.\n\n";
#if defined(EMSCRIPTEN)
    thanks += "Built with raylib in C++, compiled to WebAssembly with "
              "Emscripten.";
#else
    thanks += "Built with raylib in C++.";
#endif

    thanks += "\n\nCreated by bomberfish for Hack Club Daydream 2025.";
    switch (level) {
    case 1:
        DrawText("You are the black square.\nThe grey square underneath you is your horse, "
                 "Jake.\n\nUse the left/right arrow keys to move.\nPress R to "
                 "reset if you get stuck.",
                 20, 20, 20, BLACK);
        break;
    case 2:
        DrawText("Both you and Jake can jump. Press C to jump.", 20, 20, 20,
                 BLACK);
        break;
    case 3:
        DrawText("Oh no! That wall looks too high...\nLooks like you'll have "
                 "to sacrifice Jake.\nPress X (the one next to C, not H) in "
                 "mid-air to jump off and gain some extra height.",
                 20, 40, 20, BLACK);
        break;
    case 4:
        DrawText("Don't worry, you start with Jake in every level.", 20, 20, 20,
                 BLACK);
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    default:
        DrawText(thanks.c_str(), 20, 20, 20, BLACK);
        break;
    }
}

PhysicsBody player, horse;

bool onHorse = true;

void SetInitialPositions() {
    onHorse = true;

    if (player == NULL) {
        player = CreatePhysicsBodyRectangle(
            (Vector2){30, SCREEN_HEIGHT / 2 - 20}, PLAYER_SIZE, PLAYER_SIZE, 1);
        player->freezeOrient = true; // No rotation
    }

    player->enabled = false;
    player->position = (Vector2){30, SCREEN_HEIGHT / 2 - 20};
    player->velocity = (Vector2){0, 0};

    if (horse == NULL) {
        horse = CreatePhysicsBodyRectangle((Vector2){30, SCREEN_HEIGHT / 2},
                                           HORSE_SIZE, HORSE_SIZE, 1);
        horse->freezeOrient = true; // No rotation
    }

    horse->enabled = true;
    horse->position = (Vector2){30, SCREEN_HEIGHT / 2};
    horse->velocity = (Vector2){0, 0};
    if (horse == NULL) {
        horse = CreatePhysicsBodyRectangle((Vector2){30, SCREEN_HEIGHT / 2},
                                           HORSE_SIZE, HORSE_SIZE, 1);
        horse->freezeOrient = true; // No rotation
        horse->enabled = true;
    }
}



int main(int argc, char *argv[]) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    // SetTargetFPS(60);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitAudioDevice();

    Sound jump = LoadSound("jump.wav");
    Sound annihilator = LoadSound("horse.wav");
    Sound death = LoadSound("biteof87.wav");

    int level = 5;

    if (argc > 1) {
        int argLevel = atoi(argv[1]);
        if (argLevel >= 1) {
            level = argLevel;
        }
    }

    SetInitialPositions();
    ConstructLevel(level);

    InitPhysics();
    SetPhysicsGravity(0, 1);

    bool debug = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        UpdatePhysics();

        float delta = GetFrameTime();

        ClearBackground(RAYWHITE);
        Hint(level);

        if (IsKeyPressed(KEY_D)) {
            debug = !debug;
        }

        if (debug) {
            DrawFPS(10, 10);
        }

        PhysicsBody activeBody = onHorse ? horse : player;

        if (IsKeyPressed(KEY_R)) {
            printf("Reset!\n");
            SetInitialPositions();
        } else if (activeBody->position.y >= SCREEN_HEIGHT) {
            SetInitialPositions();
            printf("You died!\n");
            PlaySound(death);
        } else if (activeBody->position.x >=
                   SCREEN_WIDTH - (onHorse ? HORSE_SIZE : PLAYER_SIZE)) {
            level++;
            ConstructLevel(level);
            SetInitialPositions();
            printf("Level %d\n", level);
        } else if (activeBody->position.x <= 0) {
            level--;
            if (level < 1)
                level = 1;
            ConstructLevel(level);
            SetInitialPositions();
            printf("Level %d\n", level);
        } else {
            if (activeBody != NULL) {
                bool grounded = activeBody->isGrounded;
                if (!grounded) {
                    // Fallback for static geometry before Physac updates the
                    // contact flag
                    float bodyBottom =
                        activeBody->position.y +
                        (onHorse ? HORSE_SIZE : PLAYER_SIZE) / 2.0f;
                    for (PhysicsBody surface : levelSurfaces) {
                        if (!surface)
                            continue;
                        float surfaceLeft = surface->position.x;
                        float surfaceRight = surface->position.x;
                        float surfaceTop = surface->position.y;
                        unsigned int vertexCount =
                            surface->shape.vertexData.vertexCount;
                        for (unsigned int vertexIndex = 0;
                             vertexIndex < vertexCount; ++vertexIndex) {
                            Vector2 vertex =
                                GetPhysicsShapeVertex(surface, vertexIndex);
                            if (vertex.x < surfaceLeft)
                                surfaceLeft = vertex.x;
                            if (vertex.x > surfaceRight)
                                surfaceRight = vertex.x;
                            if (vertex.y < surfaceTop)
                                surfaceTop = vertex.y;
                        }
                        if (bodyBottom >= surfaceTop - 1.0f &&
                            bodyBottom <= surfaceTop + 1.0f &&
                            activeBody->position.x >= surfaceLeft &&
                            activeBody->position.x <= surfaceRight) {
                            grounded = true;
                            break;
                        }
                    }
                }

                const MovementTuning *tuning =
                    onHorse ? &HORSE_TUNING : &PLAYER_TUNING;
                int moveInput = 0;
                if (IsKeyDown(KEY_RIGHT))
                    moveInput += 1;
                if (IsKeyDown(KEY_LEFT))
                    moveInput -= 1;

                if (moveInput != 0) {
                    float accel = grounded ? tuning->groundAcceleration
                                           : tuning->airAcceleration;
                    activeBody->velocity.x += accel * delta * (float)moveInput;
                } else {
                    float decel = grounded ? tuning->groundDeceleration
                                           : tuning->airDeceleration;
                    float decelStep = decel * delta;
                    if (activeBody->velocity.x > decelStep) {
                        activeBody->velocity.x -= decelStep;
                    } else if (activeBody->velocity.x < -decelStep) {
                        activeBody->velocity.x += decelStep;
                    } else {
                        activeBody->velocity.x = 0.0f;
                    }
                }

                float clampedSpeed = tuning->maxSpeed;
                if (activeBody->velocity.x > clampedSpeed)
                    activeBody->velocity.x = clampedSpeed;
                if (activeBody->velocity.x < -clampedSpeed)
                    activeBody->velocity.x = -clampedSpeed;

                if (level > 1 && IsKeyPressed(KEY_C) && grounded) {
                    player->enabled = true;
                    activeBody->velocity.y = -tuning->jumpVelocity;
                    printf("Jump!\n");
                    PlaySound(jump);
                }
            }

            if (level > 2 && IsKeyPressed(KEY_X)) {
                // dismount, jump off horse and then destroy horse
                if (onHorse) {
                    onHorse = false;
                    player->enabled = true;
                    player->velocity = (Vector2){
                        horse->velocity.x, -0.8f * HORSE_TUNING.jumpVelocity};
                    DestroyPhysicsBody(horse);
                    horse = NULL;
                    printf("!!! THE HORSE HAS BEEN DESTROYED !!!\n");
                    PlaySound(annihilator);
                }
            }

            if (onHorse) {
                // make player follow horse
                player->position =
                    Vector2Subtract(horse->position, (Vector2){0, HORSE_SIZE});
            }
        }

        // printf("Player position: x=%f, y=%f\n", player->position.x,
        //        player->position.y);

        // draw ground
        for (PhysicsBody surface : levelSurfaces) {
            if (!surface)
                continue;
            unsigned int vertexCount = surface->shape.vertexData.vertexCount;
            if (vertexCount == 0)
                continue;
            Vector2 firstVertex = GetPhysicsShapeVertex(surface, 0);
            float minX = firstVertex.x;
            float maxX = firstVertex.x;
            float minY = firstVertex.y;
            float maxY = firstVertex.y;
            for (unsigned int vertexIndex = 1; vertexIndex < vertexCount;
                 ++vertexIndex) {
                Vector2 vertex = GetPhysicsShapeVertex(surface, vertexIndex);
                if (vertex.x < minX)
                    minX = vertex.x;
                if (vertex.x > maxX)
                    maxX = vertex.x;
                if (vertex.y < minY)
                    minY = vertex.y;
                if (vertex.y > maxY)
                    maxY = vertex.y;
            }
            DrawCheckerboardRectangleV((Vector2){minX, minY},
                                       (Vector2){maxX - minX, maxY - minY},
                                       BLACK, GROUND_DITHER_DENSITY);
        }
        if (horse != NULL) {
            DrawCheckerboardRectangleV((Vector2){horse->position.x - HORSE_SIZE / 2,
                                                  horse->position.y - HORSE_SIZE / 2},
                                       (Vector2){HORSE_SIZE, HORSE_SIZE},
                                       BLACK, HORSE_DITHER_DENSITY);
        }
        DrawRectangle(player->position.x - PLAYER_SIZE / 2,
                      player->position.y - PLAYER_SIZE / 2, PLAYER_SIZE,
                      PLAYER_SIZE, BLACK);

        if (debug) {
            // Draw created physics bodies
            int bodiesCount = GetPhysicsBodiesCount();
            for (int i = 0; i < bodiesCount; i++) {
                PhysicsBody body = GetPhysicsBody(i);

                if (body != NULL) {
                    int vertexCount = GetPhysicsShapeVerticesCount(i);
                    for (int j = 0; j < vertexCount; j++) {
                        // Get physics bodies shape vertices to draw lines
                        // Note: GetPhysicsShapeVertex() already calculates
                        // rotation transformations
                        Vector2 vertexA = GetPhysicsShapeVertex(body, j);

                        int jj = (((j + 1) < vertexCount)
                                      ? (j + 1)
                                      : 0); // Get next vertex or first to close
                                            // the shape
                        Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                        DrawLineV(
                            vertexA, vertexB,
                            GREEN); // Draw a line between two vertex positions
                    }
                }
            }
        }
        EndDrawing();
    }
    
    UnloadSound(jump);
    UnloadSound(annihilator);
    ClosePhysics();
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
