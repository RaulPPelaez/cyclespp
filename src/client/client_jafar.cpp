#include "api.h"
#include "utils.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <limits>
#include <random>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

using namespace cycles;

class SmartestBotClient {
  Connection connection;
  std::string name;
  GameState state;
  Player my_player;
  Player opponent;
  std::mt19937 rng;

  const int MAX_DEPTH = 3;  // Maximum depth for Minimax search

  bool is_valid_move(const GameState& gameState, const Player& player, Direction direction) {
    // Check if the new position is within the grid and not occupied
    auto new_pos = player.position + getDirectionVector(direction);
    if (!gameState.isInsideGrid(new_pos)) {
      // Position is outside the grid, invalid move
      return false;
    }
    if (gameState.getGridCell(new_pos) != 0) {
      // Cell is already occupied, can't move there
      return false;
    }
    return true;
  }

  int evaluate(const GameState& gameState) {
    // Simple heuristic: difference in accessible area
    // The bot wants to maximize its area and minimize opponent's area
    auto my_area = floodFill(gameState, my_player.position);
    auto opp_area = floodFill(gameState, opponent.position);
    return my_area - opp_area;
  }

  int floodFill(const GameState& gameState, const sf::Vector2i& start_pos) {
    // Perform flood fill algorithm to calculate accessible area from start_pos
    std::vector<std::vector<bool>> visited(
        gameState.gridWidth, std::vector<bool>(gameState.gridHeight, false));
    std::queue<sf::Vector2i> queue;
    queue.push(start_pos);
    int area = 0;
    while (!queue.empty()) {
      sf::Vector2i pos = queue.front();
      queue.pop();
      if (!gameState.isInsideGrid(pos)) continue;
      if (visited[pos.x][pos.y]) continue;
      if (gameState.getGridCell(pos) != 0) continue;
      visited[pos.x][pos.y] = true;
      area++;
      // Explore all adjacent positions
      for (int dir_value = 0; dir_value < 4; ++dir_value) {
        Direction dir = getDirectionFromValue(dir_value);
        sf::Vector2i next_pos = pos + getDirectionVector(dir);
        queue.push(next_pos);
      }
    }
    return area;
  }

  int minimax(GameState gameState, int depth, bool maximizingPlayer, int alpha, int beta) {
    // Minimax algorithm with alpha-beta pruning
    if (depth == 0 || game_over(gameState)) {
      // Reached maximum depth or game is over
      return evaluate(gameState);
    }

    if (maximizingPlayer) {
      int maxEval = std::numeric_limits<int>::min();
      // Try all possible moves for the maximizing player (our bot)
      for (int dir_value = 0; dir_value < 4; ++dir_value) {
        Direction dir = getDirectionFromValue(dir_value);
        if (is_valid_move(gameState, my_player, dir)) {
          GameState newGameState = gameState;
          Player newMyPlayer = my_player;
          movePlayer(newGameState, newMyPlayer, dir);
          int eval = minimax(newGameState, depth - 1, false, alpha, beta);
          maxEval = std::max(maxEval, eval);
          alpha = std::max(alpha, eval);
          if (beta <= alpha)
            break;  // Alpha Beta Pruning
        }
      }
      return maxEval;
    } else {
      int minEval = std::numeric_limits<int>::max();
      // Try all possible moves for the minimizing player (opponent)
      for (int dir_value = 0; dir_value < 4; ++dir_value) {
        Direction dir = getDirectionFromValue(dir_value);
        if (is_valid_move(gameState, opponent, dir)) {
          GameState newGameState = gameState;
          Player newOpponent = opponent;
          movePlayer(newGameState, newOpponent, dir);
          int eval = minimax(newGameState, depth - 1, true, alpha, beta);
          minEval = std::min(minEval, eval);
          beta = std::min(beta, eval);
          if (beta <= alpha)
            break;  // Alpha Beta Pruning
        }
      }
      return minEval;
    }
  }

  bool game_over(const GameState& gameState) {
    // Simple game over check: no valid moves for either player
    bool my_moves = false, opp_moves = false;
    for (int dir_value = 0; dir_value < 4; ++dir_value) {
      Direction dir = getDirectionFromValue(dir_value);
      if (is_valid_move(gameState, my_player, dir)) my_moves = true;
      if (is_valid_move(gameState, opponent, dir)) opp_moves = true;
    }
    return !(my_moves || opp_moves);
  }

  void movePlayer(GameState& gameState, Player& player, Direction dir) {
    // Move the player in the given direction and update the game state
    sf::Vector2i new_pos = player.position + getDirectionVector(dir);
    gameState.setGridCell(new_pos, 1);  // Mark the grid cell as occupied
    player.position = new_pos;
  }

  Direction decideMove() {
    int bestScore = std::numeric_limits<int>::min();
    Direction bestMove = Direction::Up;  // Default move

    // Consider all valid moves and choose the best one using Minimax
    for (int dir_value = 0; dir_value < 4; ++dir_value) {
      Direction dir = getDirectionFromValue(dir_value);
      if (is_valid_move(state, my_player, dir)) {
        GameState newGameState = state;
        Player newMyPlayer = my_player;
        movePlayer(newGameState, newMyPlayer, dir);
        int score = minimax(newGameState, MAX_DEPTH - 1, false,
                            std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        // Spdlog::debug("{}: Direction {} has score {}", name, dir_value, score);
        if (score > bestScore) {
          bestScore = score;
          bestMove = dir;
        }
      }
    }

    // Spdlog::debug("{}: Selected direction {} with score {}", name,
    //               getDirectionValue(bestMove), bestScore);
    return bestMove;
  }

  void receiveGameState() {
    // Receive the current game state from the server
    state = connection.receiveGameState();
    for (const auto& player : state.players) {
      if (player.name == name) {
        my_player = player;  // Update our player's information
      } else {
        opponent = player;   // Update opponent's information
      }
    }
  }

  void sendMove() {
    // Decide on the next move and send it to the server
    // Spdlog::debug("{}: Sending move", name);
    auto move = decideMove();
    connection.sendMove(move);
  }

 public:
  SmartestBotClient(const std::string& botName) : name(botName) {
    // Constructor to initialize the bot client
    std::random_device rd;
    rng.seed(rd());
    connection.connect(name);
    if (!connection.isActive()) {
      // Failed to connect to the server
      // Spdlog::critical("{}: Connection failed", name);
      exit(1);
    }
  }

  void run() {
    // Main loop to keep the bot running while connected
    while (connection.isActive()) {
      receiveGameState();  // Get the latest game state
      sendMove();          // Send our move
    }
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    // Uasge error, need bot name
    std::cerr << "Usage: " << argv[0] << " <bot_name>" << std::endl;
    return 1;
  }
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
  spdlog::set_level(spdlog::level::debug);
#endif
  std::string botName = argv[1];
  SmartestBotClient bot(botName);
  bot.run();
  return 0;
}
