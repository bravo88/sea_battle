#include <Arduino.h>
#include <FastLED.h>
#include <Ship.h>
#include <Aim.h>
#include <RH_ASK.h>
#include <SPI.h>

//Buttons
#define WHITE_BTN_PIN 11
#define BLUE_BTN_PIN 12
#define ENCODER_1_SW_PIN 6
#define ENCODER_1_A_PIN 2
#define ENCODER_1_B_PIN 3
#define ENCODER_2_SW_PIN 7
#define ENCODER_2_A_PIN 4
#define ENCODER_2_B_PIN 5

//Led strip
#define DATA_PIN 8
#define NUM_LEDS 100

// Radio communication
#define TX_PIN 10
#define RX_PIN 9

#define VERTICAL 1   // True
#define HORIZONTAL 0 // False

// FUNCTIONS
#define NUM_FUNCTIONS 4
#define CHANGE_Y 0
#define CHANGE_X 1
#define AIM_Y 2
#define AIM_X 3
unsigned int current_function = CHANGE_Y;

// MODES
unsigned int current_mode = 0;
#define POSITIONING_MODE 0
#define HANDSHAKE_MODE 1
#define AIM_MODE 2
#define WAIT_COORDINATES_MODE 3
#define WAIT_RESULT_MDOE 4
#define END_MODE 5

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
const unsigned long transmitInterval = 2000;
unsigned long previousTime = 0;

// Ship definitions
#define NUM_SHIPS 10
Ship ships[NUM_SHIPS] = {Ship(1), Ship(1), Ship(1), Ship(1), Ship(2), Ship(2), Ship(2), Ship(3), Ship(3), Ship(4)};
unsigned int current_ship = 0;

// COMMUNICATION
RH_ASK transmitter(2000, RX_PIN, TX_PIN, 0);

// AIM DEFINITION
Aim aim;

// SHOT RECORDS
// 0 - did not shoot
// 1 - shot
// 2 - miss
// 3 - hit

int my_shots[10][10];
int enemy_shots[10][10];

int enc1_a_last;
int enc1_a_val;
int enc2_a_last;
int enc2_a_val;

char message[] = "0";

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

bool ping(unsigned int x, unsigned int y, int ship = -1)
{
  // CHECKING FOR COLLISIONS WITH OTHER SHIPS
  // CHECKING IF ANY OF THE SHIPS ARE LOCATED AT GIVEN COORDINATES
  for (int i = 0; i < NUM_SHIPS; i++)
  {
    if (i == ship || !ships[i].visible)
      continue;
    for (int u = 0; u < ships[i].size(); u++)
    {
      // Target ship orientation vertical
      if (ships[i].orientation() == VERTICAL)
      {
        if ((ships[i].x == x) && (ships[i].y + u == y))
        {
          return true;
        }
      }

      // Target ship orientation horizontal
      if (ships[i].orientation() == HORIZONTAL)
      {
        if ((ships[i].y == y) && (ships[i].x + u == x))
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool checkCollisions(int ship)
{
  if (ships[ship].orientation() == VERTICAL)
  {
    // DEFINING SEARCH SCOPE AROUND THE ACTIVE SHIP
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

void drawShips(Ship ships[])
{
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
}

void drawAim(Aim aim)
{
  CRGB axis_color = CRGB(0x228B22);
  CRGB aim_color = CRGB(0xFF0000);
  // Draws aim on the map
  // Aim colors are dependent on mode aim/wait
  for (int i = 1; i <= 10; i++)
  {
    leds[translateToCRGB(i, aim.y)] = axis_color;
    leds[translateToCRGB(aim.x, i)] = axis_color;
    leds[translateToCRGB(aim.x, aim.y)] = aim_color;
  }
}

void drawShots(int shots[10][10])
{
  for (int x = 0; x < 10; x++)
  {
    for (int y = 0; y < 10; y++)
    {
      switch (shots[x][y])
      {
      case 1:
        leds[translateToCRGB(x + 1, y + 1)] = CRGB::Orange;
        break;

      default:
        break;
      }
    }
  }
}

void drawDisplay()
{
  FastLED.clear();
  switch (current_mode)
  {
  case POSITIONING_MODE:
    drawShips(ships);
    break;
  case HANDSHAKE_MODE:
    drawShips(ships);
    break;
  case AIM_MODE:
    drawAim(aim);
    drawShots(my_shots);
    break;
  case WAIT_COORDINATES_MODE:
    drawAim(aim);
    break;
  default:
    break;
  }
  FastLED.show();
}

void fire(unsigned int x, unsigned int y)
{
  my_shots[x - 1][y - 1] = 1;
  Serial.println(my_shots[x - 1][y - 1]);
  drawDisplay();
}

void changeFunction()
{
  switch (current_mode)
  {

  case POSITIONING_MODE:
    if (current_function == CHANGE_X)
    {
      current_function = CHANGE_Y;
    }
    if (current_function == CHANGE_Y)
    {
      current_function = CHANGE_X;
    }
    break;

  case AIM_MODE:
    if (current_function == AIM_X)
    {
      current_function = AIM_Y;
      break;
    }
    if (current_function == AIM_Y)
    {
      current_function = AIM_X;
      break;
    }
  default:
    break;
  }

  Serial.print("Curren function: ");
  Serial.println(current_function);
}

void handshake()
{
}

void checkBlueButton()
{
  if (digitalRead(BLUE_BTN_PIN))
  {
    if ((current_mode == POSITIONING_MODE) || (current_mode == AIM_MODE))
    {
      changeFunction();
      delay(200);
    }

    if (current_mode == HANDSHAKE_MODE)
    {
      handshake();
    }
  }
}

void changeShip()
{
  //Serial.println("Changing ship");
  if (current_ship < NUM_SHIPS - 1)
  {
    current_ship++;
  }
  else
  {
    current_ship = 0;
  }

  if (!ships[current_ship].visible)
  {
    ships[current_ship].visible = true;
  }
  drawDisplay();
}

void checkWhiteButton()
{
  if ((digitalRead(WHITE_BTN_PIN)) && (current_mode == POSITIONING_MODE))
  {
    changeShip();
    delay(200);
  }
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

      // Generating random position for a ship first iteration or if collision detected
      if (checkCollisions(current_ship) || u == 0)
      {
        //Serial.println(u);
        x = random(1, 11);
        y = random(1, 11);

        ships[current_ship].changeLocation(x - ships[current_ship].x, CHANGE_X);
        ships[current_ship].changeLocation(y - ships[current_ship].y, CHANGE_Y);

        // Rotate?
        bool orientation = random(0, 2);
        if (orientation)
        {
          ships[current_ship].rotate();
        }
        drawDisplay();
      }
      else
      {
        continue;
      }
    }
    changeShip();
  }
}

void rotateEncoder(bool encoder, bool direction)
{

  int vector = 1;
  if (!direction)
  {
    vector = vector * -1;
  }

  switch (current_mode)
  {
  case POSITIONING_MODE:
    ships[current_ship].changeLocation(vector, encoder);
    drawDisplay();
    break;
  case AIM_MODE:
    if (current_function == AIM_X)
    {
      aim.changeLocation(vector, false);
      drawDisplay();
    }

    if (current_function == AIM_Y)
    {
      aim.changeLocation(vector, true);
      drawDisplay();
    }
    break;
  default:
    break;
  }
}

void checkEncoderState()
{
  enc1_a_val = digitalRead(ENCODER_1_A_PIN);
  if (enc1_a_val != enc1_a_last)
  {
    if (digitalRead(ENCODER_1_B_PIN) != enc1_a_val)
    {
      rotateEncoder(false, false);
    }
    else
    {
      rotateEncoder(false, true);
    }
  }
  enc1_a_last = enc1_a_val;

  enc2_a_val = digitalRead(ENCODER_2_A_PIN);
  if (enc2_a_val != enc2_a_last)
  {
    if (digitalRead(ENCODER_2_B_PIN) != enc2_a_val)
    {
      rotateEncoder(true, false);
    }
    else
    {
      rotateEncoder(true, true);
    }
  }
  enc2_a_last = enc2_a_val;
}

void checkEncoderButton()
{
  if (!digitalRead(ENCODER_1_SW_PIN))
  {
    Serial.println("Enc 1 btn");
    if (current_mode == POSITIONING_MODE)
    {
      ships[current_ship].rotate();
      Serial.println(ships[current_ship].orientation());
      drawDisplay();
      delay(200);
    }

    if (current_mode == AIM_MODE)
    {
      fire(aim.x, aim.y);
      delay(200);
    }
  }

  if (!digitalRead(ENCODER_2_SW_PIN))
  {
    Serial.println("Enc 2 btn");
    if (current_mode == POSITIONING_MODE)
    {
      ships[current_ship].rotate();
      Serial.println(ships[current_ship].orientation());
      drawDisplay();
      delay(200);
    }

    if (current_mode == AIM_MODE)
    {
      fire(aim.x, aim.y);
      delay(200);
    }
  }
}

void defineShots()
{
  for (int i = 0; i < 10; i++)
  {
    for (int u = 0; u < 10; u++)
    {
      my_shots[i][u] = 0;
      enemy_shots[i][u] = false;
    }
  }
}

// RF CODES
/*
0001
0002
0003
0004
0005
0006
0007
0008
*/

int sequence = 0;

void transmitData()
{
  const char *msg = "0001";
  transmitter.send((uint8_t *)msg, strlen(msg));
  transmitter.waitPacketSent();
  delay(200);
}

void listenData()
{
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);

  if (transmitter.recv(buf, &buflen)) // Non-blocking
  {
    char msg[5];
    for (int i = 0; i < 4; i++)
    {
      msg[i] = (char)buf[i];
      msg[i+1] = '\0';
    }


    switch (current_mode)
    {
    case HANDSHAKE_MODE:
      if (strcmp(msg, "0001") == 0)
      {
        Serial.println("Ready");
      }
      break;

    default:
      break;
    }
  }
}

void setup()
{
  randomSeed(analogRead(0));

  Serial.begin(115200);
  pinMode(WHITE_BTN_PIN, INPUT);
  pinMode(BLUE_BTN_PIN, INPUT);
  pinMode(ENCODER_1_SW_PIN, INPUT);
  pinMode(ENCODER_1_A_PIN, INPUT);
  pinMode(ENCODER_1_B_PIN, INPUT);
  pinMode(ENCODER_2_SW_PIN, INPUT);
  pinMode(ENCODER_2_A_PIN, INPUT);
  pinMode(ENCODER_2_B_PIN, INPUT);
  pinMode(13, INPUT);

  enc1_a_last = digitalRead(ENCODER_1_A_PIN);
  enc2_a_last = digitalRead(ENCODER_2_A_PIN);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
  FastLED.setBrightness(20);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  transmitter.init();

  ships[0].visible = true;
  defineShots();

  current_function = AIM_X;

  // For testing we set random layout and mode for handshaking.
  // In handshaking mode blue button sets the master allowing to always aim and fire
  // After the blue button pressed, game starts
  // If blue button is not pressed, player is set to slave and only sends the results to master
  //randomLayout();
  current_mode = HANDSHAKE_MODE;
  drawDisplay();
}

void loop()
{

  // ENCODER
  checkEncoderState();

  // Cycling through ships
  checkWhiteButton();

  // Rotating ship
  checkEncoderButton();

  // Cycling through functions
  checkBlueButton();

  if (current_mode == HANDSHAKE_MODE)
  {
    listenData();
  }

  unsigned long currentTime = millis();
  if (currentTime - previousTime >= transmitInterval)
  {
    //transmitData();
    previousTime = currentTime;
  }

  if (digitalRead(13) == HIGH)
  {
    transmitData();
  }
}