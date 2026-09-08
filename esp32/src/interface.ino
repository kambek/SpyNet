// SPYNET. lowercase intentional. original project. 

bool dirty = true;

// dirty is a variable to refresh the screen. 

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

// encoder
int CLK = 17;
int DT = 15;
int SW = 16;

// button
int button = 19;
bool lastButton = HIGH;

//temp
int lastCLK;
int lastSWButton = HIGH;

int selected;


// init 
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// changed when radio confirmed 
bool playing = false;


// main protocol. sending cmds to python to change settings
void sendCommand(String command, String value){
  Serial.print("$CMD,");
  Serial.print(command);
  Serial.print(",");
  Serial.println(value);
}

// to start audio
void sendStart(){
  if(playing== false){
    Serial.println("$START");
    
  }
}

// to stop audio
void sendStop(){
  if(playing==true){
    Serial.println("$STOP");
    
  }
}

void setup() {
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  lastCLK = digitalRead(CLK);

  tft.initR(INITR_144GREENTAB);

  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(2);
  Serial.begin(115200);
}


// all possible values for protocol
struct radioState {
  int freq = 100400000;
  String mode = "FM";
  int volume = 50;
  int signal = 0;
  String error;
};

// declaring radio.
radioState radio;

// items in main 
const char* mainItems[] = {
  "frequency",
  "volume",
  "mode"
};

// all modes for mode scrn
const char* modeItems [] = {
  "FM",
  "AM",
  "SW",
  "ATC",
  "Marine",
  "ADS-B"
};

// variables for max items / encoder calc
const int mainItemsCount = 4;
const int modeItemsCount = 6;

// three modes 
enum Screen {
  MAIN, 
  EDIT, 
  MODE
};
// this is all rendering don't touch
Screen screen = MAIN;

void render() {
  switch(screen){
    case MAIN:
    drawMain();
    break;

    case MODE:
    //drawMenu(); 
    drawMode();
    break;

    case EDIT:
    //drawFreq();
    drawEdit();
    break;

  }
}


// draws the main screen (arrow included) dont touch coords!
void drawMain(){
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  tft.setTextSize(2);
  tft.setCursor(8,35);
  tft.print(radio.freq / 1000000.0 , 3);
  tft.setTextSize(1);
  tft.print(" mHz");

  tft.setCursor(98, 45);
  tft.setTextSize(1);
  tft.print(radio.mode);

  tft.setCursor(10, 60);
  tft.setTextSize(1);
  tft.print("frequency");

  tft.setCursor(10, 75);
  tft.setTextSize(1);
  tft.print("volume");

  tft.setCursor(10, 90);
  tft.setTextSize(1);
  tft.print("mode");
  
  tft.setCursor(10, 105);
  if (playing == false){ 
      tft.print("start");
  }
  else {
    tft.print("stop");
  }

  tft.setCursor(2, 60 + selected * 15);
  tft.print(">");
}


// menu to switch modes 
void drawMode(){
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  for (int i = 0; i < modeItemsCount; i++){
    int y = 5 + i * 20;
    if (i == selected){
      tft.fillRoundRect(
        2, y,
        124, 18,
        4, 
        ST77XX_BLUE
      );
      tft.setTextColor(ST77XX_WHITE);
    }
    else{
      tft.setTextColor(ST77XX_BLACK);

    }
    tft.setCursor(8, y + 2);
    tft.print(modeItems[i]);
  }
}
// edit any of parameters
void drawEdit(){
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(2, 5);
  tft.println("FREQ");

  tft.setCursor(5, 40);
  tft.print(radio.freq / 1000000.0, 3);

  tft.setCursor(5, 60);
  tft.println("MHz.");
}



void loop() {
  // encoder proccesing 
  int currentCLK = digitalRead(CLK);
  if (currentCLK != lastCLK){
    if (currentCLK == LOW){
      // if screen == mode/main we increment selected to move arrow
      if (screen == MODE || screen == MAIN){

      if (digitalRead(DT) != currentCLK){
        selected++;
      }
      else {
        selected--;
      } 
      // overpass prevention for main
      if (screen == MAIN){
        if (selected >= mainItemsCount){
          selected = 0;
        }

        if (selected < 0){
          selected = mainItemsCount - 1;
        }

      dirty = true;
  }
      }
  }
  lastCLK = currentCLK;
 }
  

  // overpass prevention for mode 
  if (screen == MODE){
    if (selected >= modeItemsCount){
      selected = 0;
    }
    if (selected < 0){
      selected = modeItemsCount - 1;
    }
  }
  
  // doesn't work in the current version but used in edit to edit freq
  if (currentCLK != lastCLK){
    if (screen == EDIT && selected == 0){
      if (digitalRead(DT) != currentCLK) {
        radio.freq += 100000;
      }
      else {
        radio.freq -= 100000;
      }
    }
  
    dirty = true; 
  }
  // protocol works here. done through serial/uart. 
  if (Serial.available()){
    String msg = Serial.readStringUntil('\n');
    msg.trim();
  // confirm init 
    if (msg=="$HELLO"){
      Serial.println("$OK, HELLO");
  }
  // after confirm update screen
    if (msg.startsWith("$OK,FREQ,")) {
      String value = msg.substring(9);
      radio.freq = value.toInt();
      dirty = true;
    }

    if (msg.startsWith("$OK,MODE,")) {
      String value = msg.substring(9);
      radio.mode = value;
      dirty = true;
    }

    if (msg.startsWith("$OK,VOL,")) {
      String value = msg.substring(8);
      radio.volume = value.toInt();
      dirty = true;
    }

    if (msg == "$OK,START") {
      playing = true;
      dirty = true;
    }

    if (msg == "$OK,STOP") {
      playing = false;
      dirty = true;
    }
  // asking for current state 
    if(msg.startsWith("$STATE,FREQ,")){
      String value = msg.substring(11); 
      radio.freq = value.toInt();
      dirty = true;
    }

    if(msg.startsWith("$STATE,MODE,")){
      String value = msg.substring(11);
      radio.mode = value;
      dirty = true;
    }

    if(msg.startsWith("$STATE,VOL,")){
      String value = msg.substring(10);
      radio.volume = value.toInt();
      dirty = true;
    }
    
    if (msg.startsWith("$STATE,SIGNAL,")){
      String value = msg.substring(13);
      radio.signal = value.toInt();
      dirty = true;
    }

    if (msg.startsWith("$STATE,PLAYING,")){
      String value = msg.substring(15);
      playing = value.toInt();
      dirty = true;
    }
  // troubleshooting 
    if (msg.startsWith("$ERROR,")) {
      radio.error = msg.substring(7);
      dirty = true;
  } 
}

// rendering 
  if (dirty) {
    render();
    dirty = false;
  }
}