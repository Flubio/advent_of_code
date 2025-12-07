// https://adventofcode.com/2025/day/7
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

struct Beam
{
  int row, col;
};

int solpart1(const std::vector<std::string> &grid)
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
  beams.push({startRow, startCol});

  int splitCount = 0; // count how many times beams are split

  while (!beams.empty())
  {
    Beam current = beams.front();
    beams.pop();

    int r = current.row;
    int c = current.col;

    // move downward until we hit a splitter or exit the grid
    while (r >= 0 && r < rows && c >= 0 && c < cols)
    {
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
          beams.push({r + 1, c - 1}); // left beam continues down
          beams.push({r + 1, c + 1}); // right beam continues down
        }
        break; // this beam stops at the splitter
      }

      r++; // move down
    }
  }

  return splitCount;
}

// Part 2: Count all possible timelines (paths) through the manifold
// Use memoization: memo[col] = number of timelines that reach this column
long long solpart2(const std::vector<std::string> &grid)
{
  int rows = grid.size();
  int cols = rows > 0 ? grid[0].size() : 0;

  // Find start position 'S'
  int startCol = -1;
  for (int c = 0; c < cols; c++)
  {
    if (grid[0][c] == 'S')
    {
      startCol = c;
      break;
    }
  }

  // Track number of timelines at each column position
  // We process row by row, tracking how many timelines arrive at each column
  std::map<int, long long> timelines;
  timelines[startCol] = 1; // Start with 1 timeline at S

  for (int r = 0; r < rows; r++)
  {
    std::map<int, long long> nextTimelines;

    for (auto &[col, count] : timelines)
    {
      if (col < 0 || col >= cols)
        continue;

      char cell = grid[r][col];

      if (cell == '^')
      {
        // Splitter: each timeline splits into two (left and right)
        // The particle continues from row r+1
        nextTimelines[col - 1] += count; // left path
        nextTimelines[col + 1] += count; // right path
      }
      else
      {
        // Empty space or S: timeline continues straight down
        nextTimelines[col] += count;
      }
    }

    timelines = nextTimelines;
  }

  // Sum all timelines that made it through
  long long totalTimelines = 0;
  for (auto &[col, count] : timelines)
  {
    totalTimelines += count;
  }

  return totalTimelines;
}

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
