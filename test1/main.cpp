#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <cmath>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int OBJECT_SIZE = 120;
const int TRAIL_LENGTH = 10;
const int SPAWN_INTERVAL = 40;

enum ObjectType { FRUIT, BOMB, FRAGMENT };

struct GameObject {
    int x, y;
    float speed;
    int peakHeight;
    bool rising;
    ObjectType type;
    bool sliced;
    int fragmentDirection;

    GameObject(int startX, int startY, ObjectType objType, int direction = 0) {
        x = startX;
        y = startY;
        speed = (rand() % 4 + 2) * 1.5;
        peakHeight = SCREEN_HEIGHT - (speed * 40);
        rising = true;
        type = objType;
        sliced = false;
        fragmentDirection = direction;
    }

    void update() {
        if (type == FRAGMENT) {
            y += speed;
            x += fragmentDirection * 3;
        } else {
            if (rising) {
                y -= speed;
                if (y <= peakHeight) {
                    rising = false;
                }
            } else {
                y += speed;
            }
        }
    }

    bool isSliced(int prevX, int prevY, int mouseX, int mouseY) {
        int radius = OBJECT_SIZE / 4;
        float centerX = x + radius;
        float centerY = y + radius;

        float dx = mouseX - prevX;
        float dy = mouseY - prevY;
        float len = sqrt(dx * dx + dy * dy);
        if (len < 1) return false;

        float A = dy;
        float B = -dx;
        float C = dx * prevY - dy * prevX;
        float distance = fabs(A * centerX + B * centerY + C) / len;
        bool intersects = distance <= radius;

        float endDx = mouseX - centerX;
        float endDy = mouseY - centerY;
        float endDistance = sqrt(endDx * endDx + endDy * endDy);
        bool closeEnough = endDistance < OBJECT_SIZE;

        float movementX = mouseX - prevX;
        float movementY = mouseY - prevY;
        bool hasMovement = (movementX * movementX + movementY * movementY) > 25;

        return intersects && closeEnough && hasMovement;
    }
};

struct Trail {
    std::vector<SDL_Point> points;

    void addPoint(int x, int y) {
        points.push_back({x, y});
        if (points.size() > TRAIL_LENGTH) {
            points.erase(points.begin());
        }
    }
};

bool init(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Fruit Slicer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("E:/test1/novem.ttf", 24);
    if (!font) {
        std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void close(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, int score, int hp) {
    SDL_Color white = {255, 255, 255, 255};
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
    if (scoreSurface) {
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        if (scoreTexture) {
            SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
        }
        SDL_FreeSurface(scoreSurface);
    }
    std::string hpText = "HP: " + std::to_string(hp);
    SDL_Surface* hpSurface = TTF_RenderText_Solid(font, hpText.c_str(), white);
    if (hpSurface) {
        SDL_Texture* hpTexture = SDL_CreateTextureFromSurface(renderer, hpSurface);
        if (hpTexture) {
            SDL_Rect hpRect = {10, 40, hpSurface->w, hpSurface->h};
            SDL_RenderCopy(renderer, hpTexture, NULL, &hpRect);
            SDL_DestroyTexture(hpTexture);
        }
        SDL_FreeSurface(hpSurface);
    }
}

void shakeScreen(SDL_Window* window, int intensity, int duration) {
    if (!window) return;
    int originalX, originalY;
    SDL_GetWindowPosition(window, &originalX, &originalY);
    for (int i = 0; i < duration; ++i) {
        int offsetX = (rand() % (intensity * 2 + 1)) - intensity;
        int offsetY = (rand() % (intensity * 2 + 1)) - intensity;
        SDL_SetWindowPosition(window, originalX + offsetX, originalY + offsetY);
        SDL_Delay(20);
    }
    SDL_SetWindowPosition(window, originalX, originalY);
}

void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
    for (int r = radius - 1; r >= 0; r--) {
        x = r;
        y = 0;
        err = 0;
        while (x >= y) {
            SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
            SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
            if (err <= 0) {
                y += 1;
                err += 2 * y + 1;
            }
            if (err > 0) {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }
}

// Hàm menu game
void renderMenu(SDL_Renderer* renderer, TTF_Font* font, bool& inMenu, bool& quit, int mouseX, int mouseY, bool mouseDown) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    // Nút Start
    SDL_Surface* startSurface = TTF_RenderText_Solid(font, "Start", white);
    if (startSurface) {
        int startW = startSurface->w;
        int startH = startSurface->h;
        SDL_Rect startRect = {SCREEN_WIDTH / 2 - startW / 2, SCREEN_HEIGHT / 2 - 50, startW, startH};
        // Kiểm tra chuột hover
        bool startHovered = (mouseX >= startRect.x && mouseX <= startRect.x + startRect.w &&
                             mouseY >= startRect.y && mouseY <= startRect.y + startRect.h);
        if (startHovered) {
            startRect.w = static_cast<int>(startW * 0.9); // Thu nhỏ 90%
            startRect.h = static_cast<int>(startH * 0.9);
            startRect.x += (startW - startRect.w) / 2; // Căn giữa lại
            startRect.y += (startH - startRect.h) / 2;
            if (mouseDown) {
                inMenu = false; // Thoát menu, bắt đầu game
            }
        }
        SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
        if (startTexture) {
            SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
            SDL_DestroyTexture(startTexture);
        }
        SDL_FreeSurface(startSurface);
    }

    // Nút Exit
    SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "Exit", white);
    if (exitSurface) {
        int exitW = exitSurface->w;
        int exitH = exitSurface->h;
        SDL_Rect exitRect = {SCREEN_WIDTH / 2 - exitW / 2, SCREEN_HEIGHT / 2 + 50, exitW, exitH};
        // Kiểm tra chuột hover
        bool exitHovered = (mouseX >= exitRect.x && mouseX <= exitRect.x + exitRect.w &&
                            mouseY >= exitRect.y && mouseY <= exitRect.y + exitRect.h);
        if (exitHovered) {
            exitRect.w = static_cast<int>(exitW * 0.9); // Thu nhỏ 90%
            exitRect.h = static_cast<int>(exitH * 0.9);
            exitRect.x += (exitW - exitRect.w) / 2; // Căn giữa lại
            exitRect.y += (exitH - exitRect.h) / 2;
            if (mouseDown) {
                quit = true; // Thoát game
            }
        }
        SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
        if (exitTexture) {
            SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);
            SDL_DestroyTexture(exitTexture);
        }
        SDL_FreeSurface(exitSurface);
    }

    SDL_RenderPresent(renderer);
}

int main() {
    srand(time(0));
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    if (!init(window, renderer, font)) {
        return -1;
    }

    bool quit = false;
    bool inMenu = true; // Bắt đầu ở menu
    bool gameOver = false;
    SDL_Event e;
    std::vector<GameObject> objects;
    std::vector<GameObject> newObjects;
    int spawnTimer = 0;
    Trail trail;
    int score = 0;
    int hp = 5;
    bool mouseDown = false;
    int mouseX = 0, mouseY = 0, prevMouseX = 0, prevMouseY = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                mouseDown = true;
                SDL_GetMouseState(&mouseX, &mouseY);
                if (!inMenu) {
                    prevMouseX = mouseX;
                    prevMouseY = mouseY;
                }
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                mouseDown = false;
            } else if (gameOver && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                objects.clear();
                newObjects.clear();
                score = 0;
                hp = 5;
                spawnTimer = SPAWN_INTERVAL;
                trail.points.clear();
                mouseDown = false;
                gameOver = false;
                objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, FRUIT));
                if (rand() % 3 == 0) {
                    objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, BOMB));
                }
            }
        }

        SDL_GetMouseState(&mouseX, &mouseY); // Luôn cập nhật vị trí chuột

        if (inMenu) {
            renderMenu(renderer, font, inMenu, quit, mouseX, mouseY, mouseDown);
        } else if (!gameOver) {
            int currentMouseX, currentMouseY;
            SDL_GetMouseState(&currentMouseX, &currentMouseY);

            if (mouseDown && (currentMouseX != mouseX || currentMouseY != mouseY)) {
                trail.addPoint(currentMouseX, currentMouseY);
            }

            if (++spawnTimer >= SPAWN_INTERVAL) {
                spawnTimer = 0;
                objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, FRUIT));
                if (rand() % 3 == 0) {
                    objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, BOMB));
                }
            }

            newObjects.clear();
            for (auto& obj : objects) {
                if (mouseDown && obj.isSliced(prevMouseX, prevMouseY, currentMouseX, currentMouseY) && !obj.sliced) {
                    if (obj.type == BOMB) {
                        shakeScreen(window, 5, 10);
                        hp--;
                        obj.sliced = true;
                        if (hp <= 0) {
                            gameOver = true;
                        }
                        continue;
                    } else if (obj.type == FRUIT) {
                        obj.sliced = true;
                        score += 10;
                        int radius = OBJECT_SIZE / 4;
                        newObjects.push_back(GameObject(obj.x, obj.y, FRAGMENT, -1));
                        newObjects.push_back(GameObject(obj.x + radius, obj.y, FRAGMENT, 1));
                        continue;
                    }
                }
                obj.update();
                if (!(obj.type == FRAGMENT && obj.y > SCREEN_HEIGHT)) {
                    newObjects.push_back(obj);
                }
            }
            objects = newObjects;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            for (auto& obj : objects) {
                if (obj.type == FRUIT) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                else if (obj.type == BOMB) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                else if (obj.type == FRAGMENT) SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
                drawCircle(renderer, obj.x + OBJECT_SIZE / 4, obj.y + OBJECT_SIZE / 4, OBJECT_SIZE / 4);
            }
            renderText(renderer, font, score, hp);
        }

        if (gameOver && !inMenu) {
            SDL_Color red = {255, 0, 0, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(font, "Game Over! Press R to Restart", red);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 2 - surface->h / 2, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }

        if (!inMenu) {
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(16);
        int currentMouseX,currentMouseY;
        if (!gameOver && !inMenu) {
            prevMouseX = mouseX;
            prevMouseY = mouseY;
            mouseX = currentMouseX;
            mouseY = currentMouseY;
        }
    }
    close(window, renderer, font);
    return 0;
}
