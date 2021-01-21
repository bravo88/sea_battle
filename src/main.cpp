// Redesign blink of burning ship
// Rename function to "Ping"

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

//Led strip
#define DATA_PIN 6
#define NUM_LEDS 100

#define VERTICAL 1   // True
#define HORIZONTAL 0 // False
#define CHANGE_X 1
#define CHANGE_Y 0

#define SHIP_COLOR CRGB(0x808080)       // Beige
#define SEA_COLOR CRGB(0x00008B)        // Medium Aquamarine
#define HIT_COLOR_1 CRGB(0xFF0000);     // Red
#define HIT_COLOR_2 CRGB(0xFFA500);     // Orange
#define COLLISION_COLOR CRGB(0xFF0000); //Red
#define ACTIVE_COLOR CRGB(0x228B22);    //Forest green

CRGB hit_color = HIT_COLOR_1;
CRGB hit_color_1 = HIT_COLOR_1;
CRGB hit_color_2 = HIT_COLOR_2;
CRGB leds[NUM_LEDS];

const unsigned long blinkInterval = 500;
unsigned long previousTime = 0;

// FUNCTIONS
int translateToCRGB(int x, int y)
{
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
  return led_number;
}

// Ship definitions
#define NUM_SHIPS 10
Ship ships[NUM_SHIPS] = {Ship(1), Ship(1), Ship(1), Ship(1), Ship(2), Ship(2), Ship(2), Ship(3), Ship(3), Ship(4)};

//bool ping(unsigned int x, unsigned int y, int ship = -1);
bool ping(unsigned int x, unsigned int y, int ship = -1)
{
  // CHECKING FOR COLLISIONS WITH OTHER SHIPS
  for (int i = 0; i < NUM_SHIPS; i++)
  {
    if (i == ship || !ships[i].visible)
      continue;
    //Serial.print("Checking collision with ship no.");
    //Serial.println(i);
    for (int u = 0; u < ships[i].size(); u++)
    {
      //Serial.print("Ships x=");
      //Serial.print(ships[i].x);
      //Serial.print(", y=");
      //Serial.println(ships[i].y + u);

      // Target ship orientation vertical
      if (ships[i].orientation() == VERTICAL)
      {

        if ((ships[i].x == x) && (ships[i].y + u == y))
        {
          //Serial.println("Collision detected!!!");
          return true;
        }
      }

      // Target ship orientation horizontal
      if (ships[i].orientation() == HORIZONTAL)
      {
        if ((ships[i].y == y) && (ships[i].x + u == x))
        {
          //Serial.println("Collision detected!!!");
          return true;
        }
      }
    }
  }
  //Serial.println("No collisions");
  return false;
}

bool checkCollisions(int ship)
{
  //Serial.println("------------------------");
  //Serial.println("Checking for collisions");
  if (ships[ship].orientation() == VERTICAL)
  {
    //Serial.println("Ships orientation is vertical");
    // DEFINING SEARCH SCOPE
    //Serial.println("Defining search scope");
    for (unsigned int y = ships[ship].y - 1; y <= ships[ship].y + ships[ship].size(); y++)
    {
      if (y == 0)
        y = 1;
      if (y > 10)
        return false;
      for (unsigned int x = ships[ship].x - 1; x <= ships[ship].x + 1; x++)
      {
        if (x == 0)
          x = 1;
        if (x > 10)
          continue;
        //Serial.print("Looking in y=");
        //Serial.print(y);
        //Serial.print(", x=");
        //Serial.println(x);
        if (ping(x, y, ship))
        {
          return true;
        }
      }
    }
  }

  if (ships[ship].orientation() == HORIZONTAL)
  {
    //Serial.println("Ships orientation is vertical");
    // DEFINING SEARCH SCOPE
    //Serial.println("Defining search scope");
    for (unsigned int x = ships[ship].x - 1; x <= ships[ship].x + ships[ship].size(); x++)
    {
      if (x == 0)
        x = 1;
      if (x > 10)
        return false;
      for (unsigned int y = ships[ship].y - 1; y <= ships[ship].y + 1; y++)
      {
        if (y == 0)
          y = 1;
        if (y > 10)
          continue;
        //Serial.print("Looking in y=");
        //Serial.print(y);
        //Serial.print(", x=");
        //Serial.println(x);

        // CHECKING FOR COLLISIONS WITH OTHER SHIPS
        if (ping(x, y, ship))
        {
          return true;
        }
      }
    }
  }
  return false;
}
unsigned int current_ship = 0;

void drawShips(Ship ships[])
{
  FastLED.clear();
  for (unsigned int i = 0; i < NUM_SHIPS; i++)
  {
    if (!ships[i].visible)
      continue;

    CRGB color = SHIP_COLOR;

    if (i == current_ship)
    {
      color = ACTIVE_COLOR;
    }

    if (checkCollisions(i))
    {
      color = COLLISION_COLOR;
    }

    if (ships[i].isSunk())
    {
      color = 0x00008B;
    }

    CRGB final_color;

    for (int u = 0; u < ships[i].size(); u++)
    {
      final_color = color;
      if ((ships[i].compartment_hit[u]) && (!ships[i].isSunk()))
        final_color = hit_color;

      if (ships[i].orientation() == VERTICAL)
      {
        leds[translateToCRGB(ships[i].x, ships[i].y + u)] = final_color;
      }

      if (ships[i].orientation() == HORIZONTAL)
      {
        leds[translateToCRGB(ships[i].x + u, ships[i].y)] = final_color;
      }
    }
  }

  FastLED.show();
}

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
  //Serial.print("Ship selected: ");
  //Serial.println(current_ship);
  if (!ships[current_ship].visible)
    ships[current_ship].visible = true;

  drawShips(ships);
}

void randomLayout()
{
  unsigned int max_iterations = 100;

  for (int i = 0; i < NUM_SHIPS; i++)
  {

    //Serial.print("Setting ship ");
    //Serial.println(current_ship);

    unsigned int x;
    unsigned int y;
    for (unsigned int u = 0; u < max_iterations; u++)
    {
      //Serial.print("Iteration ");

      if (checkCollisions(current_ship) || u == 0)
      {
        //Serial.println(u);
        x = random(1, 11);
        y = random(1, 11);
        bool orientation = random(0, 2);

        ships[current_ship].changeLocation(x - ships[current_ship].x, CHANGE_X);
        ships[current_ship].changeLocation(y - ships[current_ship].y, CHANGE_Y);

        if (orientation)
        {
          ships[current_ship].rotate();
        }
        drawShips(ships);
        //delay(200);
      }
      else
      {
        //Serial.println("100");
        continue;
      }
    }
    //Serial.println(x);
    //Serial.println(y);
    changeShip();
  }
}

void setup()
{
  randomSeed(analogRead(0));

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

  ships[0].visible = true;
  /*   ships[9].x = 5;
  ships[9].visible = true;
  ships[9].compartment_hit[0] = true;
  ships[9].compartment_hit[1] = true;
  ships[9].compartment_hit[2] = true;
  ships[9].compartment_hit[3] = true; */

  drawShips(ships);

  //randomLayout();
}

void loop()
{
  unsigned long currentTime = millis();

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
    //changeFunction();
    randomLayout();
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

  if (currentTime - previousTime >= blinkInterval)
  {
    if (hit_color == hit_color_1)
    {
      hit_color = hit_color_2;
    }
    else
    {
      hit_color = hit_color_1;
    }
    drawShips(ships);

    previousTime = currentTime;
  }
}