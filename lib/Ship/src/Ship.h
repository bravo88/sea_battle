#ifndef Ship_h
#define Ship_h

#include "Arduino.h"
class Ship
{
    public:
        Ship(int size);
        int x;
        int y;
        int max_loc();
        int size();
        bool orientation();
        void rotate();
        void changeLocation(int vector, bool axis);
    private:
        int _size;
        int _orientation;
        bool _isValidLocation(int new_x, int new_y, bool new_orientation);
        
};

#endif