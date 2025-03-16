#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

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
    
    bool isSliced(int mouseX, int mouseY) {
        return (mouseX >= x && mouseX <= x + OBJECT_SIZE && mouseY >= y && mouseY <= y + OBJECT_SIZE);
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
        return false;
    }
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }
    
    window = SDL_CreateWindow("Fruit Slicer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        return false;
    }
    font = TTF_OpenFont("E:/test1/novem.ttf", 24);
if (!font) {
    std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    return false;
}

if (font == nullptr) {
    std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    return -1;
}

    if (!font) {
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

void renderText(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_Color white = {255, 255, 255, 255};
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
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


int main(int argc, char* argv[]) {
    srand(time(0));
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    if (!init(window, renderer, font)) {
return -1;
    }
    int windowX, windowY;
SDL_GetWindowPosition(window, &windowX, &windowY);

    
    bool quit = false;
    SDL_Event e;
    std::vector<GameObject> objects;
    int spawnTimer = 0;
    Trail trail;
    int score = 0;
    
    bool mouseDown = false;
    
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                mouseDown = true;
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                mouseDown = false;
            }
        }
        
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseDown) {
            trail.addPoint(mouseX, mouseY);
        }
        
        if (++spawnTimer >= SPAWN_INTERVAL) {
            spawnTimer = 0;
            objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, FRUIT));
            if (rand() % 3 == 0) {
                objects.push_back(GameObject(rand() % (SCREEN_WIDTH - OBJECT_SIZE), SCREEN_HEIGHT, BOMB));
            }
        }
        
        std::vector<GameObject> newObjects;
        for (auto& obj : objects) {
            if (mouseDown && obj.isSliced(mouseX, mouseY) && !obj.sliced) {
                if (obj.type == BOMB) {
                    shakeScreen(window, 5, 10);
                    SDL_Color red = {255, 0, 0, 255}; // Màu đỏ
                    SDL_Surface* surface = TTF_RenderText_Solid(font, "Game Over!", red);
                    
                    if (surface == nullptr) {
                        std::cout << "Failed to create text surface: " << TTF_GetError() << std::endl;
                        quit = true;
                        return -1;
                    }
                
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    
                    if (texture == nullptr) {
                        std::cout << "Failed to create texture: " << SDL_GetError() << std::endl;
                        SDL_FreeSurface(surface);
                        quit = true;
                        return -1;
                    }
                
                    SDL_Rect textRect = {SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 2 - surface->h / 2, surface->w, surface->h};
                
                    SDL_FreeSurface(surface);  // Chỉ giải phóng sau khi tạo texture thành công
                
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_RenderPresent(renderer);
                
                    SDL_Delay(2000); 
                
                    SDL_DestroyTexture(texture);
                    quit = true;
                }
                
                 else if (obj.type == FRUIT) {
                    obj.sliced = true;
                    score += 10; // Tăng điểm
                    int halfSize = OBJECT_SIZE / 2;
                    newObjects.push_back(GameObject(obj.x, obj.y, FRAGMENT, -1));
                    newObjects.push_back(GameObject(obj.x + halfSize, obj.y, FRAGMENT, 1));
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
            if (obj.type == FRUIT) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else if (obj.type == BOMB) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            } else if (obj.type == FRAGMENT) {
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            }
            SDL_Rect rect = {obj.x, obj.y, OBJECT_SIZE / 2, OBJECT_SIZE / 2};
            SDL_RenderFillRect(renderer, &rect);
        }
        
        renderText(renderer, font, score);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    close(window, renderer, font);
    return 0;
}