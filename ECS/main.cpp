#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <entt/entt.hpp>
#include <iostream>
#include <vector>

// Definiciones de constantes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BALL_SIZE = 15;
const int BLOCK_ROWS = 5;
const int BLOCK_COLUMNS = 10;
const int BLOCK_WIDTH = SCREEN_WIDTH / BLOCK_COLUMNS;
const int BLOCK_HEIGHT = 20;
const int MAX_FPS = 60;
const float PADDLE_SPEED = 300.0f;
const float BALL_SPEED = 200.0f;

// Definiciones de componentes
struct Position {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

struct Renderable {
    SDL_Rect rect;
    SDL_Color color;
};

struct Paddle {
    // Component específico para el paddle
};

struct Ball {
    // Component específico para la pelota
};

struct Block {
    bool destroyed;
};

int main() {
    // Inicialización de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Breakout ECS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    entt::registry registry;

    // Creación de entidades
    auto paddleEntity = registry.create();
    registry.emplace<Position>(paddleEntity, SCREEN_WIDTH / 2.0f - PADDLE_WIDTH / 2.0f, SCREEN_HEIGHT - PADDLE_HEIGHT - 10);
    registry.emplace<Velocity>(paddleEntity, 0.0f, 0.0f);
    registry.emplace<Renderable>(paddleEntity, SDL_Rect{0, 0, PADDLE_WIDTH, PADDLE_HEIGHT}, SDL_Color{0xFF, 0xFF, 0xFF, 0xFF});
    registry.emplace<Paddle>(paddleEntity);

    auto ballEntity = registry.create();
    registry.emplace<Position>(ballEntity, 110.0f, 110.0f);
    registry.emplace<Velocity>(ballEntity, BALL_SPEED, BALL_SPEED);
    registry.emplace<Renderable>(ballEntity, SDL_Rect{0, 0, BALL_SIZE, BALL_SIZE}, SDL_Color{0xFF, 0x00, 0x00, 0xFF});
    registry.emplace<Ball>(ballEntity);

    // Creación de bloques
    for (int i = 0; i < BLOCK_ROWS; ++i) {
        for (int j = 0; j < BLOCK_COLUMNS; ++j) {
            auto blockEntity = registry.create();
            registry.emplace<Position>(blockEntity, j * BLOCK_WIDTH, i * BLOCK_HEIGHT);
            registry.emplace<Renderable>(blockEntity, SDL_Rect{0, 0, BLOCK_WIDTH, BLOCK_HEIGHT}, SDL_Color{0x00, 0xFF, 0x00, 0xFF});
            registry.emplace<Block>(blockEntity, false);
        }
    }

    // Sistema de renderizado
    auto renderSystem = [&](SDL_Renderer* renderer) {
        auto view = registry.view<Position, Renderable>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& renderable = view.get<Renderable>(entity);
            renderable.rect.x = static_cast<int>(pos.x);
            renderable.rect.y = static_cast<int>(pos.y);
            SDL_SetRenderDrawColor(renderer, renderable.color.r, renderable.color.g, renderable.color.b, renderable.color.a);
            SDL_RenderFillRect(renderer, &renderable.rect);
        }
    };

    // Sistema de movimiento
    auto movementSystem = [&](float dT) {
        auto view = registry.view<Position, Velocity>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& vel = view.get<Velocity>(entity);
            pos.x += vel.vx * dT;
            pos.y += vel.vy * dT;
        }
    };

    // Sistema de entrada
    auto inputSystem = [&](float dT) {
        const Uint8* ks = SDL_GetKeyboardState(NULL);
        auto view = registry.view<Position, Paddle>();

        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            if (ks[SDL_SCANCODE_LEFT]) {
                pos.x -= PADDLE_SPEED * dT;
            }
            if (ks[SDL_SCANCODE_RIGHT]) {
                pos.x += PADDLE_SPEED * dT;
            }
            if (pos.x < 0) pos.x = 0;
            if (pos.x > SCREEN_WIDTH - PADDLE_WIDTH) pos.x = SCREEN_WIDTH - PADDLE_WIDTH;
        }
    };

    // Sistema de colisiones
    auto collisionSystem = [&]() {
        auto ballView = registry.view<Position, Velocity, Ball>();
        auto paddleView = registry.view<Position, Paddle>();
        auto blockView = registry.view<Position, Block>();

        for (auto ballEntity : ballView) {
            auto& ballPos = ballView.get<Position>(ballEntity);
            auto& ballVel = ballView.get<Velocity>(ballEntity);

            // Colisiones con bordes de pantalla
            if (ballPos.x < 0 || ballPos.x + BALL_SIZE > SCREEN_WIDTH) {
                ballVel.vx *= -1;
            }
            if (ballPos.y < 0) {
                ballVel.vy *= -1;
            }
            if (ballPos.y + BALL_SIZE > SCREEN_HEIGHT) {
                std::cout << "Game Over" << std::endl;
                exit(0);
            }

            // Colisiones con el paddle
            for (auto paddleEntity : paddleView) {
                auto& paddlePos = paddleView.get<Position>(paddleEntity);
                if (ballPos.x < paddlePos.x + PADDLE_WIDTH &&
                    ballPos.x + BALL_SIZE > paddlePos.x &&
                    ballPos.y < paddlePos.y + PADDLE_HEIGHT &&
                    ballPos.y + BALL_SIZE > paddlePos.y) {

                    ballVel.vy *= -1;
                    ballVel.vx *= 1.1f;
                    ballVel.vy *= 1.1f;
                }
            }

            // Colisiones con bloques
            for (auto blockEntity : blockView) {
                auto& blockPos = blockView.get<Position>(blockEntity);
                auto& block = blockView.get<Block>(blockEntity);
                if (!block.destroyed &&
                    ballPos.x < blockPos.x + BLOCK_WIDTH &&
                    ballPos.x + BALL_SIZE > blockPos.x &&
                    ballPos.y < blockPos.y + BLOCK_HEIGHT &&
                    ballPos.y + BALL_SIZE > blockPos.y) {

                    block.destroyed = true;
                    ballVel.vy *= -1;
                }
            }
        }
    };

    bool quit = false;
    SDL_Event e;

    Uint32 lastFrameTime = SDL_GetTicks();
    float frameDuration = (1.0f / MAX_FPS) * 1000.0f;

    while (!quit) {
        Uint32 frameStartTimestamp = SDL_GetTicks();
        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        inputSystem(dT);
        movementSystem(dT);
        collisionSystem();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);
        renderSystem(renderer);
        SDL_RenderPresent(renderer);

        Uint32 frameEndTimestamp = SDL_GetTicks();
        float actualFrameDuration = frameEndTimestamp - frameStartTimestamp;
        if (actualFrameDuration < frameDuration) {
            SDL_Delay(frameDuration - actualFrameDuration);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
