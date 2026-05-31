#include "tetris.hpp"

#include <ranges>
#include <algorithm>
#include <cstring>

namespace T20 {
    
    TetrisGameBase::TetrisGameBase() {
        std::ranges::fill(board, 0);
        gameDataMap = {tetris_game_data.data(), tetris_trit_size};
        std::uniform_int_distribution<> distribution(0, tetris_trit_size - 1);
        tritPosition = {2, 0};
        std::default_random_engine tritGenEng{tritGenDev()};
        tritList[0] = static_cast<std::uint8_t>(distribution(tritGenEng));
        tritList[1] = static_cast<std::uint8_t>(distribution(tritGenEng));
        Trit_Load();
    }

    void TetrisGameBase::operator()() {
        while (running) {
            if (OnShouldTickLogic()) {
                if (Trit_HitObstacle()) {
                    HandleLines();
                    if (running) {
                        Trit_Spawn();
                    }
                } else {
                    Trit_Sink();
                }
            }
            OnUpdate();
        }
    }

    std::uint8_t TetrisGameBase::GetCellAt_(int x, int y, uint8_t angle) const {
        const int ox[4] = { x, y, tritWH[0] - 1 - x, tritWH[0] - 1 - y };
        const int oy[4] = { y, tritWH[1] - 1 - x, tritWH[1] - 1 - y, x  };
        return tritData[oy[angle] * tritWH[0] + ox[angle]];
    }
    
    bool TetrisGameBase::CheckCollision_(const std::array<uint8_t,2>& pos, uint8_t angle) const {
        uint8_t rot_w = (angle & 1) ? tritWH[1] : tritWH[0];
        uint8_t rot_h = (angle & 1) ? tritWH[0] : tritWH[1];
        
        for (uint8_t y = 0; y < rot_h; ++y) {
            for (uint8_t x = 0; x < rot_w; ++x) {
                if (GetCellAt_(x, y, angle)) {
                    uint8_t board_x = pos[0] + x;
                    uint8_t board_y = pos[1] + y;
                    if (board_x >= tetris_game_width || board_y >= tetris_game_height)
                        return true;
                    if (board[board_y * tetris_game_width + board_x] != 0)
                        return true;
                }
            }
        }
        return false;
    }
    
    auto TetrisGameBase::GetBoard_Merged() -> decltype(board) {
        auto mergedBoard = board;
        uint8_t rot_w = (tritAngle & 1) ? tritWH[1] : tritWH[0];
        uint8_t rot_h = (tritAngle & 1) ? tritWH[0] : tritWH[1];
        
        for (uint8_t y = 0; y < rot_h; ++y) {
            for (uint8_t x = 0; x < rot_w; ++x) {
                if (auto v = GetCellAt_(x, y, tritAngle); v) {
                    uint8_t board_x = tritPosition[0] + x;
                    uint8_t board_y = tritPosition[1] + y;
                    if (board_x < tetris_game_width && board_y < tetris_game_height) {
                        mergedBoard[board_y * tetris_game_width + board_x] = v;
                    }
                }
            }
        }
        return mergedBoard;
    }

    auto TetrisGameBase::GetBoard_Render() -> std::array<std::uint8_t, (tetris_game_width + 2) * (tetris_game_height + 2)> {
        auto mb = GetBoard_Merged();
        std::array<std::uint8_t, (tetris_game_width + 2) * (tetris_game_height + 2)> rb{}; rb.fill(8);
        for (std::size_t i = 0; i != tetris_game_height; ++i) {
            for (std::size_t j = 0; j != tetris_game_width; ++j) {
                rb[(i + 1) * (tetris_game_width + 2) + j + 1] = mb[i * tetris_game_width + j];
            }
        }
        return rb;
    }

    void TetrisGameBase::Trit_Load() {
        uint8_t offset = gameDataMap[tritList[0]];
        tritWH = {tetris_game_data[offset], tetris_game_data[offset + 1]};
        offset += 2;
        tritData  = std::span{&tetris_game_data[offset], static_cast<std::size_t>(tritWH[0] * tritWH[1])};
        tritAngle = 0;
    }

    void TetrisGameBase::Trit_Spawn() {
        tritPosition = {2, 0};
        tritList[0] = tritList[1];
        std::default_random_engine tritGenEng{tritGenDev()};
        tritList[1] = static_cast<std::uint8_t>(std::uniform_int_distribution<>(0, tetris_trit_size - 1)(tritGenEng));
        Trit_Load();
        if (CheckCollision_(tritPosition, tritAngle)) {
            running = false;
        }
    }
    
    bool TetrisGameBase::Trit_HitObstacle() {
        std::array<uint8_t,2> downPos = tritPosition;
        downPos[1] += 1;
        if (CheckCollision_(downPos, tritAngle)) {
            uint8_t rot_w = (tritAngle & 1) ? tritWH[1] : tritWH[0];
            uint8_t rot_h = (tritAngle & 1) ? tritWH[0] : tritWH[1];
            for (uint8_t y = 0; y < rot_h; ++y) {
                for (uint8_t x = 0; x < rot_w; ++x) {
                    if (auto v = GetCellAt_(x, y, tritAngle); v) {
                        uint8_t board_x = tritPosition[0] + x;
                        uint8_t board_y = tritPosition[1] + y;
                        if (board_x < tetris_game_width && board_y < tetris_game_height) {
                            board[board_y * tetris_game_width + board_x] = v;
                        }
                    }
                }
            }
            return true;
        }
        return false;
    }
    
    void TetrisGameBase::Trit_Sink() {
        std::array<uint8_t,2> newPos = tritPosition;
        newPos[1] += 1;
        if (!CheckCollision_(newPos, tritAngle)) {
            tritPosition = newPos;
        }
    }

    void TetrisGameBase::Trit_Rotate() {
        uint8_t newAngle = (tritAngle + 1) & 3;
        uint8_t new_w = (newAngle & 1) ? tritWH[1] : tritWH[0];
        uint8_t new_h = (newAngle & 1) ? tritWH[0] : tritWH[1];
        
        std::array<uint8_t,2> newPos = tritPosition;
        newPos[0] = std::clamp<uint8_t>(newPos[0], 0, tetris_game_width - new_w);
        newPos[1] = std::clamp<uint8_t>(newPos[1], 0, tetris_game_height - new_h);
        
        if (!CheckCollision_(newPos, newAngle)) {
            tritAngle = newAngle;
            tritPosition = newPos;
        }
    }

    void TetrisGameBase::Trit_Shift(uint8_t dir) {
        std::array<uint8_t,2> newPos = tritPosition;
        newPos[dir & 1] = newPos[dir & 1] - (dir == 0) + (dir != 0);
        uint8_t cur_w = (tritAngle & 1) ? tritWH[1] : tritWH[0];
        uint8_t cur_h = (tritAngle & 1) ? tritWH[0] : tritWH[1];
        newPos[0] = std::clamp<uint8_t>(newPos[0] + 1, 1, tetris_game_width - cur_w + 1) - 1;
        newPos[1] = std::clamp<uint8_t>(newPos[1], 0, tetris_game_height - cur_h);
        if (!CheckCollision_(newPos, tritAngle)) {
            tritPosition = newPos;
        }
    }

    void TetrisGameBase::HandleLines() {
        std::span topLine{board.data(), tetris_game_width};
        if (std::ranges::any_of(topLine, [](uint8_t c) { return c != 0; })) {
            running = false;
            return;
        }
        for (std::size_t i = 0; i < tetris_game_height; ++i) {
            std::span line{&board[i * tetris_game_width], tetris_game_width};
            while (std::ranges::all_of(line, [](uint8_t c) { return c != 0; })) {
                for (std::size_t r = i; r > 0; --r) {
                    std::memcpy(&board[r * tetris_game_width],
                                &board[(r-1) * tetris_game_width],
                                tetris_game_width);
                }
                std::memset(board.data(), 0, tetris_game_width);
                ++linesCleared;
            }
        }
    }
    
}