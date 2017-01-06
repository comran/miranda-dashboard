/*
 SDL2 example that allows user to move an object using arrow keys.
 This is meant to be used as a convenient single-file starting point for
 more complex projects.

 Author: Andrew Lim Chong Liang
 windrealm.org
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdio>
#include <map>
#include <math.h>
#include "dashboard/sensors.q.h"
#include "aos/linux_code/init.h"
#include <iostream>

enum {
  DISPLAY_WIDTH = 1280,
  DISPLAY_HEIGHT = 720,
  UPDATE_INTERVAL = 1000 / 60,
  HERO_SPEED = 2
};

enum { MID_RPM = 6000, HIGH_RPM = 7000 };

class Sprite {
 public:
  int x, y;
  Sprite() : x(0), y(0) {}
};

class Game {
 public:
  Game();
  ~Game();
  void start();
  void stop();
  void draw();
  void fillRect(SDL_Rect* rc, int r, int g, int b);
  void fpsChanged(int fps);
  void onQuit();
  void onKeyDown(SDL_Event* event);
  void onKeyUp(SDL_Event* event);
  void run();
  void update();

 private:
  std::map<int, int>
      keys;  // No SDLK_LAST. SDL2 migration guide suggests std::map
  int frameSkip;
  int running;
  SDL_Window* window;
  SDL_Renderer* renderer;
  Sprite hero;
};

Game::Game() : frameSkip(0), running(0), window(NULL), renderer(NULL) {}

Game::~Game() { this->stop(); }

void Game::start() {
  int flags = SDL_WINDOW_SHOWN;
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    return;
  }
  if (SDL_CreateWindowAndRenderer(DISPLAY_WIDTH, DISPLAY_HEIGHT, flags, &window,
                                  &renderer)) {
    return;
  }
  this->running = 1;
  run();
}

void Game::draw() {
  // Clear screen.
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // RPM meter.
  const int num_ticks = 125;
  const int tick_separate = 10;
  const int upper_rpm_threshold = 6500;
  const int HIGH_RPM = upper_rpm_threshold - upper_rpm_threshold * 0.16;
  const int MID_RPM = upper_rpm_threshold - upper_rpm_threshold * 0.4;
  if(!::dashboard::sensors_queue.FetchNext()) return;
  double rpm = ::dashboard::sensors_queue->rpm;
  if (rpm > upper_rpm_threshold) rpm = 0;
  for (int i = 0; i < num_ticks; i++) {
    SDL_Rect rpm_tick;
    rpm_tick.w = tick_separate / 2;
    rpm_tick.h = 25;
    rpm_tick.x =
        DISPLAY_WIDTH / 2 - num_ticks / 2 * tick_separate + i * tick_separate;
    rpm_tick.y = 80 - 50 * sin(M_PI * i / num_ticks);

    {
      int r(50), g(80), b(50);
      int tick_rpm = i / (double)num_ticks * upper_rpm_threshold;

      // Dim high RPM ticks.
      if (tick_rpm > HIGH_RPM) {
        r = 150;
        g = 50;
        b = 50;
      } else if (tick_rpm > MID_RPM) {
        r = 120;
        g = 80;
        b = 50;
      }

      // Highlighted ticks.
      if (tick_rpm < rpm) {
        if (tick_rpm > HIGH_RPM) {
          r = 255;
          g = 0;
          b = 0;
        } else if(tick_rpm > MID_RPM) {
          r = 255;
          g = 255;
          b = 0;
        } else {
          r = 0;
          g = 255;
          b = 0;
        }
      }

      fillRect(&rpm_tick, r, g, b);
    }
  }

  SDL_RenderPresent(renderer);
}

void Game::stop() {
  if (NULL != renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
  }
  if (NULL != window) {
    SDL_DestroyWindow(window);
    window = NULL;
  }
  SDL_Quit();
}

void Game::fillRect(SDL_Rect* rc, int r, int g, int b) {
  SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
  SDL_RenderFillRect(renderer, rc);
}

void Game::fpsChanged(int fps) {
  char szFps[128];
  sprintf(szFps, "%s: %d FPS", "SDL2 Base C++ - Use Arrow Keys to Move", fps);
  SDL_SetWindowTitle(window, szFps);
}

void Game::onQuit() { running = 0; }

void Game::run() {
  int past = SDL_GetTicks();
  int now = past, pastFps = past;
  int fps = 0, framesSkipped = 0;
  SDL_Event event;
  while (running) {
    int timeElapsed = 0;
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          onQuit();
          break;
        case SDL_KEYDOWN:
          onKeyDown(&event);
          break;
        case SDL_KEYUP:
          onKeyUp(&event);
          break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
          break;
      }
    }
    // update/draw
    timeElapsed = (now = SDL_GetTicks()) - past;
    if (timeElapsed >= UPDATE_INTERVAL) {
      past = now;
      update();
      if (framesSkipped++ >= frameSkip) {
        draw();
        ++fps;
        framesSkipped = 0;
      }
    }
    // fps
    if (now - pastFps >= 1000) {
      pastFps = now;
      fpsChanged(fps);
      fps = 0;
    }
    // sleep?
    SDL_Delay(1);
  }
}

void Game::update() {
  if (keys[SDLK_LEFT]) {
    hero.x -= HERO_SPEED;
  } else if (keys[SDLK_RIGHT]) {
    hero.x += HERO_SPEED;
  } else if (keys[SDLK_UP]) {
    hero.y -= HERO_SPEED;
  } else if (keys[SDLK_DOWN]) {
    hero.y += HERO_SPEED;
  }
}

void Game::onKeyDown(SDL_Event* evt) { keys[evt->key.keysym.sym] = 1; }

void Game::onKeyUp(SDL_Event* evt) { keys[evt->key.keysym.sym] = 0; }

int main(int, char**) {
  ::aos::Init();
  Game game;
  game.start();
  ::aos::Cleanup();
  return 0;
}
