#define VISUALIZATION_MODE
#define RESULT_IMPLEMENTATION
#include "result.cpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <stack>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>

// Configuration
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 1000;
const float CELL_SIZE = 6.0f;
const float OFFSET_X = 50.0f;
const float OFFSET_Y = 20.0f;

// Colors
const sf::Color COLOR_BG(10, 10, 30);      // Dark Night
const sf::Color COLOR_TREE(34, 139, 34);   // Forest Green
const sf::Color COLOR_SPLITTER(255, 0, 0); // Red Ornament
const sf::Color COLOR_BEAM(255, 255, 200); // Warm Light
const sf::Color COLOR_VISITED(0, 100, 0);  // Darker Green for visited paths
const sf::Color COLOR_MEMO(0, 0, 255);     // Blue for memoized hits

class SoundSystem
{
public:
  SoundSystem()
  {
    // Generate a few buffers
    generateTone(moveBuffer, 440, 0.05); // A4, short
    generateTone(splitBuffer, 880, 0.1); // A5, medium
    generateTone(memoBuffer, 1760, 0.2); // A6, long

    // Create a pool of sounds for polyphony
    for (int i = 0; i < 16; ++i)
    {
      soundPool.emplace_back();
    }
  }

  void playMove(float pitch = 1.0f)
  {
    playSound(moveBuffer, pitch * 0.5f, 0.2f); // Lower volume
  }

  void playSplit()
  {
    playSound(splitBuffer, 1.0f, 0.4f);
  }

  void playMemo()
  {
    playSound(memoBuffer, 1.0f, 0.6f);
  }

private:
  sf::SoundBuffer moveBuffer;
  sf::SoundBuffer splitBuffer;
  sf::SoundBuffer memoBuffer;
  std::vector<sf::Sound> soundPool;
  int poolIndex = 0;

  void generateTone(sf::SoundBuffer &buffer, double freq, double duration)
  {
    std::vector<sf::Int16> samples;
    int sampleRate = 44100;
    int sampleCount = sampleRate * duration;
    for (int i = 0; i < sampleCount; i++)
    {
      double t = (double)i / sampleRate;
      // Simple envelope
      double amp = 30000 * std::exp(-10 * t / duration);
      samples.push_back((sf::Int16)(amp * std::sin(2 * 3.14159 * freq * t)));
    }
    buffer.loadFromSamples(&samples[0], samples.size(), 1, sampleRate);
  }

  void playSound(const sf::SoundBuffer &buffer, float pitch, float volume)
  {
    sf::Sound &sound = soundPool[poolIndex];
    sound.setBuffer(buffer);
    sound.setPitch(pitch);
    sound.setVolume(volume * 100.0f);
    sound.play();

    poolIndex = (poolIndex + 1) % soundPool.size();
  }
};

enum Mode
{
  MODE_BFS,
  MODE_DFS,
  MODE_DONE
};

class Visualizer
{
public:
  Visualizer(const std::vector<std::string> &grid) : grid(grid)
  {
    rows = grid.size();
    cols = rows > 0 ? grid[0].size() : 0;

    // Find start
    for (int r = 0; r < rows; r++)
    {
      for (int c = 0; c < cols; c++)
      {
        if (grid[r][c] == 'S')
        {
          startRow = r;
          startCol = c;
        }
      }
    }

    // Start the solver thread
    solverThread = std::thread(&Visualizer::runSolver, this);
  }

  ~Visualizer()
  {
    if (solverThread.joinable())
    {
      // In a real app we should signal thread to stop
      // For now we just detach or let it finish if it's fast
      // But since it has infinite loops/sleeps, we should detach
      solverThread.detach();
    }
  }

  void draw(sf::RenderWindow &window)
  {
    std::lock_guard<std::mutex> lock(stateMutex);

    // Draw static elements (splitters as ornaments)
    for (int r = 0; r < rows; r++)
    {
      for (int c = 0; c < cols; c++)
      {
        float x = OFFSET_X + c * CELL_SIZE;
        float y = OFFSET_Y + r * CELL_SIZE;

        if (grid[r][c] == '^')
        {
          sf::CircleShape ornament(CELL_SIZE / 2.5f);
          ornament.setPosition(x, y);
          ornament.setFillColor(COLOR_SPLITTER);
          window.draw(ornament);
        }
      }
    }

    // Draw visited paths (Tree branches)
    // Use lines instead of points for connections
    sf::VertexArray lines(sf::Lines);
    for (auto &line : treeLines)
    {
      lines.append(line);
    }
    window.draw(lines);

    // Draw active beams/probes
    if (mode == MODE_BFS)
    {
      // Draw current beam
      sf::CircleShape light(CELL_SIZE / 1.5f);
      light.setPosition(OFFSET_X + currentBeam.col * CELL_SIZE, OFFSET_Y + currentBeam.row * CELL_SIZE);
      light.setFillColor(COLOR_BEAM);
      window.draw(light);
    }
    else if (mode == MODE_DFS)
    {
      // Draw DFS Probe
      sf::CircleShape probe(CELL_SIZE / 1.5f);
      probe.setPosition(OFFSET_X + dfsProbe.col * CELL_SIZE, OFFSET_Y + dfsProbe.row * CELL_SIZE);
      probe.setFillColor(sf::Color::Yellow);
      window.draw(probe);

      // Draw Memo Hits
      for (auto const &p : memoHits)
      {
        sf::CircleShape dot(CELL_SIZE / 2.0f);
        dot.setPosition(OFFSET_X + p.second * CELL_SIZE, OFFSET_Y + p.first * CELL_SIZE);
        dot.setFillColor(COLOR_MEMO);
        window.draw(dot);
      }
    }
  }

private:
  std::vector<std::string> grid;
  int rows, cols;
  int startRow, startCol;

  // Shared State
  std::mutex stateMutex;
  Mode mode = MODE_BFS;

  // BFS State
  Beam currentBeam = {0, 0, 0, 0};
  std::vector<sf::Vertex> treeLines;

  // DFS State
  Beam dfsProbe = {0, 0, 0, 0};
  std::vector<std::pair<int, int>> memoHits;

  std::thread solverThread;
  SoundSystem soundSystem;

  void runSolver()
  {
    // Run Part 1 (BFS)
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      mode = MODE_BFS;
    }

    int bfsStep = 0;
    auto bfsCallback = [this, &bfsStep](int r, int c, int from_r, int from_c, const std::set<std::pair<int, int>> &visitedSplitters)
    {
      {
        std::lock_guard<std::mutex> lock(stateMutex);
        currentBeam = {r, c, from_r, from_c};

        // Add line from previous to current
        float x1 = OFFSET_X + from_c * CELL_SIZE + CELL_SIZE / 2;
        float y1 = OFFSET_Y + from_r * CELL_SIZE + CELL_SIZE / 2;
        float x2 = OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2;
        float y2 = OFFSET_Y + r * CELL_SIZE + CELL_SIZE / 2;

        treeLines.push_back(sf::Vertex(sf::Vector2f(x1, y1), COLOR_TREE));
        treeLines.push_back(sf::Vertex(sf::Vector2f(x2, y2), COLOR_TREE));
      }

      // Sound
      if (grid[r][c] == '^')
      {
        soundSystem.playSplit();
      }
      else
      {
        // Pitch based on column to give stereo-like effect or just variation
        float pitch = 0.8f + (float)c / cols * 0.4f;
        soundSystem.playMove(pitch);
      }

      // Speed up: only sleep every 10 steps
      if (bfsStep++ % 10 == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };

    solpart1(grid, bfsCallback);

    // Transition
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
      std::lock_guard<std::mutex> lock(stateMutex);
      mode = MODE_DFS;
      treeLines.clear(); // Clear for Part 2
    }

    // Run Part 2 (DFS)
    int dfsStep = 0;
    auto dfsCallback = [this, &dfsStep](int r, int c, int from_r, int from_c, DFSAction action, long long val)
    {
      {
        std::lock_guard<std::mutex> lock(stateMutex);
        dfsProbe = {r, c, from_r, from_c};

        if (action == DFS_VISIT)
        {
          // Add line for DFS path
          float x1 = OFFSET_X + from_c * CELL_SIZE + CELL_SIZE / 2;
          float y1 = OFFSET_Y + from_r * CELL_SIZE + CELL_SIZE / 2;
          float x2 = OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2;
          float y2 = OFFSET_Y + r * CELL_SIZE + CELL_SIZE / 2;

          treeLines.push_back(sf::Vertex(sf::Vector2f(x1, y1), COLOR_TREE));
          treeLines.push_back(sf::Vertex(sf::Vector2f(x2, y2), COLOR_TREE));
        }
        else if (action == DFS_MEMO_HIT)
        {
          memoHits.push_back({r, c});
        }
      }

      if (action == DFS_VISIT)
      {
        // Sound based on column
        float pitch = 1.0f + (float)c / cols;
        soundSystem.playMove(pitch);

        if (dfsStep++ % 50 == 0) // Much faster
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      else if (action == DFS_MEMO_HIT)
      {
        soundSystem.playMemo();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Pause on hit
      }
    };

    solpart2(grid, dfsCallback);

    {
      std::lock_guard<std::mutex> lock(stateMutex);
      mode = MODE_DONE;
    }
  }
};

int main()
{
  // Load Grid
  std::vector<std::string> grid;
  std::ifstream file("2025/day_7/input/input.txt");
  if (!file.is_open())
  {
    file.open("input/input.txt");
  }
  if (!file.is_open())
  {
    std::cerr << "Failed to open input file" << std::endl;
    return 1;
  }
  std::string line;
  while (std::getline(file, line))
  {
    grid.push_back(line);
  }

  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AOC 2025 Day 7 - Christmas Tree Viz");
  window.setFramerateLimit(60);

  Visualizer viz(grid);

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    // No update() needed, thread handles it

    window.clear(COLOR_BG);
    viz.draw(window);
    window.display();
  }

  return 0;
}
