#include "Arduino.h"
#include "Ship.h"

#define VERTICAL 1
#define HORIZONTAL 0

Ship::Ship(int size)
{
    x = 1;
    y = 1;
    _orientation = true;
    _size = size;
}

int Ship::max_loc()
{
    return 10 - _size + 1;
}

int Ship::size()
{
    return _size;
}

void Ship::rotate()
{

    if ((orientation() == VERTICAL) && (x <= max_loc()))
    {
        _orientation = HORIZONTAL;
        return;
    }

    if ((orientation() == HORIZONTAL) && (y <= max_loc()))
    {
        _orientation = VERTICAL;
        return;
    }
}

void Ship::changeLocation(int vector, bool axis)
{
    Serial.println("Changing ship position");
    Serial.print("Current position: X:");
    Serial.print(x);
    Serial.print(", Y:");
    Serial.println(y);

    // X
    if (axis && _isValidLocation(x + vector, y, _orientation))
    {
        x = x + vector;
        return;
    }
    // Y
    if (!axis && _isValidLocation(x, y + vector, _orientation))
    {
        y = y + vector;
        return;
    }
}

bool Ship::_isValidLocation(int new_x, int new_y, bool new_orientation)
{
    // Performing check for vertical
    if (new_orientation == VERTICAL)
    {
        if (new_x > 0 && new_x <= 10 &&
            new_y > 0 && new_y <= max_loc())
            return true;
    }
    // Performing check for horizontal
    else
    {
        if (new_x > 0 && new_x <= max_loc() &&
            new_y > 0 && new_y <= 10)
            return true;
    }

    return false;
}

bool Ship::orientation()
{
    return _orientation;
}