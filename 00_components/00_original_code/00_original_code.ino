//fuzzywobble
//NOV 2016


#define NUM_SONGS 45
String SONGS[NUM_SONGS] = {"1542","2998","3540","4466","5971","6308","7459","8912","9021","1039","1165","1254","1332","1437","1522","1602","1700","1845","1926","2077","2155","2228","2334","2475","2511","2634","2701","2876","2944","3003","3197","3256","3335","3471","3588","3619","3787","3813","3905","4096","4173","4219","4350","4433","4560"};                                                                                                

char CODE_ENTERED[4];
int CODE_COUNTER = 0;

//0 – waiting
//1 – proximity triggered, ringing
//2 – in use
//3 – hangup, idle for XXXXms
int MODE = 0;


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

bool BROADCAST = false;






//#############################################################
//#############################################################

//_________________________________________________________
//_________________________________________________________ HANDSET HANGUP SWITCH
//we are reading the handset switch on pin 40
//low when hung up
//high when used
int PIN_HANDSET_SWITCH = 40;

//_________________________________________________________
//_________________________________________________________ PROXIMITY SENSOR
int PROXIMITY_THRESHOLD = 42; //change this
int PIN_PROXIMITY_SENSOR = A8;
int PROXIMITY_VAL_PREV = 0;

//_________________________________________________________
//_________________________________________________________ MP3 SHIELD
// include SPI, MP3 and SD libraries
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

//_________________________________________________________
//_________________________________________________________ KEYPAD
#include <Keypad.h>
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 33, 34, 35, 36 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 30, 31, 32 }; 
// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );





//#############################################################
//#############################################################
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







//#############################################################
//#############################################################
void loop() {



        



  switch (MODE) {


    //################################################################################
    case 1: //1 – proximity triggered, ringing
      
      //if(DEBUG==true){Serial.println("MODE 1");}
      
      //musicPlayer.setVolume(100,10); //handset off. speaker very loud.
      musicPlayer.setVolume(45,100); //handset off. speaker very loud.
      
      musicPlayer.startPlayingFile("bell.mp3"); //bell sound
      
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
      break;


    //################################################################################
    case 2: //2 – in use

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
        if(DEBUG==true){Serial.println("Play hello...");}
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("hello6.mp3"); //play start sound
      }
      if(TIMEOUT_DIGIT==true){ //turns on after you hit one digit
        if(millis() - MODE_2_TIMER_DIGIT_START > MODE_2_DURATION_DIGIT_TIMEOUT){
          CODE_COUNTER = 0;
          TIMEOUT_DIGIT = false;
          if(DEBUG==true){Serial.println("Digit Timeout...");}
          musicPlayer.stopPlaying();
          musicPlayer.startPlayingFile("sorry2.mp3"); //play start sound
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
        int randIdx = int(random(0, 45));
        play_song(SONGS[randIdx]);
      }
        
      break;


    //################################################################################
    case 3: //3 – hangup, idle for XXXXms

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
      
      break;


    //################################################################################  
    default: //0 – waiting

      if(DEBUG==true){Serial.println("MODE 0");}
      if(DEBUG==true){Serial.println(analogRead(PIN_PROXIMITY_SENSOR));}

      //got two readings in a row below sensor threshold
      if(analogRead(PIN_PROXIMITY_SENSOR) < PROXIMITY_THRESHOLD && PROXIMITY_VAL_PREV < PROXIMITY_THRESHOLD){
        MODE = 1;
        MODE_1_TIMER_START = millis();
      }
      PROXIMITY_VAL_PREV = analogRead(PIN_PROXIMITY_SENSOR);
      
      delay(100);
    
    break;


    
  }


  


}







void keypad_to_song(){
  //if(DEBUG==true){Serial.println("key");}
  char key = kpd.getKey();
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
    musicPlayer.playFullFile("d1.mp3");
    CODE_ENTERED[CODE_COUNTER] = '1';
  }
  if(key=='2'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d2.mp3");
    CODE_ENTERED[CODE_COUNTER] = '2';
  }
  if(key=='3'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d3.mp3");
    CODE_ENTERED[CODE_COUNTER] = '3';
  }
  if(key=='4'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d4.mp3");
    CODE_ENTERED[CODE_COUNTER] = '4';
  }
  if(key=='5'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d5.mp3");
    CODE_ENTERED[CODE_COUNTER] = '5';
  }
  if(key=='6'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d6.mp3");
    CODE_ENTERED[CODE_COUNTER] = '6';
  }
  if(key=='7'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d7.mp3");
    CODE_ENTERED[CODE_COUNTER] = '7';
  }
  if(key=='8'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d8.mp3");
    CODE_ENTERED[CODE_COUNTER] = '8';
  }
  if(key=='9'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d9.mp3");
    CODE_ENTERED[CODE_COUNTER] = '9';
  }
  if(key=='0'){
    musicPlayer.stopPlaying();
    musicPlayer.playFullFile("d0.mp3");
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
    musicPlayer.startPlayingFile("sorry2.mp3");  
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
