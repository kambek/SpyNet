// SPYNET. lowercase intentional. original project. 

bool dirty = true;
// change the start/stop on the main screen
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
unsigned long lastButtonTime = 0;
const unsigned long debounceTime = 50;

// changed when radio starts/stops 
bool playingState = LOW;

//temp
int lastCLK;
int lastSWButton = HIGH;
unsigned long lastEncoderTime = 0;
const unsigned long encoderDebounce = 20;

int selected;


// init 
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// changed when radio confirmed 


// main protocol. sending cmds to python to change settings
void sendCommand(String command, String value){
  Serial.print("$CMD,");
  Serial.print(command);
  Serial.print(",");
  Serial.println(value);
}

// to start audio
void sendStart(){
    Serial.println("$START");
}

// to stop audio
void sendStop(){
    Serial.println("$STOP");
}


void setup() {
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  lastCLK = digitalRead(CLK);
  lastButton = digitalRead(button);


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
  "start",
  "volume",
  "mode",
  "frequency",
  "experiment"
};

// all modes for mode scrn
const char* modeItems [] = {
  "FM",
  "AM",
  "SW",
  "ATC",
  "AIS",
  "ADS-B"
};

void defaultFrequency(){
  if(radio.mode == "FM"){
    radio.freq = 100400000;
  }
    if (radio.mode == "AM") {
    radio.freq = 999000;
  }

  if (radio.mode == "SW") {
    radio.freq = 7100000;
  }

  if (radio.mode == "ATC") {
    radio.freq = 120800000;
  }

  if (radio.mode == "AIS") {
    radio.freq = 156800000;
  }

  if (radio.mode == "ADS-B") {
    radio.freq = 1090000000;
  }
}

int getMinFreq(){
  if (radio.mode == "FM") return 87500000;
  if (radio.mode == "AM") return 531000;
  if (radio.mode == "SW") return 100000;
  if (radio.mode == "ATC") return 118000000;
  if (radio.mode == "AIS") return 156000000;
  if (radio.mode == "ADS-B") return 1090000000;

  return 0;
}

int getMaxFreq(){
  if (radio.mode == "FM") return 108000000;
  if (radio.mode == "AM") return 1602000;
  if (radio.mode == "SW") return 30000000;
  if (radio.mode == "ATC") return 137000000;
  if (radio.mode == "AIS") return 162000000;
  if (radio.mode == "ADS-B") return 1090000000;

  return 0;
}

int steps(){
  if (radio.mode == "FM") return 100000;
  if (radio.mode == "AM") return 10000;
  if (radio.mode == "SW") return 5000;
  if (radio.mode == "ATC") return 10000;
  if (radio.mode == "AIS") return 25000;
}

// variables for max items / encoder calc
const int mainItemsCount = 5;
const int modeItemsCount = 6;

// three modes 
enum Screen {
  MAIN, 
  EDIT, 
  MODE,
  EXPERIMENT,
  VOLUME
};
// this is all rendering don't touch
Screen screen = MAIN;

void render() {
  switch(screen){
    case MAIN:
    drawMain();
    break;

    case MODE: 
    drawMode();
    break;

    case EDIT:
    drawEdit();
    break;

    case EXPERIMENT:
    drawExperiment();
    break; 

    case VOLUME:
    drawVolume();
    break;
  }
}


// draws the main screen (arrow included) dont touch coords!
void drawMain(){
  if (selected >= mainItemsCount) {
    selected = 0;
  }
  if (selected < 0){
    selected = mainItemsCount - 1;
  }
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  tft.setTextSize(2);
  tft.setCursor(8,15);
  tft.print(radio.freq / 1000000.0 , 3);
  tft.setTextSize(1);
  tft.print(" mHz");
  tft.setCursor(98, 25);
  tft.setTextSize(1);
  tft.print(radio.mode);
  
  tft.setCursor(10, 40);
  tft.setTextSize(1);
  if (playingState == LOW){ 
      tft.print("start");
  }
  else if (playingState == HIGH) {
    tft.print("stop");
  }
  

  tft.setCursor(10, 55);
  tft.setTextSize(1);
  tft.print("volume");

  tft.setCursor(10, 70);
  tft.setTextSize(1);
  tft.print("mode");
  
  tft.setCursor(10, 85);
  tft.print("frequency");

  tft.setCursor(10, 100);
  tft.print("experiment");

  tft.setCursor(2, 40 + selected * 15);
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
      tft.setTextColor(ST77XX_WHITE);

    }
    tft.setCursor(8, y + 2);
    tft.print(modeItems[i]);
  }
}
// edit any of parameters
void drawEdit(){
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(18, 45);
  tft.println("freq");

  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.print(radio.freq / 1000000.0, 3);

  tft.setTextSize(1);
  tft.setCursor(90, 80);
  tft.println("mhz");
}

void drawExperiment(){
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);

  tft.setTextSize(1);
  tft.setCursor(20, 45);
  tft.println("experiment");

  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.print(radio.freq / 1000000.0, 3);
  
  tft.setTextSize(1);
  tft.setCursor(70, 80);
  tft.print("mhz");

  tft.setCursor(20, 80);
  tft.print("mode: ");
  tft.print(radio.mode);
}

void drawVolume(){
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(27, 15);
  tft.setTextSize(2);
  tft.print("volume");
  if(radio.volume != 100 && radio.volume != 0){
    tft.setCursor(40, 50);
    tft.setTextSize(4);
    tft.print(radio.volume);
  }
  if(radio.volume == 100){
    tft.setTextSize(4);
    tft.setCursor(30, 50);
    tft.print(radio.volume);
  }
  if(radio.volume == 0){
    tft.setTextSize(4);
    tft.setCursor(50,50);
    tft.print(radio.volume);
  }
}

void loop() {
  // encoder proccesing 

  int currentCLK = digitalRead(CLK);
  if (currentCLK != lastCLK){
    if (millis() - lastEncoderTime >= encoderDebounce){
      if (currentCLK == LOW){
        // if screen == mode/main we increment selected to move arrow
        if (screen == MODE || screen == MAIN){

        if (digitalRead(DT) != currentCLK){
          selected++;
          dirty = true;
        }
        else {
          selected--;
          dirty = true;
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
      if (screen == EDIT && selected == 0){
        if(digitalRead(DT) != currentCLK){
          radio.freq += steps();
          if (radio.freq >= getMaxFreq()){
            radio.freq = getMaxFreq();
          }
        }
        else {
          radio.freq -= steps();
          if (radio.freq <= getMinFreq()){
            radio.freq = getMinFreq();
          }
        }
        dirty = true;
      }
      if (screen == EXPERIMENT) {
        if (digitalRead(DT) != currentCLK) {
          radio.freq += steps();
          if(radio.freq >= getMaxFreq()){
            radio.freq = getMaxFreq();
          }
        }
        else {
          radio.freq -= steps();
          if(radio.freq <= getMinFreq()){
            radio.freq = getMinFreq();
          }
        }
        sendCommand("FREQ", String(radio.freq));
        dirty = true;
      }
      if (screen == VOLUME) {
        if (digitalRead(DT) != currentCLK){
          radio.volume += 10;
        }
        else {
          radio.volume -= 10;
        }
        dirty = true; 
      }
    }
    lastCLK = currentCLK;
    }
  }
    
  int currentButton = digitalRead(button);
  if (currentButton != lastButton){
    if (millis() - lastButtonTime > debounceTime) {
      lastButton = currentButton;
      lastButtonTime = millis();
      
      if(currentButton == LOW){
        if(screen==MAIN){
          if (selected == 0){
            if (playingState == LOW){
              sendCommand("FREQ", String(radio.freq));
              sendCommand("MODE", radio.mode);
              sendCommand("VOL", String(radio.volume));
              sendStart();
              playingState = HIGH;
            }
            else {
              sendStop();
              playingState = LOW;
            }
            screen = MAIN;
            selected = 0;
            dirty = true;
            
          }
          if (selected == 1){
            screen = VOLUME;
            selected = 0;
            dirty = true;
          }
          if (selected == 2){
            screen = MODE;
            selected = 0;
            dirty = true;
          }
          if (selected == 3){
            screen = EDIT;
            selected = 0;
            dirty = true;
          }
          if (selected == 4){
            screen = EXPERIMENT;
            selected = 0;
            dirty = true;
          }
        }
        else if (screen == MODE){
          if (selected == 0){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
          if (selected == 1){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
          if (selected == 2){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
          if (selected == 3){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
          if (selected == 4){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
          if (selected == 5){
            sendCommand("MODE", modeItems[selected]);
            radio.mode = modeItems[selected];
            defaultFrequency();
            sendCommand("FREQ", String(radio.freq));

            screen = MAIN;
            dirty = true;
          }
        }
        else if (screen == EDIT){
          sendCommand("FREQ", String(radio.freq));
          screen = MAIN;
          selected = 0;
          dirty = true;
        }
        else if (screen == EXPERIMENT){
          screen = MAIN;
          selected = 0;
          dirty = true; 
        }
        else if (screen == VOLUME){
          sendCommand("VOL", String(radio.volume));
          screen = MAIN;
          selected = 0;
          dirty = true; 
        }
      }
    }
    
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
  if (screen == VOLUME){
    if (radio.volume <= 0){
      radio.volume = 0;
    }
    if (radio.volume >= 100){
      radio.volume = 100;
    }
  }

  
  // protocol works here. done through serial/uart. 
  if (Serial.available()){
    String msg = Serial.readStringUntil('\n');
    msg.trim();
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
      playingState = value.toInt();
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
