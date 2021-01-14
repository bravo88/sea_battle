#include <Arduino.h>
#include <FastLED.h>

//Buttons
#define ROTATE_PIN 2
#define SHIP_PIN 3

// Pots
#define X_POT A2
#define Y_POT A3

//Led
#define DATA_PIN 6
#define NUM_LEDS 100

#define VERTICAL 1
#define HORIZONTAL 0

#define SHIP_COLOR CRGB(0x808080) // Beige
#define SEA_COLOR CRGB(0x00008B)  // Medium Aquamarine

CRGB leds[NUM_LEDS];

// FUNCTIONS
int translateToCRGB(int x, int y)
{
  //Serial.print("Translate. X: ");
  //Serial.print(x);
  //Serial.print(" Y: ");
  //Serial.print(y);

  int order;
  int led_number;
  // if even
  if (x % 2 == 0)
  {
    order = (x - 1) * 10 - 1;
    led_number = order + y;
  }
  // if odd
  else
  {
    order = x * 10;
    led_number = order - y;
  }
  //Serial.print(" = Led number: ");
  //Serial.println(led_number);
  return led_number;
}

void updateShip(int ship[])
{
  /* Serial.print("New x: ");
  Serial.print(ship[4]);
  Serial.print(", new y: ");
  Serial.print(ship[5]);
  Serial.print(", new orient: ");
  Serial.println(ship[6]);

  Serial.print("old x: ");
  Serial.print(ship[1]);
  Serial.print(", old y: ");
  Serial.print(ship[2]);
  Serial.print(", old orient: ");
  Serial.println(ship[3]); */

  // Setting blacks
  // Vertical
  if (ship[3] == VERTICAL)
  {
    for (int i = 0; i < ship[0]; i++)
    {
      leds[translateToCRGB(ship[1], ship[2] + i)] = SEA_COLOR;
    }
    // Horizontal
  }
  else
  {
    for (int i = 0; i < ship[0]; i++)
    {
      leds[translateToCRGB(ship[1] + i, ship[2])] = SEA_COLOR;
    }
  }

  // Setting new position

  if (ship[6] == VERTICAL)
  {
    Serial.println("Setting new whites - vertically");
    for (int i = 0; i < ship[0]; i++)
    {
      leds[translateToCRGB(ship[4], ship[5] + i)] = SHIP_COLOR;
    }
    // Horizontal
  }
  else
  {
    Serial.println("Setting new whites - horizontally");
    for (int i = 0; i < ship[0]; i++)
    {
      leds[translateToCRGB(ship[4] + i, ship[5])] = SHIP_COLOR;
    }
  }

  // Updating ship object
  ship[1] = ship[4];
  ship[2] = ship[5];
  ship[3] = ship[6];

  FastLED.show();
}

void shiftRow(int ship[], int vector)
{

  if ((ship[3] == VERTICAL) && ((ship[0] + ship[2] - 1 + vector > 10) || (ship[2] + vector < 1)))
  {
    return;
  }

  if ((ship[3] == HORIZONTAL) && ((ship[2] + vector <= 0) || (ship[2] + vector > 10)))
  {
    return;
  }

  ship[5] = ship[2] + vector;

  updateShip(ship);
}

void shiftColumn(int ship[], int vector)
{
  // Horizontal
  if ((ship[3] == HORIZONTAL) && ((ship[1] + vector <= 0) || (ship[1] + ship[0] - 1 + vector > 10)))
  {
    return;
  }

  //Vertical
  if ((ship[3] == VERTICAL) && ((ship[1] + vector <= 0) || (ship[1] + vector > 10)))
  {
    return;
  }

  ship[4] = ship[1] + vector;
  updateShip(ship);
}

void ShiftOrientation(int ship[])
{
  // Vertical to horizontal
  if (ship[3] == VERTICAL)
  {
    Serial.println("Vertical to Horizontal");
    if (ship[0] + ship[1] - 1 <= 10)
    {
      ship[6] = HORIZONTAL;
    }
  }
  // Horizontal to vertical
  else
  {
    Serial.println("Horizontal to Vertical");
    if (ship[0] + ship[2] - 1 <= 10)
    {
      Serial.println("Space check pass");
      ship[6] = VERTICAL;
    }
  }
  updateShip(ship);
}

// Vars

// [number][size, x, y, orient, new x, new y, new orient (0-horizont, 1-vertical)]
int ship[10][7] = {
    {1, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 0
    {1, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 1
    {1, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 2
    {1, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 3

    {2, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 4
    {2, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 5
    {2, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 6

    {3, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 7
    {3, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 8

    {4, 1, 8, HORIZONTAL, 1, 8, VERTICAL}, // 9
};

unsigned int current_ship = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(ROTATE_PIN, INPUT);
  pinMode(X_POT, INPUT);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
  FastLED.setBrightness(20);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = SEA_COLOR;
  }

  updateShip(ship[current_ship]);
  FastLED.show();
}

void loop()
{
  unsigned int loc_x = map(analogRead(X_POT), 0, 1023, 1, 11);
  unsigned int loc_y = map(analogRead(Y_POT), 0, 1023, 1, 11);
  ship[current_ship][4] = loc_x;
  ship[current_ship][4] = loc_y;

  if (digitalRead(ROTATE_PIN))
  {
    if (ship[3] == HORIZONTAL)
    {
      ship[current_ship][6] = VERTICAL;
    }
    else
    {
      ship[current_ship][6] = HORIZONTAL;
    }
    delay(200);
  }

  if (digitalRead(SHIP_PIN))
  {
    if(current_ship<10){
      current_ship++;
    }else{
      current_ship=0;
    }
    delay(200);
  }

  updateShip(ship[current_ship]);
}