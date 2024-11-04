#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>
#include <chrono>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        // Inicializar o tabuleiro com espaços vazios
        for (auto &row : board)
            row.fill(' ');
    }

    void display_board() {
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto &row : board) {
            for (char cell : row) {
                std::cout << (cell == ' ' ? '.' : cell) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [this, player] { return current_player == player && !game_over; });

        // Verificar se a posição está livre e fazer a jogada
        if (board[row][col] == ' ') {
            board[row][col] = player;
            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D';
            } else {
                // Alternar o jogador
                current_player = (current_player == 'X') ? 'O' : 'X';
            }
            display_board();
            turn_cv.notify_all(); // Notificar o próximo jogador
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        // Verificar linhas, colunas e diagonais
        for (int i = 0; i < 3; ++i) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
            if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
        }
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;
        return false;
    }

    bool check_draw() {
        for (const auto &row : board)
            for (char cell : row)
                if (cell == ' ')
                    return false;
        return true;
    }

    bool is_game_over() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return game_over;
    }

    char get_winner() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        }
    }

private:
    void play_sequential() {
        for (int i = 0; i < 3 && !game.is_game_over(); ++i) {
            for (int j = 0; j < 3 && !game.is_game_over(); ++j) {
                if (game.make_move(symbol, i, j)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Pausa para simular tempo de processamento
                }
            }
        }
    }

    void play_random() {
        std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 2);

        while (!game.is_game_over()) {
            int row = distribution(generator);
            int col = distribution(generator);
            if (game.make_move(symbol, row, col)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Pausa para simular tempo de processamento
            }
        }
    }
};

// Função principal
int main() {
    TicTacToe game;

    // Criar jogadores com estratégias diferentes
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    if (game.get_winner() == 'D') {
        std::cout << "O jogo terminou em empate!" << std::endl;
    } else {
        std::cout << "O vencedor é: " << game.get_winner() << std::endl;
    }

    return 0;
}
