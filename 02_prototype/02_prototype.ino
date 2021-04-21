/// *SONGS & BASICS* ///
#define NUM_SONGS 32
String SONGS[NUM_SONGS] = {
  "1001","1002","1003","1004","1005",
  "1006","1007","1008","1009","1010",
  "1011","1012","1013","1014","1015",
  "1016","1017","1018","1019","1020",
  "1021","1022","1023","1024","1025",
  "1026","1027","1028","1029","1030",
  "1031","1032",
};

char CODE_ENTERED[4];
int CODE_COUNTER = 0;


/// *MODES/STATES* ///
int MODE = 0;
// mode 0: waiting;
// mode 1: proximity triggered;
// mode 2: in use;

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
int PIN_COIN_SWITCH = 50;


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
  //_________________________________________________________ HANDSET HANGUP SWITCH
  pinMode(PIN_HANDSET_SWITCH, INPUT_PULLUP);
    Serial.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
// Set volume for left, right channels. lower numbers == louder volume!
   musicPlayer.setVolume(20,20);
  //musicPlayer.startPlayingFile("track002.mp3");

    // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  // Play one file, don't return until complete
  // Serial.println(F("Playing track 001"));
  // musicPlayer.playFullFile("/track002.mp3");
  // Play another file in the background, REQUIRES interrupts!
  // Serial.println(F("Playing track 002"));
  // musicPlayer.startPlayingFile("/track002.mp3");
}

void loop() {
  
    // Serial.println(digitalRead(PIN_HANDSET_SWITCH));
    
  for (int i = 0; i < 4; ++i){
    while((CODE_ENTERED[i] = customKeypad.getKey())==NO_KEY) {
      delay(1); // Just wait for a key
    } 
    // Wait for the key to be released
    while(customKeypad.getKey() != NO_KEY) {
      delay(1);
    } 
    }
    String code = String(CODE_ENTERED[0]) + String(CODE_ENTERED[1]) + String(CODE_ENTERED[2]) + String(CODE_ENTERED[3]);
  
  Serial.println("Entered number is ");
  Serial.println(code);
bool song_was_found = false;
  for(int i=0;i<NUM_SONGS;i++){
    if(code == SONGS[i]){
      song_was_found = true;

      CODE_COUNTER = 0;
//      TIMEOUT_DIGIT = false;
      
//      if(DEBUG==true){Serial.println("Song Found...");Serial.println(SONGS[i]+".mp3");}
      if (i<10) {
      String s = "/track00"+String(i+1)+".mp3";
      char filename[14]; // null termination. thanks for the ground work fuzzy
      s.toCharArray(filename, 14);
      Serial.println(s);
      //musicPlayer.setVolume(50,50);
      musicPlayer.startPlayingFile(filename); 
      Serial.println("done playing");
 //     PLAYED_A_SONG = true;
      } else {
        String s = "/track0"+String(i+1)+".mp3";
      char filename[14]; // null termination. thanks for the ground work fuzzy
      s.toCharArray(filename, 14);
      Serial.println(s);
      //musicPlayer.setVolume(50,50);
      musicPlayer.startPlayingFile(filename); 
      Serial.println("done playing");
       }
    }
  }

  if(song_was_found == false){
    CODE_COUNTER = 0;
    //TIMEOUT_DIGIT = false;
    musicPlayer.playFullFile("/track002.mp3");  
  }
 

  delay(5000);


  
 
  //musicPlayer.playFullFile("/track021.mp3");
  
}
