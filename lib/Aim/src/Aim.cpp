#include "Arduino.h"
#include "Aim.h"

Aim::Aim()
{
    x = 5;
    y = 5;
}

void Aim::changeLocation(int vector, bool axis)
{
    
    // X
    if (axis && (x + vector <= 10) && (x + vector > 0))
    {
        Serial.println("Changing aim x");
        x = x + vector;
            Serial.print("X");
    Serial.print(x);
    Serial.print("Y");
    Serial.println(y);
        return;
    }
    // Y
    if (!axis && (y + vector <= 10) && (y + vector > 0))
    {
        Serial.println("Changing aim y");
        y = y + vector;
            Serial.print("X");
    Serial.print(x);
    Serial.print("Y");
    Serial.println(y);
        return;
    }

}