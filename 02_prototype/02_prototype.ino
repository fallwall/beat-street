/// *SONGS & BASICS* ///
#define NUM_SONGS 49
String SONGS[NUM_SONGS] = {"9922","7665","5865","9997","5329","3505","9498","8787","2400","4957","9690","2951","5898","2426","6233","3327","3352","1485","3555","9672","1967","9904","6981","4536","5911","4061","3717","9156","2470","1840","1406","8830","1057","5433","5282","3201","6730","3905","3968","6666","1889","7917","7977","8967","8063","4033","6214","1005","3977"};

char CODE_ENTERED[4];
int CODE_COUNTER = 0;


/// *MODES/STATES* ///
int MODE = 0;
// mode 0: waiting;
// mode 1: proximity triggered;
// mode 2: in use;
// mode 3: hang up, idle for a while
int DEBUG = true;

long MODE_1_TIMER_START;
long MODE_1_DURATION = 5000; //rings for 5s

bool REPLAY_MODE = true; 
bool TIMEOUT_DIGIT = false;
bool PLAYED_A_SONG = false;
long MODE_2_TIMER_DIGIT_START;
long MODE_2_DURATION_DIGIT_TIMEOUT = 5000; //

long MODE_3_TIMER_START;
long MODE_3_DURATION = 5000; //idle for 5s after hangup


int VOL_SPEAKER = 10;
float VOL_SPEAKER_MULTIPLIER = 1.0; //0.0-3.0 (-= 0.5 each time)
int VOL_HEADSET = 28;

bool BROADCAST = true;

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
int PROXIMITY_THRESHOLD = 20; //TO ADJUST
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
  digitalWrite(42,HIGH);

  //_________________________________________________________ MP3 SHIELD
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find MP3 Shield, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("MP3 shield found"));
  SD.begin(CARDCS);
  //musicPlayer.setVolume(100,100); //weird, low is loud! left speaker is handset. right speaker is bell. 
  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  pinMode(52,OUTPUT);
  digitalWrite(52,LOW);
  
}

void caseOne() {
 //1 – proximity triggered, ringing
      
      //if(DEBUG==true){Serial.println("MODE 1");}
      
      //musicPlayer.setVolume(100,10); //handset off. speaker very loud.
      musicPlayer.setVolume(45,100); //handset off. speaker very loud.
      
      musicPlayer.startPlayingFile("ring.mp3"); //bell sound
      
      while (musicPlayer.playingMusic){

        if(DEBUG==true){Serial.print("ring ");}
      
        //phone was answered
        if(digitalRead(PIN_HANDSET_SWITCH)==LOW){ //may have to switch to HIGH !!!
          musicPlayer.stopPlaying();
          MODE = 2;   
          REPLAY_MODE = true;
          TIMEOUT_DIGIT = false;
        }
        
        if(millis() - MODE_1_TIMER_START > MODE_1_DURATION){
          musicPlayer.stopPlaying();
          MODE = 0; 
        }
        delay(100);
      }
 }

void caseTwo() {
  //2 – in use

      //if(DEBUG==true){Serial.println("MODE 2");}

      //remember! high number = low volume. low number = high volume. 100 = off. 0 = max. 
      if(BROADCAST==true){
        //if(DEBUG==true){Serial.println("Broadcasting...");}
        //musicPlayer.setVolume(VOL_HEADSET,VOL_SPEAKER); //handset. speaker.  
        musicPlayer.setVolume(VOL_SPEAKER,VOL_HEADSET); 
      }else{
        //if(DEBUG==true){Serial.println("Not Broadcasting...");}
        //musicPlayer.setVolume(VOL_HEADSET,100); //handset. speaker off. 
        musicPlayer.setVolume(100,VOL_HEADSET);
      }

      if(REPLAY_MODE==true){ //this goes to false once a digit is clicked 
        // if(DEBUG==true){Serial.println("picked up!");}
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("hello.mp3"); //play start sound
      }
      if(TIMEOUT_DIGIT==true){ //turns on after you hit one digit
        if(millis() - MODE_2_TIMER_DIGIT_START > MODE_2_DURATION_DIGIT_TIMEOUT){
          CODE_COUNTER = 0;
          TIMEOUT_DIGIT = false;
          if(DEBUG==true){Serial.println("Digit Timeout...");}
          musicPlayer.stopPlaying();
          musicPlayer.startPlayingFile("sorry.mp3"); //play start sound
        }
      }

      //read keypad
      keypad_to_song();

      //hung up
      if(digitalRead(PIN_HANDSET_SWITCH)==HIGH){  //may have to switch to LOW !!!
        musicPlayer.stopPlaying();
        MODE = 3;
        MODE_3_TIMER_START = millis();
      }

      //when music or message is playing we are stuck in this loop :U
      while (musicPlayer.playingMusic){

        //hung up
        if(digitalRead(PIN_HANDSET_SWITCH)==HIGH){  //may have to switch to LOW !!!
          musicPlayer.stopPlaying();
          MODE = 3;
          MODE_3_TIMER_START = millis();
          PLAYED_A_SONG = false;
        }

        //still need to read keypad
        keypad_to_song();
        
      }

      if(PLAYED_A_SONG==true){ //we are here because a song has finished playing. why don't we randomly play another?
        if(DEBUG==true){Serial.println("Song Finished...");}
        int randIdx = int(random(0, 49));
        play_song(SONGS[randIdx]);
      }
        
  }

 void caseThree() {
   //3 – hangup, idle for XXXXms

      BROADCAST = false;
      PLAYED_A_SONG = false;
      
      if(DEBUG==true){Serial.println("MODE 3");}

      //idle timer (to prevent it from ringing right away)
      if(millis() - MODE_3_TIMER_START > MODE_3_DURATION){
        MODE = 0;
      }

      //pickup right after hangup
      if(digitalRead(PIN_HANDSET_SWITCH)==LOW){ //may have to switch to HIGH !!!
        musicPlayer.stopPlaying();
        MODE = 2;   
        REPLAY_MODE = true;
      } 
    }

void caseZero() {
  if(DEBUG==true){Serial.println("MODE 0");}
      if(DEBUG==true){Serial.println(analogRead(PIN_PROXIMITY_SENSOR));}

      //got two readings in a row below sensor threshold
      if(analogRead(PIN_PROXIMITY_SENSOR) < PROXIMITY_THRESHOLD && PROXIMITY_VAL_PREV < PROXIMITY_THRESHOLD){
        MODE = 1;
        MODE_1_TIMER_START = millis();
      }
      PROXIMITY_VAL_PREV = analogRead(PIN_PROXIMITY_SENSOR);
      
      delay(100);
  }

void loop() {
  
  switch (MODE) {
    case 1:
      caseOne();
      break;
    case 2:
      caseTwo();
      break;
    case 3:
      caseThree();
      break;
    default:
      caseZero();
      break;
    }
  
}

void keypad_to_song(){
  //if(DEBUG==true){Serial.println("key");}
  char key = customKeypad.getKey();
  if(key=='#'){ //increase volume

    if(DEBUG==true){Serial.println("#");}

    VOL_SPEAKER_MULTIPLIER -= 0.5;
    if(VOL_SPEAKER_MULTIPLIER<0){VOL_SPEAKER_MULTIPLIER=3.0;}
    VOL_SPEAKER = int(10.0*VOL_SPEAKER_MULTIPLIER);
    VOL_HEADSET = VOL_SPEAKER + 20;

    if(DEBUG==true){Serial.print(VOL_SPEAKER);Serial.print(",");Serial.print(VOL_HEADSET);Serial.println();}

    if(BROADCAST==true){
      //musicPlayer.setVolume(VOL_HEADSET,VOL_SPEAKER); //handset. speaker. 
      musicPlayer.setVolume(VOL_SPEAKER,VOL_HEADSET); 
    }else{
      //musicPlayer.setVolume(VOL_HEADSET,100); //handset. speaker off. 
      musicPlayer.setVolume(100,VOL_HEADSET);
    }
    
  }
  if(key=='*'){ //toggle broadcast
    
    BROADCAST = !BROADCAST;

    if(BROADCAST==true){
      //musicPlayer.setVolume(VOL_HEADSET,VOL_SPEAKER); //handset. speaker.  
      musicPlayer.setVolume(VOL_SPEAKER,VOL_HEADSET);
    }else{
      //musicPlayer.setVolume(VOL_HEADSET,100); //handset. speaker off. 
      musicPlayer.setVolume(100,VOL_HEADSET); //handset. speaker off. 
    }
    
  }
  if(key=='1'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k1.wav");
    CODE_ENTERED[CODE_COUNTER] = '1';
  }
  if(key=='2'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k2.wav");
    CODE_ENTERED[CODE_COUNTER] = '2';
  }
  if(key=='3'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k3.wav");
    CODE_ENTERED[CODE_COUNTER] = '3';
  }
  if(key=='4'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k4.wav");
    CODE_ENTERED[CODE_COUNTER] = '4';
  }
  if(key=='5'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k5.wav");
    CODE_ENTERED[CODE_COUNTER] = '5';
  }
  if(key=='6'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k6.wav");
    CODE_ENTERED[CODE_COUNTER] = '6';
  }
  if(key=='7'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k7.wav");
    CODE_ENTERED[CODE_COUNTER] = '7';
  }
  if(key=='8'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k8.wav");
    CODE_ENTERED[CODE_COUNTER] = '8';
  }
  if(key=='9'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k9.wav");
    CODE_ENTERED[CODE_COUNTER] = '9';
  }
  if(key=='0'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("k0.wav");
    CODE_ENTERED[CODE_COUNTER] = '0';
  }
  if(key=='0'||key=='1'||key=='2'||key=='3'||key=='4'||key=='5'||key=='6'||key=='7'||key=='8'||key=='9'){
    MODE_2_TIMER_DIGIT_START = millis(); TIMEOUT_DIGIT = true; REPLAY_MODE = false; PLAYED_A_SONG = false; 
    CODE_COUNTER++; if(CODE_COUNTER==4){code_entered();}
  }
}


void code_entered(){

  TIMEOUT_DIGIT = false;

  String code_4 = String(CODE_ENTERED[0]) + String(CODE_ENTERED[1]) + String(CODE_ENTERED[2]) + String(CODE_ENTERED[3]);

  bool song_was_found = false;
  for(int i=0;i<NUM_SONGS;i++){
    if(code_4 == SONGS[i]){
      
      song_was_found = true;

      CODE_COUNTER = 0;
      TIMEOUT_DIGIT = false;
      Serial.println("at this point!");
      if(DEBUG==true){Serial.println("Song Found...");Serial.println(SONGS[i]+".mp3");}
      
      String s = SONGS[i]+".mp3";
      char filename[9]; //learned the hard way about null termination. fuk you C! 
      
      s.toCharArray(filename, 9);
      
      musicPlayer.startPlayingFile(filename); 
   
      PLAYED_A_SONG = true;
      
    }
  }

  if(song_was_found == false){
    CODE_COUNTER = 0;
    TIMEOUT_DIGIT = false;
    musicPlayer.startPlayingFile("sorry.mp3");  
  }
  
}


void play_song(String song){

      CODE_COUNTER = 0;
      TIMEOUT_DIGIT = false;
      
      String s = song+".mp3";

      if(DEBUG==true){Serial.println("Jukebox...");Serial.println(song+".mp3");}

      
      char filename[9]; //learned the hard way about null termination. fuk you C! 
      s.toCharArray(filename, 9);
      musicPlayer.startPlayingFile(filename); 
      
}
