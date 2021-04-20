/// *KEYBOARD SETUP* ///
#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {34,35,36,37}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {31,32,33}; //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


/// *RANGEFINDER SETUP* ///
int PROXIMITY_THRESHOLD = 15; //TO ADJUST
int PIN_PROXIMITY_SENSOR = A8;
int PROXIMITY_VAL_PREV = 0;


/// *HANDSET SETUP* ///
//low when hung up; high when used
int PIN_HANDSET_SWITCH = 40;


/// *COIN RELEASE SETUP* ///
//low when not triggered; high when triggered
int PIN_COIN_SWITCH = 41;


/// *MUSIC SHIELD SETUP* ///
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
}

void loop() {
  /// input from keyboard verification
  char customKey = customKeypad.getKey();
  if (customKey){
    Serial.print("Pressed ");
    Serial.println(customKey);
  }
}
