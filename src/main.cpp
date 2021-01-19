#include <Arduino.h>
#include <FastLED.h>
#include <Ship.h>
//#include <Encoder.h>

//Buttons
#define SHIP_PIN 5
#define ENCODER_SW_PIN 4
#define ENCODER_A_PIN 2
#define ENCODER_B_PIN 3
#define FUNCTION_PIN 7

// Pots
#define X_POT A2
#define Y_POT A3

//Led
#define DATA_PIN 6
#define NUM_LEDS 100

#define VERTICAL 1   // True
#define HORIZONTAL 0 // False
#define CHANGE_X 1
#define CHANGE_Y 0

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

// Vars

// Ship definitions
#define NUM_SHIPS 10
Ship ships[NUM_SHIPS] = {Ship(1), Ship(1), Ship(1), Ship(1), Ship(2), Ship(2), Ship(2), Ship(3), Ship(3), Ship(4)};
// Encoder my_encoder = (ENCODER_A_PIN, ENCODER_B_PIN);

void drawShips(Ship ships[])
{
  FastLED.clear();
  for (unsigned int i = 0; i < NUM_SHIPS; i++)
  {
    for (int u = 0; u < ships[i].size(); u++)
    {
      if (ships[i].orientation() == VERTICAL)
      {
        leds[translateToCRGB(ships[i].x, ships[i].y + u)] = SHIP_COLOR;
      }

      if (ships[i].orientation() == HORIZONTAL)
      {
        leds[translateToCRGB(ships[i].x + u, ships[i].y)] = SHIP_COLOR;
      }
    }
  }

  FastLED.show();
}

unsigned int current_ship = 0;
bool current_function = CHANGE_X;

void changeFunction()
{
  current_function = !current_function;
  Serial.print("Current function: ");
  Serial.println(current_function);
}

void changeShip()
{
  if (current_ship < NUM_SHIPS - 1)
  {
    current_ship++;
  }
  else
  {
    current_ship = 0;
  }
  Serial.print("Ship selected: ");
  Serial.println(current_ship);
}

void setup()
{
  Serial.begin(9600);
  pinMode(SHIP_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT);
  pinMode(FUNCTION_PIN, INPUT);
  pinMode(ENCODER_A_PIN, INPUT);
  pinMode(ENCODER_B_PIN, INPUT);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
  FastLED.setBrightness(20);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  ships[9].x = 5;

  drawShips(ships);
}

void loop()
{
  // Encoder function
  static uint16_t state = 0; //, counter = 0;
  state = (state << 1) | digitalRead(ENCODER_A_PIN) | 0xe000;

  if (state == 0xf000)
  {
    state = 0x0000;
    if (digitalRead(ENCODER_B_PIN))
    {
      ships[current_ship].changeLocation(+1, current_function);
      drawShips(ships);
    }
    //counter++;
    else
    {
      ships[current_ship].changeLocation(-1, current_function);
      drawShips(ships);
    }

    //counter--;
    //Serial.println(counter);
  }

  // Cycling through ships
  if (digitalRead(SHIP_PIN))
  {
    changeShip();
    delay(200);
  }

  if (digitalRead(FUNCTION_PIN))
  {
    changeFunction();
    delay(500);
  }

  /*   unsigned int loc_x = map(analogRead(X_POT), 0, 1023, 1, 11);
  unsigned int loc_y = map(analogRead(Y_POT), 0, 1023, 1, 11);
  ships[current_ship].x = loc_x;
  ships[current_ship].y = loc_y; */

  if (!digitalRead(ENCODER_SW_PIN))
  {
    Serial.println("Rotating ship");
    ships[current_ship].rotate();
    Serial.println(ships[current_ship].orientation());
    drawShips(ships);
    delay(200);
  }
}