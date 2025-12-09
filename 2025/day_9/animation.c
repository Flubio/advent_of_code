// animated visualization for day 9 using sdl2
// compile: gcc -std=c11 -O2 -o animation_sdl animation.c -lSDL2 -lm
// install sdl2: sudo apt install libsdl2-dev

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

typedef struct
{
  int x, y;
} Point;

typedef struct
{
  int minx, miny, maxx, maxy;
  long long area;
  int i, j;
} Rectangle;

Point *pts = NULL;
size_t n = 0;

// bounds
int min_x, max_x, min_y, max_y;

// window
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

char *readFile()
{
  FILE *file = fopen("input/input.txt", "r");
  static char content[100000];
  char line[1000];

  if (file)
  {
    content[0] = '\0';
    while (fgets(line, sizeof(line), file))
    {
      strcat(content, line);
    }
    fclose(file);
    return content;
  }
  return "";
}

void parseInput(char *input)
{
  if (input == NULL || input[0] == '\0')
    return;

  size_t len = strlen(input);
  char *buf = malloc(len + 1);
  if (!buf)
    return;
  memcpy(buf, input, len + 1);

  size_t cap = 512;
  pts = malloc(cap * sizeof(Point));
  if (!pts)
  {
    free(buf);
    return;
  }

  char *line = strtok(buf, "\n");
  while (line)
  {
    int x, y;
    if (sscanf(line, "%d,%d", &x, &y) == 2)
    {
      if (n >= cap)
      {
        cap *= 2;
        Point *tmp = realloc(pts, cap * sizeof(Point));
        if (!tmp)
          break;
        pts = tmp;
      }
      pts[n].x = x;
      pts[n].y = y;
      n++;
    }
    line = strtok(NULL, "\n");
  }
  free(buf);

  // find bounds with padding
  min_x = pts[0].x;
  max_x = pts[0].x;
  min_y = pts[0].y;
  max_y = pts[0].y;
  for (size_t i = 1; i < n; i++)
  {
    if (pts[i].x < min_x)
      min_x = pts[i].x;
    if (pts[i].x > max_x)
      max_x = pts[i].x;
    if (pts[i].y < min_y)
      min_y = pts[i].y;
    if (pts[i].y > max_y)
      max_y = pts[i].y;
  }
  int pad = (max_x - min_x) / 20;
  min_x -= pad;
  max_x += pad;
  min_y -= pad;
  max_y += pad;
}

// convert world coords to screen coords
int toScreenX(int x)
{
  return (int)((double)(x - min_x) / (max_x - min_x) * WINDOW_WIDTH);
}

int toScreenY(int y)
{
  return (int)((double)(y - min_y) / (max_y - min_y) * WINDOW_HEIGHT);
}

void setColor(int r, int g, int b, int a)
{
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void drawLine(int x1, int y1, int x2, int y2)
{
  SDL_RenderDrawLine(renderer, toScreenX(x1), toScreenY(y1), toScreenX(x2), toScreenY(y2));
}

void drawThickLine(int x1, int y1, int x2, int y2, int thickness)
{
  int sx1 = toScreenX(x1), sy1 = toScreenY(y1);
  int sx2 = toScreenX(x2), sy2 = toScreenY(y2);

  for (int dx = -thickness / 2; dx <= thickness / 2; dx++)
  {
    for (int dy = -thickness / 2; dy <= thickness / 2; dy++)
    {
      SDL_RenderDrawLine(renderer, sx1 + dx, sy1 + dy, sx2 + dx, sy2 + dy);
    }
  }
}

void drawRect(int x1, int y1, int x2, int y2)
{
  SDL_Rect rect;
  rect.x = toScreenX(x1 < x2 ? x1 : x2);
  rect.y = toScreenY(y1 < y2 ? y1 : y2);
  rect.w = abs(toScreenX(x2) - toScreenX(x1));
  rect.h = abs(toScreenY(y2) - toScreenY(y1));
  SDL_RenderDrawRect(renderer, &rect);
}

void fillRect(int x1, int y1, int x2, int y2)
{
  SDL_Rect rect;
  rect.x = toScreenX(x1 < x2 ? x1 : x2);
  rect.y = toScreenY(y1 < y2 ? y1 : y2);
  rect.w = abs(toScreenX(x2) - toScreenX(x1));
  rect.h = abs(toScreenY(y2) - toScreenY(y1));
  SDL_RenderFillRect(renderer, &rect);
}

void drawCircle(int cx, int cy, int radius)
{
  int sx = toScreenX(cx);
  int sy = toScreenY(cy);
  for (int w = 0; w < radius * 2; w++)
  {
    for (int h = 0; h < radius * 2; h++)
    {
      int dx = radius - w;
      int dy = radius - h;
      if ((dx * dx + dy * dy) <= (radius * radius))
      {
        SDL_RenderDrawPoint(renderer, sx + dx, sy + dy);
      }
    }
  }
}

void fillCircle(int cx, int cy, int radius)
{
  int sx = toScreenX(cx);
  int sy = toScreenY(cy);
  for (int dy = -radius; dy <= radius; dy++)
  {
    int dx = (int)sqrt(radius * radius - dy * dy);
    SDL_RenderDrawLine(renderer, sx - dx, sy + dy, sx + dx, sy + dy);
  }
}

void drawPolygon(size_t edgeCount, bool showVertices)
{
  // draw edges
  setColor(50, 205, 50, 255); // green
  for (size_t i = 0; i < edgeCount && i < n; i++)
  {
    size_t next = (i + 1) % n;
    drawThickLine(pts[i].x, pts[i].y, pts[next].x, pts[next].y, 2);
  }

  // draw vertices
  if (showVertices)
  {
    setColor(255, 80, 80, 255); // red
    for (size_t i = 0; i <= edgeCount && i < n; i++)
    {
      fillCircle(pts[i].x, pts[i].y, 4);
    }
  }
}

void drawPolygonFull()
{
  drawPolygon(n, true);
}

int isValidPart2(int minx, int miny, int maxx, int maxy)
{
  for (size_t k = 0; k < n; k++)
  {
    size_t next = (k + 1) % n;
    int ex1 = pts[k].x, ey1 = pts[k].y;
    int ex2 = pts[next].x, ey2 = pts[next].y;

    if (ex1 == ex2)
    {
      int edge_miny = (ey1 < ey2) ? ey1 : ey2;
      int edge_maxy = (ey1 < ey2) ? ey2 : ey1;
      if (ex1 > minx && ex1 < maxx)
        if (edge_miny < maxy && edge_maxy > miny)
          return 0;
    }
    else if (ey1 == ey2)
    {
      int edge_minx = (ex1 < ex2) ? ex1 : ex2;
      int edge_maxx = (ex1 < ex2) ? ex2 : ex1;
      if (ey1 > miny && ey1 < maxy)
        if (edge_minx < maxx && edge_maxx > minx)
          return 0;
    }
  }

  int cx = (minx + maxx) / 2;
  int cy = (miny + maxy) / 2;
  int crossings = 0;
  for (size_t k = 0; k < n; k++)
  {
    size_t next = (k + 1) % n;
    int px1 = pts[k].x, py1 = pts[k].y;
    int px2 = pts[next].x, py2 = pts[next].y;
    if ((py1 <= cy && cy < py2) || (py2 <= cy && cy < py1))
    {
      double t = (double)(cy - py1) / (py2 - py1);
      double x_intersect = px1 + t * (px2 - px1);
      if (cx < x_intersect)
        crossings++;
    }
  }
  return (crossings % 2) == 1;
}

bool handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
      return false;
    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
        return false;
      if (event.key.keysym.sym == SDLK_SPACE)
        return true; // skip animation
    }
  }
  return true;
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  char *input = readFile();
  parseInput(input);

  if (n < 2)
  {
    printf("not enough points\n");
    return 1;
  }

  printf("loaded %zu points\n", n);

  // init sdl
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("sdl init failed: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Day 9 - Rectangle Visualization",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window)
  {
    printf("window creation failed: %s\n", SDL_GetError());
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    printf("renderer creation failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // phase 1: draw polygon edges one by one
  printf("phase 1: drawing polygon...\n");
  for (size_t i = 0; i <= n; i++)
  {
    if (!handleEvents())
      goto cleanup;

    setColor(20, 20, 40, 255);
    SDL_RenderClear(renderer);

    drawPolygon(i, true);

    SDL_RenderPresent(renderer);
    SDL_Delay(10);
  }

  SDL_Delay(500);

  // phase 2: search for part 1
  printf("phase 2: searching part 1...\n");
  Rectangle best1 = {0, 0, 0, 0, 0, -1, -1};
  int step = (int)(n / 80) + 1;

  for (size_t i = 0; i < n; i += step)
  {
    for (size_t j = i + 1; j < n; j += step)
    {
      if (!handleEvents())
        goto cleanup;

      long long dx = llabs((long long)pts[i].x - (long long)pts[j].x);
      long long dy = llabs((long long)pts[i].y - (long long)pts[j].y);
      long long area = (dx + 1) * (dy + 1);

      int x1 = pts[i].x, y1 = pts[i].y;
      int x2 = pts[j].x, y2 = pts[j].y;
      int minx = (x1 < x2) ? x1 : x2;
      int maxx = (x1 < x2) ? x2 : x1;
      int miny = (y1 < y2) ? y1 : y2;
      int maxy = (y1 < y2) ? y2 : y1;

      setColor(20, 20, 40, 255);
      SDL_RenderClear(renderer);

      // draw best so far (blue fill)
      if (best1.i >= 0)
      {
        setColor(66, 135, 245, 60);
        fillRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);
        setColor(66, 135, 245, 200);
        drawRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);
      }

      // draw current test (cyan outline)
      setColor(100, 200, 255, 150);
      drawRect(minx, miny, maxx, maxy);

      drawPolygonFull();
      SDL_RenderPresent(renderer);

      if (area > best1.area)
      {
        best1.area = area;
        best1.i = (int)i;
        best1.j = (int)j;
        best1.minx = minx;
        best1.maxx = maxx;
        best1.miny = miny;
        best1.maxy = maxy;
        SDL_Delay(30);
      }
      else
      {
        SDL_Delay(5);
      }
    }
  }

  // compute actual best for part 1
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = i + 1; j < n; j++)
    {
      long long dx = llabs((long long)pts[i].x - (long long)pts[j].x);
      long long dy = llabs((long long)pts[i].y - (long long)pts[j].y);
      long long area = (dx + 1) * (dy + 1);
      if (area > best1.area)
      {
        best1.area = area;
        best1.i = (int)i;
        best1.j = (int)j;
      }
    }
  }
  printf("part 1: %lld\n", best1.area);

  // show part 1 result
  for (int flash = 0; flash < 6; flash++)
  {
    if (!handleEvents())
      goto cleanup;

    setColor(20, 20, 40, 255);
    SDL_RenderClear(renderer);

    if (flash % 2 == 0)
    {
      setColor(66, 135, 245, 100);
      fillRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);
    }
    setColor(66, 135, 245, 255);
    drawRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);

    drawPolygonFull();
    SDL_RenderPresent(renderer);
    SDL_Delay(200);
  }

  SDL_Delay(1000);

  // phase 3: search for part 2
  printf("phase 3: searching part 2...\n");
  Rectangle best2 = {0, 0, 0, 0, 0, -1, -1};

  for (size_t i = 0; i < n; i += step)
  {
    for (size_t j = i + 1; j < n; j += step)
    {
      if (!handleEvents())
        goto cleanup;

      int x1 = pts[i].x, y1 = pts[i].y;
      int x2 = pts[j].x, y2 = pts[j].y;
      int minx = (x1 < x2) ? x1 : x2;
      int maxx = (x1 < x2) ? x2 : x1;
      int miny = (y1 < y2) ? y1 : y2;
      int maxy = (y1 < y2) ? y2 : y1;

      long long dx = maxx - minx;
      long long dy = maxy - miny;
      long long area = (dx + 1) * (dy + 1);
      int valid = isValidPart2(minx, miny, maxx, maxy);

      setColor(20, 20, 40, 255);
      SDL_RenderClear(renderer);

      // draw part 1 best (dim blue)
      setColor(66, 135, 245, 30);
      fillRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);

      // draw part 2 best so far (green fill)
      if (best2.i >= 0)
      {
        setColor(76, 217, 100, 80);
        fillRect(pts[best2.i].x, pts[best2.i].y, pts[best2.j].x, pts[best2.j].y);
        setColor(76, 217, 100, 220);
        drawRect(pts[best2.i].x, pts[best2.i].y, pts[best2.j].x, pts[best2.j].y);
      }

      // draw current test
      if (valid)
      {
        setColor(255, 220, 100, 150); // yellow for valid
      }
      else
      {
        setColor(255, 80, 80, 100); // red for invalid
      }
      drawRect(minx, miny, maxx, maxy);

      drawPolygonFull();
      SDL_RenderPresent(renderer);

      if (valid && area > best2.area)
      {
        best2.area = area;
        best2.i = (int)i;
        best2.j = (int)j;
        best2.minx = minx;
        best2.maxx = maxx;
        best2.miny = miny;
        best2.maxy = maxy;
        SDL_Delay(50);
      }
      else
      {
        SDL_Delay(5);
      }
    }
  }

  // compute actual best for part 2
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = i + 1; j < n; j++)
    {
      int x1 = pts[i].x, y1 = pts[i].y;
      int x2 = pts[j].x, y2 = pts[j].y;
      int minx = (x1 < x2) ? x1 : x2;
      int maxx = (x1 < x2) ? x2 : x1;
      int miny = (y1 < y2) ? y1 : y2;
      int maxy = (y1 < y2) ? y2 : y1;

      long long dx = maxx - minx;
      long long dy = maxy - miny;
      long long area = (dx + 1) * (dy + 1);

      if (area > best2.area && isValidPart2(minx, miny, maxx, maxy))
      {
        best2.area = area;
        best2.i = (int)i;
        best2.j = (int)j;
      }
    }
  }
  printf("part 2: %lld\n", best2.area);

  // final display - loop until user quits
  printf("showing final result (press q or esc to quit)\n");
  bool running = true;
  int t = 0;
  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        running = false;
      if (event.type == SDL_KEYDOWN)
      {
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
          running = false;
      }
    }

    setColor(20, 20, 40, 255);
    SDL_RenderClear(renderer);

    // part 1 rectangle (blue, pulsing)
    int alpha1 = 40 + (int)(20 * sin(t * 0.05));
    setColor(66, 135, 245, alpha1);
    fillRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);
    setColor(66, 135, 245, 180);
    drawRect(pts[best1.i].x, pts[best1.i].y, pts[best1.j].x, pts[best1.j].y);

    // part 2 rectangle (green, pulsing)
    int alpha2 = 80 + (int)(30 * sin(t * 0.07 + 1));
    setColor(76, 217, 100, alpha2);
    fillRect(pts[best2.i].x, pts[best2.i].y, pts[best2.j].x, pts[best2.j].y);
    setColor(76, 217, 100, 255);
    drawRect(pts[best2.i].x, pts[best2.i].y, pts[best2.j].x, pts[best2.j].y);

    // highlight corners
    setColor(66, 135, 245, 255);
    fillCircle(pts[best1.i].x, pts[best1.i].y, 6);
    fillCircle(pts[best1.j].x, pts[best1.j].y, 6);

    setColor(76, 217, 100, 255);
    fillCircle(pts[best2.i].x, pts[best2.i].y, 8);
    fillCircle(pts[best2.j].x, pts[best2.j].y, 8);

    drawPolygonFull();

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
    t++;
  }

cleanup:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  free(pts);

  printf("\nfinal results:\n");
  printf("part 1: %lld\n", best1.area);
  printf("part 2: %lld\n", best2.area);

  return 0;
}
