#ifndef Ship_h
#define Ship_h

#include "Arduino.h"
class Ship
{
public:
    Ship(int size);
    unsigned int x;
    unsigned int y;
    unsigned int max_loc();
    int size();
    bool orientation();
    void rotate();
    void changeLocation(int vector, bool axis);
    bool visible;
    bool compartment_hit[4];
    bool isSunk();

private:
    int _size;
    int _orientation;
    bool _isValidLocation(unsigned int new_x, unsigned int new_y, bool new_orientation);
};

#endif