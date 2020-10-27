#pragma once

#include <list>
#include <memory>
#include <algorithm>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"
#include "EventT.hpp"

class Event;
class IPort;


namespace Snake
{
using MapDimension =std::pair<int,int>;
using Segments=std::list<Segment> ;

struct ConfigurationError : std::logic_error
{
    ConfigurationError();
};

struct UnexpectedEventException : std::runtime_error
{
    UnexpectedEventException();
};

class Controller : public IEventHandler
{
public:
    Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config);

    Controller(Controller const& p_rhs) = delete;
    Controller& operator=(Controller const& p_rhs) = delete;

    void receive(std::unique_ptr<Event> e) override;

private:
    

    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;

    MapDimension m_mapDimension;
    Coordinate m_foodPosition;

    Direction m_currentDirection;
    Segments m_segments;
    
    void updateSnake(Segment& newHead);
    void eatOrMove(Segment& newHead);
    void handleTimeEvent(std::unique_ptr<Event> &event);
    void handleReciveFood(std::unique_ptr<Event> &event);
    void handleRequestedFood(std::unique_ptr<Event> &event);
    void handleDirectionChange(std::unique_ptr<Event> &event);
    bool isFoodCollideWithSnake(Coordinate foodCoordinate);
    DisplayInd calculateDisplayInd(Coordinate coordinate,Cell cellCategory);
    Segment createNewHead();
    bool isCollideWithSnake(Segment newSegment);
    bool isOutOfMap(Segment newSegment);
    bool isOnFood(Segment& newHead);
    void moveSnake();
    void updateSegments();
};

} // namespace Snake
