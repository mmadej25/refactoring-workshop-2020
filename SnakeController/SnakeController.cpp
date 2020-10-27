#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "IPort.hpp"

namespace Snake
{
ConfigurationError::ConfigurationError ()
    : std::logic_error ("Bad configuration of Snake::Controller.")
{
}

UnexpectedEventException::UnexpectedEventException ()
    : std::runtime_error ("Unexpected event received!")
{
}

Controller::Controller (IPort &p_displayPort, IPort &p_foodPort,
                        IPort &p_scorePort, std::string const &p_config)
    : m_displayPort (p_displayPort), m_foodPort (p_foodPort),
      m_scorePort (p_scorePort)
{
  std::istringstream istr (p_config);
  char w, f, s, d;

  int width, height, length;
  int foodX, foodY;
  istr >> w >> width >> height >> f >> foodX >> foodY >> s;

  if (w == 'W' and f == 'F' and s == 'S')
    {
      m_mapDimension = std::make_pair (width, height);
      m_foodPosition = { foodX, foodY };

      istr >> d;
      switch (d)
        {
        case 'U':
          m_currentDirection = Direction_UP;
          break;
        case 'D':
          m_currentDirection = Direction_DOWN;
          break;
        case 'L':
          m_currentDirection = Direction_LEFT;
          break;
        case 'R':
          m_currentDirection = Direction_RIGHT;
          break;
        default:
          throw ConfigurationError ();
        }
      istr >> length;

      while (length)
        {
          Segment seg;
          istr >> seg.x >> seg.y;
          seg.ttl = length--;

          m_segments.push_back (seg);
        }
    }
  else
    {
      throw ConfigurationError ();
    }
}

void
Controller::receive (std::unique_ptr<Event> event)
{
  try
    {
      handleTimeEvent (event);
    }
  catch (std::bad_cast &)
    {
      handleDirectionChange (event);
    }
}

void
Controller::handleTimeEvent (std::unique_ptr<Event> &event)
{
  auto const &timerEvent = *dynamic_cast<EventT<TimeoutInd> const &> (*event);

  Segment newHead = createNewHead ();

  bool lostInd = isCollideWithSnake (newHead) or isOutOfMap (newHead);

  if (lostInd)
    m_scorePort.send (std::make_unique<EventT<LooseInd> > ());
  else
    {
      eatOrMove (newHead);
      updateSnake (newHead);
    }
}

void
Controller::eatOrMove (Segment &newHead)
{
  if (isOnFood (newHead))
    {
      m_scorePort.send (std::make_unique<EventT<ScoreInd> > ());
      m_foodPort.send (std::make_unique<EventT<FoodReq> > ());
    }
  else
    moveSnake ();
}

void
Controller::moveSnake ()
{
  auto freeSegment = [this](Segment &segment) {
    if (not--segment.ttl)
      m_displayPort.send (std::make_unique<EventT<DisplayInd> > (
          calculateDisplayInd (segment, Cell_FREE)));
  };
  std::for_each (m_segments.begin (), m_segments.end (), freeSegment);
}

void
Controller::updateSnake (Segment &newHead)
{
  m_segments.push_front (newHead);

  m_displayPort.send (std::make_unique<EventT<DisplayInd> > (
      calculateDisplayInd (newHead, Cell_SNAKE)));
  updateSegments ();
}

void
Controller::updateSegments ()
{
  auto lambda_remove
      = [](auto const &segment) { return not(segment.ttl > 0); };

  m_segments.erase (
      std::remove_if (m_segments.begin (), m_segments.end (), lambda_remove),
      m_segments.end ());
}

void
Controller::handleDirectionChange (std::unique_ptr<Event> &event)
{

  try
    {
      auto direction
          = dynamic_cast<EventT<DirectionInd> const &> (*event)->direction;

      if ((m_currentDirection & Direction_LEFT)
          != (direction & Direction_LEFT))
        {
          m_currentDirection = direction;
        }
    }
  catch (std::bad_cast &)
    {
      handleReciveFood (event);
    }
}

void
Controller::handleReciveFood (std::unique_ptr<Event> &event)
{
  try
    {
      auto receivedFood = *dynamic_cast<EventT<FoodInd> const &> (*event);
      bool requestedFoodCollidedWithSnake{ isFoodCollideWithSnake (
          receivedFood) };

      if (requestedFoodCollidedWithSnake)
        {
          m_foodPort.send (std::make_unique<EventT<FoodReq> > ());
        }
      else
        {
          m_displayPort.send (std::make_unique<EventT<DisplayInd> > (
              calculateDisplayInd (m_foodPosition, Cell_FREE)));

          m_displayPort.send (std::make_unique<EventT<DisplayInd> > (
              calculateDisplayInd (receivedFood, Cell_FOOD)));
        }

      m_foodPosition = receivedFood;
    }
  catch (std::bad_cast &)
    {
      try
        {
          handleRequestedFood (event);
        }
      catch (std::bad_cast &)
        {
          throw UnexpectedEventException ();
        }
    }
}

void
Controller::handleRequestedFood (std::unique_ptr<Event> &event)
{
  auto requestedFood = *dynamic_cast<EventT<FoodResp> const &> (*event);

  bool requestedFoodCollidedWithSnake{ isFoodCollideWithSnake (
      requestedFood) };

  if (requestedFoodCollidedWithSnake)
    {
      m_foodPort.send (std::make_unique<EventT<FoodReq> > ());
    }
  else
    {

      m_displayPort.send (std::make_unique<EventT<DisplayInd> > (
          calculateDisplayInd (requestedFood, Cell_FOOD)));
    }

  m_foodPosition = requestedFood;
}

bool
Controller::isFoodCollideWithSnake (Coordinate foodCoordinate)
{
  for (auto const &segment : m_segments)
    {
      if (segment == foodCoordinate)
        {
          return true;
          break;
        }
    }
  return false;
}

DisplayInd
Controller::calculateDisplayInd (Coordinate coordinate, Cell cellCategory)
{
  DisplayInd placeNewFood;
  placeNewFood.x = coordinate.x;
  placeNewFood.y = coordinate.y;
  placeNewFood.value = cellCategory;
  return placeNewFood;
}

Segment
Controller::createNewHead ()
{
  Segment const &currentHead = m_segments.front ();
  Segment newHead;
  newHead.x = currentHead.x
              + ((m_currentDirection bitand Direction_LEFT)
                     ? (m_currentDirection bitand Direction_DOWN) ? 1 : -1
                     : 0);
  newHead.y = currentHead.y
              + (not(m_currentDirection bitand Direction_LEFT)
                     ? (m_currentDirection bitand Direction_DOWN) ? 1 : -1
                     : 0);
  newHead.ttl = currentHead.ttl;
  return newHead;
}

bool
Controller::isCollideWithSnake (Segment newSegment)
{
  auto collision
      = std::find (m_segments.begin (), m_segments.end (), newSegment);
  return (collision != m_segments.end ());
}

bool
Controller::isOutOfMap (Segment newSegment)
{
  return (newSegment.x < 0 or newSegment.y < 0
          or newSegment.x >= m_mapDimension.first
          or newSegment.y >= m_mapDimension.second);
}

bool
Controller::isOnFood (Segment &newHead)
{
  return newHead == m_foodPosition;
}

bool
operator== (const Segment &segment, const Coordinate &coordinate)
{
  return ((segment.x == coordinate.x) and (segment.y == coordinate.y));
}

} // namespace Snake
