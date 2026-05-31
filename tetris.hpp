#pragma once

#include <span>
#include <array>
#include <random>
#include <cstring>
#include <algorithm>

namespace T20 {
    
    static constexpr std::size_t                  tetris_game_width      = 10;
    static constexpr std::size_t                  tetris_game_height     = 20;
    static constexpr std::size_t                  tetris_trit_size       = 7;
    static constexpr std::array<std::uint8_t, 59> tetris_game_data       = {
        // offset map,
        7, 13, 19, 27, 35, 43, 51,
        
        // data definitions.
        // bounding with (1 byte), bounding height (1 byte), bounding data (width * height bytes)
        
        // O
        2, 2,
        1, 1,
        1, 1,
        
        // I
        4, 1,
        2, 2, 2, 2,
        
        // S
        3, 2,
        0, 3, 3,
        3, 3, 0,
        
        // Z
        3, 2,
        4, 4, 0,
        0, 4, 4,
        
        // L 
        2, 3,
        5, 0,
        5, 0,
        5, 5,
        
        // J
        2, 3,
        0, 6,
        0, 6,
        6, 6,
        
        // T
        3, 2,
        7, 7, 7,
        0, 7, 0
    };
    
    class TetrisGameBase {
    public:
        bool                                                                                    running{true};
        std::array <std::uint8_t, tetris_game_width * tetris_game_height>                       board{};
        std::span  <const std::uint8_t, std::dynamic_extent>                                    gameDataMap{};
        
        std::span  <const std::uint8_t, std::dynamic_extent>                                    tritData{};
        std::array <std::uint8_t, 2>                                                            tritWH;      // [width, height] 原始方向尺寸
        std::uint8_t                                                                            tritAngle;
        
        std::array <std::uint8_t, 2>                                                            tritList{};
        std::array <std::uint8_t, 2>                                                            tritPosition{};
        std::random_device                                                                      tritGenDev{};
        std::size_t                                                                             linesCleared{0};
        

        TetrisGameBase();
        TetrisGameBase(const TetrisGameBase&)               = delete;
        TetrisGameBase(TetrisGameBase&&)                    = delete;
        TetrisGameBase& operator=(const TetrisGameBase&)    = delete;
        TetrisGameBase& operator=(TetrisGameBase&&)         = delete;
        virtual ~TetrisGameBase()                           = default;
        
        
        // transplant methods.
        virtual void      OnUpdate()            = 0;
        virtual bool      OnShouldTickLogic()        = 0;
        
        void              operator()();
        auto              GetBoard_Merged() -> decltype(board);
        auto              GetBoard_Render() -> std::array<std::uint8_t, (tetris_game_width + 2) * (tetris_game_height + 2)>;
        
        void              Trit_Load();
        void              Trit_Spawn();
        bool              Trit_HitObstacle();
        void              Trit_Sink();
        void              Trit_Rotate();
        void              Trit_Shift (std::uint8_t dir);
        void              HandleLines();

    private:
        std::uint8_t GetCellAt_(int x, int y, uint8_t angle) const;
        bool         CheckCollision_(const std::array<uint8_t,2>& pos, uint8_t angle) const;
    };
}