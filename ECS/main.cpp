#define SDL_MAIN_HANDLED
#include <SDL.h>
//#include <SDL_ttf.h>
#include <SDL_keyboard.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int MAX_FPS = 60;
const int BALL_SPEED = 120;
const int BALL_SIZE = 13;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int PADDLE_SPEED = 200;
const int BLOCK_ROWS = 5;
const int BLOCK_COLUMNS = 10;
const int BLOCK_WIDTH = SCREEN_WIDTH / BLOCK_COLUMNS;
const int BLOCK_HEIGHT = 20;

struct Rect {
    SDL_Rect rect = {0, 0, BALL_SIZE, BALL_SIZE};
    float vx = BALL_SPEED;
    float vy = BALL_SPEED;
    SDL_Color color = {0xFF, 0x00, 0x00, 0xFF}; // Rojo
};

struct Block {
    SDL_Rect rect;
    bool destroyed = false;
};

Rect ball = {{110, 110, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED, {0xFF, 0x00, 0x00, 0xFF}};//posicion inicial de la pelota
SDL_Rect paddle = {SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2, SCREEN_HEIGHT - PADDLE_HEIGHT - 10, PADDLE_WIDTH, PADDLE_HEIGHT};
std::vector<Block> blocks;

void createBlocks() {
    for (int i = 0; i < BLOCK_ROWS; ++i) {
        for (int j = 0; j < BLOCK_COLUMNS; ++j) {
            blocks.push_back({{j * BLOCK_WIDTH, i * BLOCK_HEIGHT, BLOCK_WIDTH, BLOCK_HEIGHT}});
        }
    }
}

void renderRect(SDL_Renderer* renderer, Rect& rect) {
    SDL_SetRenderDrawColor(renderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
    SDL_RenderFillRect(renderer, &rect.rect);
}

void renderPaddle(SDL_Renderer* renderer, SDL_Rect& paddle) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // Blanco
    SDL_RenderFillRect(renderer, &paddle);
}

void renderBlocks(SDL_Renderer* renderer) {
    for (const auto& block : blocks) {
        if (!block.destroyed) {
            // Dibujar borde negro
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Negro
            SDL_Rect borderRect = {block.rect.x - 1, block.rect.y - 1, block.rect.w + 2, block.rect.h + 2};
            SDL_RenderFillRect(renderer, &borderRect);

            // Dibujar bloque verde
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF); // Verde
            SDL_RenderFillRect(renderer, &block.rect);
        }
    }
}

void update(float dT, bool& gameOver, bool& youWin) {
    // Rebote en los bordes de la pantalla
    if (ball.rect.x < 0 || ball.rect.x + ball.rect.w > SCREEN_WIDTH) {
        ball.vx *= -1;
    }
    if (ball.rect.y < 0) {
        ball.vy *= -1;
    }

    // Si la pelota toca la parte inferior de la pantalla
    if (ball.rect.y + ball.rect.h > SCREEN_HEIGHT) {
        gameOver = true;
        std::cout << "Game Over" << std::endl;
    }

    // Rebote con el paddle
    if (SDL_HasIntersection(&ball.rect, &paddle)) {
        // Calcular el punto de impacto relativo en el paddle
        float relativeIntersectX = (ball.rect.x + (BALL_SIZE / 2)) - (paddle.x + (paddle.w / 2));
        float normalizedRelativeIntersectionX = relativeIntersectX / (paddle.w / 2);
        float bounceAngle = normalizedRelativeIntersectionX * (M_PI / 4); // Ángulo máximo de 45 grados

        // Ajustar velocidades de la pelota
        ball.vx = BALL_SPEED * normalizedRelativeIntersectionX;
        ball.vy = -BALL_SPEED * std::cos(bounceAngle);

        // Aumentar la velocidad de la pelota
        ball.vx *= 1.1f;
        ball.vy *= 1.1f;

        ball.rect.y = paddle.y - ball.rect.h; // La pelota se mueva hacia arriba después de rebotar
    }

    // Colisión con bloques
    bool allBlocksDestroyed = true;
    for (auto& block : blocks) {
        if (!block.destroyed && SDL_HasIntersection(&ball.rect, &block.rect)) {
            block.destroyed = true;
            ball.vy *= -1;
            break;
        }
        if (!block.destroyed) {
            allBlocksDestroyed = false;
        }
    }

    if (allBlocksDestroyed) {
        youWin = true;
        std::cout << "You Win!" << std::endl;
    }

    ball.rect.x += ball.vx * dT;
    ball.rect.y += ball.vy * dT;
}

void handleInput(float dT) {
    const Uint8* ks = SDL_GetKeyboardState(NULL);

    if (ks[SDL_SCANCODE_LEFT]) {
        paddle.x -= PADDLE_SPEED * dT;
    }
    if (ks[SDL_SCANCODE_RIGHT]) {
        paddle.x += PADDLE_SPEED * dT;
    }

    if (paddle.x < 0) paddle.x = 0;
    if (paddle.x > SCREEN_WIDTH - PADDLE_WIDTH) paddle.x = SCREEN_WIDTH - PADDLE_WIDTH;
}

int main() {
    SDL_SetMainReady(); // Esto se llama para inicializar correctamente SDL en entornos no predeterminados
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Bouncing Ball with Paddle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    createBlocks();

    bool quit = false;
    bool gameOver = false;
    bool youWin = false;
    SDL_Event e;

    Uint32 frameStartTimestamp, frameEndTimestamp, lastFrameTime = SDL_GetTicks(), lastUpdateTime = 0;
    float frameDuration = (1.0f / MAX_FPS) * 1000.0f;
    int FPS = MAX_FPS;

    while (!quit) {
        frameStartTimestamp = SDL_GetTicks();

        // delta time
        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        // poll events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // handle input
        if (!gameOver && !youWin) {
            handleInput(dT);
        }

        // update
        if (!gameOver && !youWin) {
            update(dT, gameOver, youWin);
        }

        // render
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        renderRect(renderer, ball);
        renderPaddle(renderer, paddle);
        renderBlocks(renderer);

        SDL_RenderPresent(renderer);

        frameEndTimestamp = SDL_GetTicks();
        float actualFrameDuration = frameEndTimestamp - frameStartTimestamp;

        if (actualFrameDuration < frameDuration) {
            SDL_Delay(frameDuration - actualFrameDuration);
        }

        // calculo de fps
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastUpdateTime > 1000) {
            FPS = static_cast<int>((1.0f / actualFrameDuration) * 1000.0f);
            SDL_SetWindowTitle(window, ("FPS: " + std::to_string(FPS)).c_str());
            lastUpdateTime = currentTime;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}