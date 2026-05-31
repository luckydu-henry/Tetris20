#include <chrono>
#include <cstdint>
#include <cstddef>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#endif

#include "tetris.hpp"

namespace T20 {

    class TetrisGameConsole final : public TetrisGameBase {
    public:
        float                                          time = 0.F;
        std::chrono::high_resolution_clock::time_point start_time;
        bool                                           cursorHidden = false;

        TetrisGameConsole() : TetrisGameBase() {
            start_time = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
            std::system("cls");
#else
            std::system("clear");
#endif
        }
        
        ~TetrisGameConsole() override {
            // show cursor again.
            std::cout << std::endl << "game over!" << std::endl;
            std::cout << "\033[?25h" << std::endl;
        }

        void UpdateTimer() {
            auto end_time = std::chrono::high_resolution_clock::now();
            time += std::chrono::duration<float>(end_time - start_time).count();
            start_time = end_time;
        }

        void GetInput();

        void Render() {
            
            if (!cursorHidden) {
                std::cout << "\033[?25l";   // hide cursor
                cursorHidden = true;
            }
            
            constexpr char TetrisIndexToChar[] = { ' ', 'O', 'I', 'S', 'Z', 'L', 'J', 'T', '*' };
            
            auto rb = GetBoard_Render();
            for (auto& c : rb) {
                c = TetrisIndexToChar[c];
            }
            
            std::cout << "\033[H";

            for (std::size_t i = 0; i < tetris_game_height + 2; ++i) {
                std::cout.write(reinterpret_cast<const char*>(&rb[i * (tetris_game_width + 2)]),tetris_game_width + 2);
                std::cout.put('\n');
            }
            // clear line remained.
            std::cout << "\033[" << tetris_game_height + 3 << ";1H\033[K";
            std::cout << "lines cleared: " << linesCleared;
            std::cout << "\033[" << tetris_game_height + 4 << ";1H\033[K";
            std::cout << "next         : " << TetrisIndexToChar[tritList[1] + 1];
        }

        void OnUpdate() final {
            UpdateTimer();
            GetInput();
            Render();
        }

        bool OnShouldTickLogic() final {
            if (time >= 1.F) {
                time -= 1.F;
                return true;
            }
            return false;
        }
    };

    void TetrisGameConsole::GetInput() {
#ifdef _WIN32
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
            case 'a': Trit_Shift(0); break;
            case 's': Trit_Shift(1); break;
            case 'd': Trit_Shift(2); break;
            case 'w': Trit_Rotate();    break;
            default:  break;
            }
        }
#endif
    }

}

int main() {
    T20::TetrisGameConsole tetrisConsole;
    tetrisConsole();
    return 0;
}