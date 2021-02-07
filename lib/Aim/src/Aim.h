#ifndef Aim_h
#define Aim_h

#include "Arduino.h"
class Aim
{
public:
    Aim();
    unsigned int x;
    unsigned int y;
    void changeLocation(int vector, bool axis);
};

#endif