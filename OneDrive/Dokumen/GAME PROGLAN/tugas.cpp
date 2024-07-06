#include "raylib.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>

#define MAX_TUBES 100
#define FLOPPY_RADIUS 20
#define TUBES_WIDTH 80
#define GRAVITY 0.9f // Gravity constant

const int screenWidth = 800;
const int screenHeight = 450;

class Floppy {
public:
    Vector2 position;
    int radius;
    Texture2D texture;

    Floppy();
    ~Floppy();

    void Init();
    void Update();
    void Draw();
};

class Tube {
public:
    Rectangle rec;
    Color color;
    bool active;

    Tube();
    void Init(float x, float y);
    void Update();
    void Draw();
};

class Game {
private:
    static const int scoreBoxWidth = 150;
    static const int scoreBoxHeight = 60;

    bool gameOver;
    bool pause;
    int score;
    int hiScore;
    bool gameStarted;

    Floppy floppy;
    Tube tubes[MAX_TUBES * 2];
    Vector2 tubesPos[MAX_TUBES];
    float tubesSpeedX;
    bool superfx;

    Texture2D background;
    Texture2D title;

    std::vector<int> highScores;

    void LoadHighScores();
    void SaveHighScores();
    void UpdateHighScores();
    void DrawHighScores();

public:
    Game();
    ~Game();

    void Init();
    void Update();
    void Draw();
    void DrawScore();
    void DrawTitle();
};

Floppy::Floppy() : radius(FLOPPY_RADIUS) {}

Floppy::~Floppy() {
    UnloadTexture(texture);
}

void Floppy::Init() {
    texture = LoadTexture("floppy.png");
    texture.width = 80; // Adjust dimensions to fit the floppy texture size
    texture.height = 40; // Adjust dimensions to fit the floppy texture size
    position = {80, screenHeight / 2 - radius};
}

void Floppy::Update() {
    position.y += GRAVITY;

    if (IsKeyDown(KEY_W))
        position.y -= 3;
    if (IsKeyDown(KEY_S))
        position.y += 3;
    if (IsKeyDown(KEY_A))
        position.x -= 3;
    if (IsKeyDown(KEY_D))
        position.x += 3;

    if (position.y >= screenHeight - radius) {
        position.y = screenHeight - radius;
    }
}

void Floppy::Draw() {
    DrawTexture(texture, position.x - radius, position.y - radius, WHITE);
}

Tube::Tube() : active(true), color(GREEN) {}

void Tube::Init(float x, float y) {
    rec = {x, y, TUBES_WIDTH, 255};
    active = true;
}

void Tube::Update() {
    rec.x -= 2; // Speed is managed in the game class
}

void Tube::Draw() {
    DrawRectangleRec(rec, color);
}

Game::Game() : gameOver(false), pause(false), score(0), hiScore(0), gameStarted(false), tubesSpeedX(2), superfx(false) {
    LoadHighScores();
}

Game::~Game() {
    UnloadTexture(background);
    UnloadTexture(title);
    SaveHighScores();
}

void Game::Init() {
    floppy.Init();

    background = LoadTexture("floppy background.png");
    title = LoadTexture("THE ADVENTURE OF FLOPPY.png");

    for (int i = 0; i < MAX_TUBES; i++) {
        tubesPos[i] = {400 + 280 * i, -GetRandomValue(0, 120)};
    }

    for (int i = 0; i < MAX_TUBES * 2; i += 2) {
        tubes[i].Init(tubesPos[i / 2].x, tubesPos[i / 2].y);
        tubes[i + 1].Init(tubesPos[i / 2].x, 600 + tubesPos[i / 2].y - 255);
    }

    gameOver = false;
    pause = false;
    score = 0; // Reset score
    tubesSpeedX = 2;
    superfx = false;
    gameStarted = false; // Game starts in a non-started state

    SetTargetFPS(60);
}

void Game::LoadHighScores() {
    std::ifstream file("highscores.txt");
    highScores.clear();
    int score;
    while (file >> score) {
        highScores.push_back(score);
    }
    file.close();
    std::sort(highScores.begin(), highScores.end(), std::greater<int>());
    while (highScores.size() > 5) {
        highScores.pop_back();
    }
}

void Game::SaveHighScores() {
    std::ofstream file("highscores.txt");
    for (int score : highScores) {
        file << score << std::endl;
    }
    file.close();
}

void Game::UpdateHighScores() {
    highScores.push_back(score);
    std::sort(highScores.begin(), highScores.end(), std::greater<int>());
    if (highScores.size() > 5) {
        highScores.pop_back();
    }
    if (score > hiScore) {
        hiScore = score;
    }
}

void Game::DrawHighScores() {
    int yOffset = screenHeight / 2 - 100;
    DrawText("Your Highest Score:", screenWidth / 2 - MeasureText("Your Highest Score:", 20) / 2, yOffset, 20, BLACK);
    DrawText(TextFormat("%d", hiScore), screenWidth / 2 - MeasureText(TextFormat("%d", hiScore), 20) / 2, yOffset + 30, 20, BLACK);
    DrawText("Your Score:", screenWidth / 2 - MeasureText("Your Score:", 20) / 2, yOffset + 70, 20, BLACK);
    DrawText(TextFormat("%d", score), screenWidth / 2 - MeasureText(TextFormat("%d", score), 20) / 2, yOffset + 100, 20, BLACK);
}

void Game::Update() {
    if (!gameOver && gameStarted) {
        if (IsKeyPressed(KEY_P))
            pause = !pause;

        if (!pause) {
            for (int i = 0; i < MAX_TUBES; i++)
                tubesPos[i].x -= tubesSpeedX;

            for (int i = 0; i < MAX_TUBES * 2; i += 2) {
                tubes[i].rec.x = tubesPos[i / 2].x;
                tubes[i + 1].rec.x = tubesPos[i / 2].x;
            }

            floppy.Update();

            for (int i = 0; i < MAX_TUBES * 2; i++) {
                if (CheckCollisionCircleRec(floppy.position, floppy.radius, tubes[i].rec)) {
                    gameOver = true;
                    pause = false;
                    UpdateHighScores();
                } else if ((tubesPos[i / 2].x < floppy.position.x) && tubes[i / 2].active && !gameOver) {
                    score += 100;
                    tubes[i / 2].active = false;
                    superfx = true;
                    if (score > hiScore)
                        hiScore = score;

                    if (score >= 2500 && score < 4500) {
                        tubesSpeedX = 3.5f;
                    } else if (score >= 4500) {
                        tubesSpeedX = 4.0f;
                    } else if (score >= 5500) {
                        tubesSpeedX = 6.0f;
                    }
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            Init();
            gameOver = false;
            gameStarted = true; // Start game when enter is pressed after game over
        }
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (!gameStarted) {
        DrawTexture(title, screenWidth / 2 - title.width / 2, screenHeight / 4 - title.height / 4, WHITE);
        DrawText("PRESS [ENTER] TO START", screenWidth / 2 - MeasureText("PRESS [ENTER] TO START", 20) / 2, screenHeight / 2 + title.height / 2, 20, WHITE);
    } else {
        DrawTexture(background, 0, 0, WHITE);

        if (!gameOver) {
            for (int i = 0; i < MAX_TUBES; i++) {
                tubes[i * 2].Draw();
                tubes[i * 2 + 1].Draw();
            }

            if (superfx) {
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
                superfx = false;
            }

            if (pause)
                DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, BLACK);

            floppy.Draw();
        } else {
            DrawHighScores();
            DrawText("PRESS [ENTER] TO PLAY AGAIN", screenWidth / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, screenHeight / 2 + 150, 20, GRAY);
        }

        DrawScore();
    }

    EndDrawing();
}

void Game::DrawScore() {
    DrawText(TextFormat("%04i", score), 10, 10, 30, WHITE);
}

void Game::DrawTitle() {
    DrawText("FLOPPY", screenWidth / 2 - MeasureText("FLOPPY", 40) / 2, screenHeight / 4 - 40, 40, GRAY);
}

int main() {
    InitWindow(screenWidth, screenHeight, "Floppy Game");

    Game game;
    game.Init();

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    CloseWindow();

    return 0;
}
