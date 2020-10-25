#pragma once

#include <cstdint>

namespace Snake
{

enum Direction
{
    Direction_UP    = 0b00,
    Direction_DOWN  = 0b10,
    Direction_LEFT  = 0b01,
    Direction_RIGHT = 0b11
};

struct Coordinate{
    int x;
    int y;
};

struct Segment :Coordinate
    {
        int ttl;
    };

struct DirectionInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x10;

    Direction direction;
};


struct TimeoutInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x20;
};

enum Cell
{
    Cell_FREE,
    Cell_FOOD,
    Cell_SNAKE
};

struct DisplayInd : Coordinate
{
    static constexpr std::uint32_t MESSAGE_ID = 0x30;

    Cell value;
};

struct FoodInd : Coordinate
{
    static constexpr std::uint32_t MESSAGE_ID = 0x40;

};

struct FoodReq
{
    static constexpr std::uint32_t MESSAGE_ID = 0x41;
};

struct FoodResp : Coordinate
{
    static constexpr std::uint32_t MESSAGE_ID = 0x42;

};

struct ScoreInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x70;
};

struct LooseInd
{
    static constexpr std::uint32_t MESSAGE_ID = 0x71;
};

} // namespace Snake
