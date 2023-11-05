#include "SDL.h"
#include "SDL_image.h"
#include<iostream>
#include <vector>

struct Button {
    SDL_Rect rect;
    bool hovering;
};

struct Bullet {
	SDL_Rect rect;
	bool active;
    int speed;
    int direction;
};

struct Enemy {
	SDL_Rect rect; // enemy
    SDL_Rect healthBarBox; // black border
    SDL_Rect healthBar;  // red health
    int health;
	int speed;
	int direction;
    bool active;
};

Bullet shoot(SDL_Renderer* renderer, SDL_Rect character, bool& canShoot) {
    //canShoot = false;
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    double deltaX = mouseX - (character.x+character.w / 2);
    double deltaY = mouseY - (character.y + character.h / 2);
    double angle = atan2(deltaY, deltaX);

    std::cout << "x: " << mouseX << " y: " << mouseY << " angle: " << angle << "\n";
    std::cout << "x: " << character.x << " y: " << character.y << "\n";

    // Create the bullet
    Bullet bullet = {
        { character.x + character.w / 2 - 5, character.y + character.h / 2 - 5, 10, 10 }, 
        true, 
        10, 
        angle 
    };

    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255); 
    SDL_RenderFillRect(renderer, &character);

    return bullet;
}

void collisionCheck(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies) {
    if (bullets.empty() || enemies.empty()) return;

    for (auto& bullet : bullets) {
        for (auto& enemy : enemies) {
            if (SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
				bullet.active = false;
				enemy.health -= 1;
                enemy.healthBar.w -= 1;
                std::cout<<enemy.health<<std::endl;
                if (enemy.health <= 0) {
					enemy.active = false;
					enemies.erase(enemies.begin());
				}
			}
		}
	}
}

void enemySpawner(std::vector<Enemy>& enemies, int w, int h, int max) {
    std::srand(static_cast<unsigned int>(time(nullptr)));
    int count = std::rand() % max + 1;

    for (int i = 0; i < count; ++i) {
        int spawnX = std::rand() % w;
        int spawnY = std::rand() % h;
        Enemy enemy = { {spawnX, spawnY, 50, 50}, {spawnX, spawnY - 25, 50, 10}, {spawnX, spawnY - 25, 50, 10}, 10, 1, 0, true };
        enemies.push_back(enemy);
    }
}

int main(int argc, char* argv[])
{
    int WIDTH = 1920;
    int HEIGHT = 1080;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    bool done = false;

    window = SDL_CreateWindow("SDL2 Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    //interaction
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mouseP = {mouseX, mouseY};


    SDL_Rect character = { WIDTH / 2 - 250, HEIGHT / 2 - 250, 30, 30 };
    std::vector<Bullet> bullets; 
    std::vector<Enemy> enemies; 

    enemySpawner(enemies, WIDTH, HEIGHT, 5);
    
    //game logic
    const int speed = 10;
    int point = 0;
    bool canShoot = true; 

    //time
    Uint64 lastTime = SDL_GetTicks64();
    const Uint64 interval = 1000; // 1 second

    Bullet myBullet = { { 0, 0, 10, 10 }, true, 10, 0 };
    Enemy myEnemy = { { 0, 0, 50, 50 }, { 0, 0, 50, 10 }, { 0, 0, 50, 10 }, 10, 5, 0, true };

    SDL_Event event;
    while (!done) {

        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_KEYDOWN: //input - character movement
                if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDL_SCANCODE_LEFT) {
					character.x -= speed;
				}
                else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDL_SCANCODE_RIGHT) {
                    character.x += speed;
                }
                if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDL_SCANCODE_UP) {
					character.y -= speed;
				}
                else if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDL_SCANCODE_DOWN) {
					character.y += speed;
				}
				break;


                //input - mouse
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&mouseX, &mouseY);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (canShoot) {
                    myBullet = shoot(renderer, character, canShoot);
                    bullets.push_back(myBullet);
				}
				break;
            }
        }
        for (auto& bullet : bullets) {
            bullet.rect.x += cos(bullet.direction) * bullet.speed;
            bullet.rect.y += sin(bullet.direction) * bullet.speed;
        }

        for (auto& enemy : enemies) {
			enemy.rect.x += cos(enemy.direction) * enemy.speed;
            enemy.healthBarBox.x += cos(enemy.direction) * enemy.speed;
            enemy.healthBar.x += cos(enemy.direction) * enemy.speed;

			enemy.rect.y += sin(enemy.direction) * enemy.speed;
            enemy.healthBarBox.y += sin(enemy.direction) * enemy.speed;
			enemy.healthBar.y += sin(enemy.direction) * enemy.speed;
		}

        if (SDL_GetTicks64() > lastTime + 1000) {
			enemySpawner(enemies, WIDTH, HEIGHT, 5);
            lastTime = SDL_GetTicks64();
        }

        if (SDL_GetTicks64() > lastTime + 40) {
            collisionCheck(bullets, enemies);
			lastTime = SDL_GetTicks64();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);

        //draw
        SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255); 
        SDL_RenderFillRect(renderer, &character);
       

        for (auto& bullet : bullets) {
            if (bullet.active) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); 
                SDL_RenderFillRect(renderer, &bullet.rect);

                if (bullet.rect.x < 0 || bullet.rect.x > WIDTH || bullet.rect.y < 0 || bullet.rect.y > HEIGHT) {
					bullet.active = false;
                    bullets.erase(bullets.begin());
				}
            }
        }

        for(auto& enemy : enemies){
            if (enemy.active) {

			    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			    SDL_RenderFillRect(renderer, &enemy.rect);
                SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255); 
			    SDL_RenderFillRect(renderer, &enemy.healthBar);
		    }
        
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
