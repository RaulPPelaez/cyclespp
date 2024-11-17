#include "api.h"
#include "utils.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <queue>
#include <random>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

using namespace cycles;

class SmarterBotClient {
  Connection connection;
  std::string name;
  GameState state;
  Player my_player;
  Player opponent;
  std::mt19937 rng;

  bool is_valid_move(Direction direction) {
    auto new_pos = my_player.position + getDirectionVector(direction);
    if (!state.isInsideGrid(new_pos)) {
      return false;
    }
    if (state.getGridCell(new_pos) != 0) {
      return false;
    }
    return true;
  }

  int floodFill(const sf::Vector2i& start_pos, std::vector<std::vector<bool>>& visited) {
    std::queue<sf::Vector2i> queue;
    queue.push(start_pos);
    int area = 0;
    while (!queue.empty()) {
      sf::Vector2i pos = queue.front();
      queue.pop();
      if (!state.isInsideGrid(pos)) continue;
      if (visited[pos.x][pos.y]) continue;
      if (state.getGridCell(pos) != 0) continue;
      visited[pos.x][pos.y] = true;
      area++;
      for (int dir_value = 0; dir_value < 4; ++dir_value) {
        Direction dir = getDirectionFromValue(dir_value);
        sf::Vector2i next_pos = pos + getDirectionVector(dir);
        queue.push(next_pos);
      }
    }
    return area;
  }

  Direction decideMove() {
    std::vector<std::pair<Direction, int>> moves;
    const auto position = my_player.position;
    const auto opp_position = opponent.position;

    for (int dir_value = 0; dir_value < 4; ++dir_value) {
      Direction dir = getDirectionFromValue(dir_value);
      if (is_valid_move(dir)) {
        auto new_pos = position + getDirectionVector(dir);
        std::vector<std::vector<bool>> visited(
            state.gridWidth, std::vector<bool>(state.gridHeight, false));
        int my_area = floodFill(new_pos, visited);

        // Estimate opponent area if they moved
        int opp_area = 0;
        for (int opp_dir_value = 0; opp_dir_value < 4; ++opp_dir_value) {
          Direction opp_dir = getDirectionFromValue(opp_dir_value);
          auto opp_new_pos = opp_position + getDirectionVector(opp_dir);
          if (state.isInsideGrid(opp_new_pos) && state.getGridCell(opp_new_pos) == 0) {
            std::vector<std::vector<bool>> opp_visited = visited;
            opp_area += floodFill(opp_new_pos, opp_visited);
          }
        }

        // Minimize opponent's area while maximizing mine
        int score = my_area - opp_area;
        moves.emplace_back(dir, score);
        spdlog::debug("{}: Direction {} has score {}", name, dir_value, score);
      }
    }

    if (moves.empty()) {
      spdlog::error("{}: No valid moves available", name);
      exit(1);
    }

    // Choose the move with the best score
    auto best_move = *std::max_element(
        moves.begin(), moves.end(),
        [](const std::pair<Direction, int>& a, const std::pair<Direction, int>& b) {
          return a.second < b.second;
        });

    spdlog::debug("{}: Selected direction {} with score {}", name,
                  getDirectionValue(best_move.first), best_move.second);
    return best_move.first;
  }

  void receiveGameState() {
    state = connection.receiveGameState();
    for (const auto& player : state.players) {
      if (player.name == name) {
        my_player = player;
      } else {
        opponent = player;
      }
    }
  }

  void sendMove() {
    spdlog::debug("{}: Sending move", name);
    auto move = decideMove();
    connection.sendMove(move);
  }

 public:
  SmarterBotClient(const std::string& botName) : name(botName) {
    std::random_device rd;
    rng.seed(rd());
    connection.connect(name);
    if (!connection.isActive()) {
      spdlog::critical("{}: Connection failed", name);
      exit(1);
    }
  }

  void run() {
    while (connection.isActive()) {
      receiveGameState();
      sendMove();
    }
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <bot_name>" << std::endl;
    return 1;
  }
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
  spdlog::set_level(spdlog::level::debug);
#endif
  std::string botName = argv[1];
  SmarterBotClient bot(botName);
  bot.run();
  return 0;
}