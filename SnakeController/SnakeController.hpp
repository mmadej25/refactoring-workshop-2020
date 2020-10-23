#pragma once

#include <list>
#include <memory>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"
#include "EventT.hpp"

class Event;
class IPort;

namespace Snake
{
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
    struct Segment
    {
        int x;
        int y;
        int ttl;
    };

    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;

    std::pair<int, int> m_mapDimension;
    std::pair<int, int> m_foodPosition;

    Direction m_currentDirection;
    std::list<Segment> m_segments;
    void displayNewHead(Segment& newHead);
    void updateScorePort(Segment& newHead, bool &lost);
    void handleTimeEvent(std::unique_ptr<Event> &event);
    void handleReciveFood(std::unique_ptr<Event> &event);
    void handleRequestedFood(std::unique_ptr<Event> &event);
    void handleDirectionChange(std::unique_ptr<Event> &event);
};

} // namespace Snake
