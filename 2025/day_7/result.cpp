// https://adventofcode.com/2025/day/7
#ifndef RESULT_H
#define RESULT_H

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>

struct Beam
{
  int row, col;
  int from_r, from_c; // For visualization connections
};

enum DFSAction
{
  DFS_VISIT,
  DFS_MEMO_HIT,
  DFS_RETURN
};

// Callback types
// r, c: current position
// from_r, from_c: previous position (for drawing lines)
// visitedSplitters: current set of splitters
using Part1Callback = std::function<void(int r, int c, int from_r, int from_c, const std::set<std::pair<int, int>> &visitedSplitters)>;
using Part2Callback = std::function<void(int r, int c, int from_r, int from_c, DFSAction action, long long val)>;

int solpart1(const std::vector<std::string> &grid, Part1Callback callback = nullptr);
long long solpart2(const std::vector<std::string> &grid, Part2Callback callback = nullptr);

#endif // RESULT_H

#ifdef RESULT_IMPLEMENTATION
int solpart1(const std::vector<std::string> &grid, Part1Callback callback)
{
  int rows = grid.size();
  int cols = rows > 0 ? grid[0].size() : 0;

  // Find start position 'S'
  int startRow = -1, startCol = -1;
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      if (grid[r][c] == 'S')
      {
        startRow = r;
        startCol = c;
        break;
      }
    }
    if (startRow != -1)
      break;
  }

  // BFS
  std::queue<Beam> beams;
  std::set<std::pair<int, int>> visitedSplitters; // track which splitters have been hit

  // start beam moving down from S
  beams.push({startRow, startCol, startRow, startCol}); // Start from itself effectively

  int splitCount = 0; // count how many times beams are split

  while (!beams.empty())
  {
    Beam current = beams.front();
    beams.pop();

    int r = current.row;
    int c = current.col;
    int from_r = current.from_r;
    int from_c = current.from_c;

    // move downward until we hit a splitter or exit the grid
    while (r >= 0 && r < rows && c >= 0 && c < cols)
    {
      if (callback)
        callback(r, c, from_r, from_c, visitedSplitters);

      char cell = grid[r][c];

      if (cell == '^')
      {
        // check if this splitter was already hit
        auto splitterPos = std::make_pair(r, c);
        if (visitedSplitters.count(splitterPos) == 0)
        {
          visitedSplitters.insert(splitterPos);
          splitCount++; // count this split

          // create two new beams from immediate left and right, going down
          // Connection is from (r, c) to (r+1, c-1) and (r+1, c+1)
          beams.push({r + 1, c - 1, r, c}); // left beam continues down
          beams.push({r + 1, c + 1, r, c}); // right beam continues down
        }
        break; // this beam stops at the splitter
      }

      // Prepare for next step
      from_r = r;
      from_c = c;
      r++; // move down
    }
  }

  return splitCount;
}

// Helper for Part 2 DFS
long long countPathsRecursive(int r, int c, int from_r, int from_c, int rows, int cols,
                              const std::vector<std::string> &grid,
                              std::map<std::pair<int, int>, long long> &memo,
                              Part2Callback callback)
{

  // Check bounds
  if (c < 0 || c >= cols)
    return 0;
  if (r >= rows)
    return 1; // Reached bottom successfully

  // Check memo
  if (memo.count({r, c}))
  {
    if (callback)
      callback(r, c, from_r, from_c, DFS_MEMO_HIT, memo[{r, c}]);
    return memo[{r, c}];
  }

  if (callback)
    callback(r, c, from_r, from_c, DFS_VISIT, 0);

  long long result = 0;
  char cell = grid[r][c];

  if (cell == '^')
  {
    // Splitter: sum paths from left and right
    result = countPathsRecursive(r + 1, c - 1, r, c, rows, cols, grid, memo, callback) +
             countPathsRecursive(r + 1, c + 1, r, c, rows, cols, grid, memo, callback);
  }
  else
  {
    // Continue straight down
    result = countPathsRecursive(r + 1, c, r, c, rows, cols, grid, memo, callback);
  }

  memo[{r, c}] = result;
  if (callback)
    callback(r, c, from_r, from_c, DFS_RETURN, result);
  return result;
}

// Part 2: Count all possible timelines (paths) through the manifold
// REWRITTEN: Recursive DFS with Memoization
long long solpart2(const std::vector<std::string> &grid, Part2Callback callback)
{
  int rows = grid.size();
  int cols = rows > 0 ? grid[0].size() : 0;

  // Find start position 'S'
  int startRow = -1, startCol = -1;
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      if (grid[r][c] == 'S')
      {
        startRow = r;
        startCol = c;
        break;
      }
    }
    if (startRow != -1)
      break;
  }

  std::map<std::pair<int, int>, long long> memo;
  // Start from S, no previous node really, so use S itself
  return countPathsRecursive(startRow, startCol, startRow, startCol, rows, cols, grid, memo, callback);
}
#endif

#ifndef VISUALIZATION_MODE
int main()
{
  std::ifstream file("input/input.txt");
  std::string line;
  std::vector<std::string> grid;

  // Parse the grid
  while (std::getline(file, line))
  {
    grid.push_back(line);
  }

  std::cout << "Part 1 - Total splits: " << solpart1(grid) << std::endl;
  std::cout << "Part 2 - Total timelines: " << solpart2(grid) << std::endl;

  return 0;
}
#endif
