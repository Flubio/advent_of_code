#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

char *readFile()
{
  FILE *file = fopen("input/input.txt", "r");
  static char content[10000];
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

// find the biggest possible rectangle that can be spanned with the two given corner points
// and calculate its area
// having just two points, we need to find the largest possible area all these points fit in
// after that we can translate the corner points from xxxx, xxxx to x,y coordinates for each point
// and then calculate the area of the rectangle spanned by these two points

long long solvePart1(char *input)
{
  if (input == NULL || input[0] == '\0')
  {
    return 0;
  }

  // parse all points into an array
  size_t len = strlen(input);
  char *buf = malloc(len + 1);
  if (!buf)
    return 0;
  memcpy(buf, input, len + 1);

  size_t cap = 512;
  size_t n = 0;
  typedef struct
  {
    int x, y;
  } Point;
  Point *pts = malloc(cap * sizeof(Point));
  if (!pts)
  {
    free(buf);
    return 0;
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

  if (n < 2)
  {
    free(pts);
    return 0;
  }

  long long max_area = 0;
  int best_i = -1, best_j = -1;
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = i + 1; j < n; j++)
    {
      long long dx = llabs((long long)pts[i].x - (long long)pts[j].x);
      long long dy = llabs((long long)pts[i].y - (long long)pts[j].y);
      long long area = (dx + 1) * (dy + 1);
      if (area > max_area)
      {
        max_area = area;
        best_i = (int)i;
        best_j = (int)j;
      }
    }
  }

  // optionally, we could print the best pair
  if (best_i != -1 && best_j != -1)
  {
    printf("Best corners: (%d,%d) and (%d,%d) => area=%lld\n", pts[best_i].x, pts[best_i].y, pts[best_j].x, pts[best_j].y, max_area);
  }

  free(pts);
  return max_area;
}

long long solvePart2(char *input)
{
  if (input == NULL || input[0] == '\0')
    return 0;

  size_t len = strlen(input);
  char *buf = malloc(len + 1);
  if (!buf)
    return 0;
  memcpy(buf, input, len + 1);

  size_t cap = 512;
  size_t n = 0;
  typedef struct
  {
    int x, y;
  } Point;
  Point *pts = malloc(cap * sizeof(Point));
  if (!pts)
  {
    free(buf);
    return 0;
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

  if (n < 2)
  {
    free(pts);
    return 0;
  }

  // find polygon bounding box
  int min_x = pts[0].x, max_x = pts[0].x;
  int min_y = pts[0].y, max_y = pts[0].y;
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

  // check each pair of red tiles as rectangle corners
  long long max_area = 0;
  int best_i = -1, best_j = -1;

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

      // early termination: skip if can't beat current max
      if (area <= max_area)
        continue;

      // check if any polygon edge passes through the interior of the rectangle
      // if an edge crosses through the rectangle (not just touches the boundary),
      // then some tiles inside the rectangle are outside the polygon
      int valid = 1;

      for (size_t k = 0; k < n && valid; k++)
      {
        size_t next = (k + 1) % n;
        int ex1 = pts[k].x, ey1 = pts[k].y;
        int ex2 = pts[next].x, ey2 = pts[next].y;

        if (ex1 == ex2)
        {
          // vertical edge at x = ex1
          // check if it crosses through the interior of the rectangle
          int edge_miny = (ey1 < ey2) ? ey1 : ey2;
          int edge_maxy = (ey1 < ey2) ? ey2 : ey1;

          // edge is strictly inside rectangle's x range (not on boundary)
          if (ex1 > minx && ex1 < maxx)
          {
            // check if edge's y range overlaps with rectangle's interior
            if (edge_miny < maxy && edge_maxy > miny)
            {
              valid = 0; // edge cuts through rectangle interior
            }
          }
        }
        else if (ey1 == ey2)
        {
          // horizontal edge at y = ey1
          int edge_minx = (ex1 < ex2) ? ex1 : ex2;
          int edge_maxx = (ex1 < ex2) ? ex2 : ex1;

          // edge is strictly inside rectangle's y range (not on boundary)
          if (ey1 > miny && ey1 < maxy)
          {
            // check if edge's x range overlaps with rectangle's interior
            if (edge_minx < maxx && edge_maxx > minx)
            {
              valid = 0; // edge cuts through rectangle interior
            }
          }
        }
      }

      // also check that the rectangle is actually inside the polygon
      // check the center point of the rectangle
      if (valid)
      {
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
        if ((crossings % 2) == 0)
          valid = 0; // center not inside polygon
      }

      if (valid && area > max_area)
      {
        max_area = area;
        best_i = (int)i;
        best_j = (int)j;
      }
    }
  }

  if (best_i != -1 && best_j != -1)
  {
    printf("Part 2 - Best corners: (%d,%d) and (%d,%d) => area=%lld\n", pts[best_i].x, pts[best_i].y, pts[best_j].x, pts[best_j].y, max_area);
  }

  free(pts);
  return max_area;
}

int main()
{
  char *input = readFile();

  long long area1 = solvePart1(input);
  printf("Part 1: Maximum rectangle area: %lld\n", area1);

  long long area2 = solvePart2(input);
  printf("Part 2: Maximum rectangle area (red/green only): %lld\n", area2);

  return 0;
}