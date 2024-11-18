#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "connection.h"

using namespace std;

class CyclesBot {
public:
    CyclesBot(cycles::Connection& connection) : conn(connection) {}

    void play() {
        // Read the initial game state from the server
        while (true) {
            cycles::GameState state = conn.receiveGameState();
            if (state.gameOver) {
                cout << "Game Over! Exiting..." << endl;
                break;
            }

            // Custom logic to determine the bot's next move
            cycles::Move move = determineNextMove(state);

            // Send the move to the server
            conn.sendMove(move);
        }
    }

private:
    cycles::Connection& conn;

    // Define your bot's strategy here
    cycles::Move determineNextMove(const cycles::GameState& state) {
        cycles::Move move;

        // Example logic: Choose a random valid move
        if (!state.validMoves.empty()) {
            move = state.validMoves[rand() % state.validMoves.size()];
        } else {
            move = {0, 0};  // Default move if no valid moves are found
        }

        return move;
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));  // Seed for random number generation

    try {
        cycles::Connection conn("localhost", "3490");  // Connect to the game server
        CyclesBot bot(conn);
        bot.play();
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

