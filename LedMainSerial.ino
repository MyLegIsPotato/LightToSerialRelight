#include <FastLED.h>

#define LED_PIN     9
#define NUM_LEDS    120
int SelectedBRIGHTNESS = 120;
int BRIGHTNESS = 120;
int BRIGHTNESS_MAX = 128;
int BRIGHTNESS_MIN = 1;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

int UPDATES_PER_SECOND = 100;
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

CRGBPalette16 primaryColorPalette = CRGBPalette16(CRGB::Blue);
CRGBPalette16 secondaryColorPalette = CRGBPalette16(CRGB::Red);
CRGBPalette16 mixedColorPalette;
CRGBPalette16 mixedSpacedColorPalette;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

const int buttonOnePin = 5;     // the number of the pushbutton pin
const int buttonTwoPin = 3;     // the number of the pushbutton pin
const int buttonThreePin = 4;     // the number of the pushbutton pin

int buttonOne = 0;         // variable for reading the pushbutton status
int buttonTwo = 0;
int buttonThree = 0;

int menuPos = 1;
int menuNum = 0;
int menusCount = 2;

bool buttonOnePressed = false;
bool buttonTwoPressed = false;
bool buttonThreePressed = false;

bool animationDirection = true;
int animationSpeedIndex = 10;
float speedsArray[] = {1 / 1000, 1 / 500, 1 / 300, 1 / 200, 1 / 100, 1 / 50, 1 / 20, 1 / 10, 1 / 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
float scaling = 1;
int currentAnimation = 0;
int solidOffset = 0;

int animationTimer = 0;
bool playAnimation = false;
bool ignorePalettes = false;

uint8_t input;
uint8_t timer = 0;

bool receivingColorData = false;
bool enableShowing = true;
byte colorData[6];

CRGB primaryColor = CRGB::Blue;
CRGB secondaryColor = CRGB::Red;

void IncreaseAnimationSpeedIndex(bool increase) {
  int newIndex;
  if (increase) {
    newIndex = ++animationSpeedIndex;
  } else {
    newIndex = --animationSpeedIndex;
  }

  if (newIndex > 0 && newIndex < sizeof(speedsArray)) {
    animationSpeedIndex = newIndex;
  }
}

void setup() {
  //Default palette
  currentPalette = RainbowColors_p;
  currentBlending = NOBLEND;
  SetupBSPalettes();
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  FastLED.setTemperature(UncorrectedTemperature);

  // initialize the pushbutton pin as an input:
  pinMode(buttonOnePin, INPUT);
  pinMode(buttonTwoPin, INPUT);
  pinMode(buttonThreePin, INPUT);

  Serial.begin(1200);
  FastLED.setMaxRefreshRate(120);
}

void loop() {
  // read the state of the pushbutton value:
  buttonOne = digitalRead(buttonOnePin);
  buttonTwo = digitalRead(buttonTwoPin);
  buttonThree = digitalRead(buttonThreePin);
  FastLED.setBrightness(BRIGHTNESS);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonOne == HIGH || buttonTwo == HIGH || buttonThree == HIGH ) {
    // turn LED on:
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
  }

  //////////////////////////////////////////
  //------------BUTTON CONTROL------------// //This is for premade palettes to control using digital pins on arduino.
  //////////////////////////////////////////
  if (buttonOne == HIGH && !buttonOnePressed && menuPos <= 12) {
    if (menuNum == 0) {
      menuPos++;
    } else if (menuNum == 1) {
      IncreaseAnimationSpeedIndex(true);
    } else if (menuNum == 2) {
      SelectedBRIGHTNESS = SelectedBRIGHTNESS * 2;
    };
    buttonOnePressed = true;
  } else if (buttonOne == LOW && buttonOnePressed) {
    buttonOnePressed = false;
  }

  if (buttonTwo == HIGH && !buttonTwoPressed) {
    if (menuNum < menusCount) {
      menuNum++;
    } else {
      menuNum = 0;
    }
    buttonTwoPressed = true;
  } else if (buttonTwo == LOW && buttonTwoPressed) {
    buttonTwoPressed = false;
  }

  if (buttonThree == HIGH && !buttonThreePressed  && menuPos > 0 ) {
    if (menuNum == 0) {
      menuPos--;
    } else if (menuNum == 1) {
      IncreaseAnimationSpeedIndex(false);
    } else if (menuNum == 2) {
      SelectedBRIGHTNESS = SelectedBRIGHTNESS / 2;
    };
    buttonThreePressed = true;
  } else if (buttonThree == LOW && buttonThreePressed) {
    buttonThreePressed = false;
  }

  //////////////////////////////////////////
  //----------- SERIAL CONTROL------------// //This is for different setting using Serial Monitor (Arduino IDE) OR my separate console app.
  //////////////////////////////////////////
  //Check for serial each n cycles (does not miss any bytes, they are just waiting in buffer for next n-th cycle)
  //n --------v
  if (timer > 2) {
    input = Serial.read();
    char inputChar = input;
    //Serial.println(inputChar);
    // $ and % are color identifiers, recieving it tells arduino to use incoming bytes as color data, not as a command. Only use first $ and then % otherwise it won't send data correctly.
    if(inputChar == '$'){ //Start
      enableShowing = false;
    }

    if(inputChar == '%'){ //End
      primaryColor.r = colorData[0];
      primaryColor.g = colorData[1];
      primaryColor.b = colorData[2];
      secondaryColor.r = colorData[3];
      secondaryColor.g = colorData[4];
      secondaryColor.b = colorData[5];
      SetupBSPalettes();
      BRIGHTNESS = 255;
      enableShowing = true;
//      for(int e=0; e < 6; e++){
//        Serial.println(colorData[e]);
//      }
    }

    if(!enableShowing){
      Serial.readBytes(colorData, 6);
    }
   
    if(!receivingColorData){
      //These are arduino commands. They are single characters so arduino can process them instantly.
      //This is basically code for you PC to communicate with arduino, if you want you can just send ASCII bytes equivalents
      //Serial.println(menuPos); // <--- You can debug things here
      switch (inputChar) {
      case '1': menuPos--; break; //1
      case '2': menuPos++; break; //2
      case '3': animationSpeedIndex--; break; //3
      case '4': animationSpeedIndex++; break; //4
      case '5': currentAnimation--; break; //5
      case '6': currentAnimation++; break; //6
      case '7': SelectedBRIGHTNESS = SelectedBRIGHTNESS / 2; break; //7
      case '8': SelectedBRIGHTNESS = SelectedBRIGHTNESS * 2; break; //8

      case 'z': currentAnimation = 0; break;
      case 'x': currentAnimation = 1; break;
      case 'c': LoadDefaultSettings(); break; //reset

      case '@': menuPos = 1; currentAnimation = 0; break; //Closes Connection and enables other effects.
      
      //BeatSaber mod palettes:
      case '#': menuPos = 20; break;
      case 'q': menuPos = 21; break;
      case 'w': menuPos = 22; break;
      case 'e': menuPos = 23; break;
      case 'r': menuPos = 24; break;
      case 't': menuPos = 25; break;
      case 'y': menuPos = 26; break;
      case 'p': menuPos = 27; break;  
      } 
    }
    //Resets timer when the cycle repeats
    timer = 0;
  } else {
    //Otherwise increments
    timer++;
  }

  /////////////////////////////////////////
  //----------PALETTE SELECTION----------//
  /////////////////////////////////////////
    switch (menuPos) {
      case 0: GeneratedPalette0(); currentAnimation = 0; break;
      case 1: GeneratedPalette1(); break;
      case 2: GeneratedPalette2(); break;
      case 3: Aqua(); break;
      case 4: GeneratedPalette3(); break;
      case 5: GeneratedPalette4(); break;
      case 6: GeneratedPalette5(); break;
      case 7: GeneratedPalette6(); break;
      case 8: GeneratedPalette7(); break;
      case 9: GeneratedPalette0(); break;
      
      case 20: FullWhitePalletteWithEnds(); currentAnimation = 4; break;
      case 21: RightTurnOn(); currentAnimation = 0; break;
      case 22: LeftTurnOn(); currentAnimation = 0; break;
      case 23: RightFlashAndLeaveOn(); currentAnimation = 1; break;
      case 24: LeftFlashAndLeaveOn(); currentAnimation = 1; break;
      case 25: RightFlashAndTurnOff(); currentAnimation = 0; break;
      case 26: LeftFlashAndTurnOff(); currentAnimation = 0; break;
      case 27: TurnOff();
    }

  /////////////////////////////////////////
  //--------ANIMATION SELECTION----------//
  /////////////////////////////////////////
  switch (currentAnimation) {
    case 0: AnimationSolid(); break;
    case 1: AnimationBurst(); break; //Requires signal to start => playAnimation = true;
    case 2: AnimationSpots(); break; //Requires signal to start => playAnimation = true;
    case 3: AnimationScale(); break; //Requires signal to start => playAnimation = true;
    case 4: AnimationBreath(); break;
  }
}//END OF loop();

/////////////////////////////////////////
//----------SYSTEM FUNCTIONS-----------//
/////////////////////////////////////////

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  uint8_t brightness = 255;

  //ALL LEDS AT THE SAME TIME
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += (255 / NUM_LEDS) * scaling; // 255 Color Slots / this number = NUM_LEDS
  }

  //EXPLANATION FROM DOCUMENTATION
  // Palettes
  //
  // RGB Palettes map an 8-bit value (0..255) to an RGB color.
  //
  // You can create any color palette you wish; a couple of starters
  // are provided: Forest, Clouds, Lava, Ocean, Rainbow, and Rainbow Stripes.
  //
  // Palettes come in the traditional 256-entry variety, which take
  // up 768 bytes of RAM, and lightweight 16-entry varieties.  The 16-entry
  // variety automatically interpolates between its entries to produce
  // a full 256-element color map, but at a cost of only 48 bytes or RAM.
}

int Lerp(int outMin, int outMax, int inMin, int inMax, int input) {
  int output;
  output = ((outMin + outMax) / (inMin + inMax)) * input;
  return output;
}

void SetupBSPalettes(){
  primaryColorPalette = CRGBPalette16(
    primaryColor, CRGB::Black, primaryColor, CRGB::Black,
    primaryColor, CRGB::Black, primaryColor, CRGB::Black,
    primaryColor, CRGB::Black, primaryColor, CRGB::Black,
    primaryColor, CRGB::Black, primaryColor, CRGB::Black
    );
  secondaryColorPalette = CRGBPalette16(
    CRGB::Black, secondaryColor, CRGB::Black, secondaryColor,
    CRGB::Black, secondaryColor, CRGB::Black, secondaryColor,
    CRGB::Black, secondaryColor, CRGB::Black, secondaryColor,
    CRGB::Black, secondaryColor, CRGB::Black, secondaryColor
    );
  
//  mixedColorPalette = CRGBPalette16(
//    primaryColor, secondaryColor, primaryColor, secondaryColor,
//    primaryColor, secondaryColor, primaryColor, secondaryColor,
//    primaryColor, secondaryColor, primaryColor, secondaryColor,
//    primaryColor, secondaryColor, primaryColor, secondaryColor
//    );
//  mixedSpacedColorPalette = CRGBPalette16(
//    primaryColor, CRGB::Black,  secondaryColor, CRGB::Black,
//    primaryColor, CRGB::Black,  secondaryColor, CRGB::Black,
//    primaryColor, CRGB::Black,  secondaryColor, CRGB::Black,
//    primaryColor, CRGB::Black,  secondaryColor, CRGB::Black
//    );

    BRIGHTNESS = 200;
}

CRGBPalette16 fillPalette(CRGB colorSlots[], CRGBPalette16 currentPalette, CRGB backgroundColor){
  for(int i = 0; i < sizeof(colorSlots); i++){
    currentPalette[i] = colorSlots[i];
  };
  for(int j = 0; j < (16 - sizeof(colorSlots)); j++){
    int x = sizeof(colorSlots)+j;
    currentPalette[x] = backgroundColor;
  };
  return currentPalette;
}

void LoadDefaultSettings(){
  menuPos = 0;
  timer = 0;
  animationSpeedIndex = 10;
  animationTimer = 0;
  currentAnimation = 0;
  scaling = 1;
  playAnimation = false;
}


/////////////////////////////////////////
//--------------PALETTES---------------//
/////////////////////////////////////////

///////////////
//GENERATED----
void GeneratedPalette0() {
  Blackout();
}
void GeneratedPalette1() {
  CRGB slots[2];

  slots[0].r = 0;
  slots[0].g = 128;
  slots[0].b = 0;
  slots[1].r = 128;
  slots[1].g = 0;
  slots[1].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 90;
  currentPalette = fillPalette(slots, currentPalette, background);
}

void GeneratedPalette2(){
  CRGB slots[2];

  slots[0].r = 0;
  slots[0].g = 128;
  slots[0].b = 0;
  slots[1].r = 128;
  slots[1].g = 0;
  slots[1].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}


void GeneratedPalette3(){
    CRGB slots[15];

slots[0].r=255;
slots[0].g=255;
slots[0].b=0;
slots[1].r=255;
slots[1].g=255;
slots[1].b=0;
slots[2].r=255;
slots[2].g=128;
slots[2].b=0;
slots[3].r=255;
slots[3].g=128;
slots[3].b=64;
slots[4].r=255;
slots[4].g=255;
slots[4].b=0;
slots[5].r=255;
slots[5].g=255;
slots[5].b=0;
slots[6].r=255;
slots[6].g=128;
slots[6].b=64;
slots[7].r=255;
slots[7].g=128;
slots[7].b=64;
slots[8].r=255;
slots[8].g=128;
slots[8].b=0;
slots[9].r=255;
slots[9].g=255;
slots[9].b=0;
slots[10].r=255;
slots[10].g=255;
slots[10].b=0;
slots[11].r=255;
slots[11].g=255;
slots[11].b=0;
slots[12].r=255;
slots[12].g=128;
slots[12].b=64;
slots[13].r=255;
slots[13].g=128;
slots[13].b=0;
slots[14].r=255;
slots[14].g=255;
slots[14].b=0;
slots[15].r=255;
slots[15].g=255;
slots[15].b=0;
CRGB background;
    background.r = 255;
    background.g = 255;
    background.b = 0;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
                        }
void GeneratedPalette4(){
    CRGB slots[15];

slots[0].r=255;
slots[0].g=0;
slots[0].b=0;
slots[1].r=255;
slots[1].g=128;
slots[1].b=0;
slots[2].r=255;
slots[2].g=0;
slots[2].b=0;
slots[3].r=255;
slots[3].g=128;
slots[3].b=0;
slots[4].r=255;
slots[4].g=0;
slots[4].b=0;
slots[5].r=255;
slots[5].g=128;
slots[5].b=0;
slots[6].r=255;
slots[6].g=0;
slots[6].b=0;
slots[7].r=255;
slots[7].g=128;
slots[7].b=0;
slots[8].r=255;
slots[8].g=0;
slots[8].b=0;
slots[9].r=255;
slots[9].g=128;
slots[9].b=0;
slots[10].r=255;
slots[10].g=0;
slots[10].b=0;
slots[11].r=255;
slots[11].g=128;
slots[11].b=0;
slots[12].r=255;
slots[12].g=0;
slots[12].b=0;
slots[13].r=255;
slots[13].g=128;
slots[13].b=0;
slots[14].r=255;
slots[14].g=0;
slots[14].b=0;
slots[15].r=255;
slots[15].g=128;
slots[15].b=0;
CRGB background;
    background.r = 255;
    background.g = 0;
    background.b = 0;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
                        }
void GeneratedPalette5(){
    CRGB slots[15];

slots[0].r=0;
slots[0].g=128;
slots[0].b=192;
slots[1].r=0;
slots[1].g=128;
slots[1].b=255;
slots[2].r=0;
slots[2].g=128;
slots[2].b=192;
slots[3].r=0;
slots[3].g=128;
slots[3].b=255;
slots[4].r=0;
slots[4].g=128;
slots[4].b=128;
slots[5].r=0;
slots[5].g=128;
slots[5].b=128;
slots[6].r=0;
slots[6].g=128;
slots[6].b=128;
slots[7].r=0;
slots[7].g=128;
slots[7].b=128;
slots[8].r=0;
slots[8].g=128;
slots[8].b=255;
slots[9].r=0;
slots[9].g=128;
slots[9].b=192;
slots[10].r=0;
slots[10].g=128;
slots[10].b=255;
slots[11].r=0;
slots[11].g=128;
slots[11].b=192;
slots[12].r=0;
slots[12].g=128;
slots[12].b=128;
slots[13].r=0;
slots[13].g=128;
slots[13].b=128;
slots[14].r=0;
slots[14].g=128;
slots[14].b=128;
slots[15].r=0;
slots[15].g=128;
slots[15].b=128;
CRGB background;
    background.r = 0;
    background.g = 128;
    background.b = 128;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
                        }
void GeneratedPalette6(){
    CRGB slots[15];

slots[0].r=255;
slots[0].g=128;
slots[0].b=255;
slots[1].r=0;
slots[1].g=0;
slots[1].b=0;
slots[2].r=0;
slots[2].g=0;
slots[2].b=0;
slots[3].r=0;
slots[3].g=0;
slots[3].b=0;
slots[4].r=255;
slots[4].g=0;
slots[4].b=255;
slots[5].r=0;
slots[5].g=0;
slots[5].b=0;
slots[6].r=0;
slots[6].g=0;
slots[6].b=0;
slots[7].r=0;
slots[7].g=0;
slots[7].b=0;
slots[8].r=255;
slots[8].g=128;
slots[8].b=255;
slots[9].r=0;
slots[9].g=0;
slots[9].b=0;
slots[10].r=0;
slots[10].g=0;
slots[10].b=0;
slots[11].r=0;
slots[11].g=0;
slots[11].b=0;
slots[12].r=255;
slots[12].g=0;
slots[12].b=255;
slots[13].r=0;
slots[13].g=0;
slots[13].b=0;
slots[14].r=0;
slots[14].g=0;
slots[14].b=0;
slots[15].r=0;
slots[15].g=0;
slots[15].b=0;
CRGB background;
    background.r = 0;
    background.g = 0;
    background.b = 0;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
                        }
void GeneratedPalette7(){
    CRGB slots[15];

slots[0].r=0;
slots[0].g=128;
slots[0].b=255;
slots[1].r=0;
slots[1].g=128;
slots[1].b=255;
slots[2].r=128;
slots[2].g=255;
slots[2].b=128;
slots[3].r=128;
slots[3].g=255;
slots[3].b=128;
slots[4].r=255;
slots[4].g=255;
slots[4].b=0;
slots[5].r=255;
slots[5].g=255;
slots[5].b=0;
slots[6].r=128;
slots[6].g=255;
slots[6].b=128;
slots[7].r=128;
slots[7].g=255;
slots[7].b=128;
slots[8].r=0;
slots[8].g=128;
slots[8].b=255;
slots[9].r=0;
slots[9].g=128;
slots[9].b=255;
slots[10].r=128;
slots[10].g=255;
slots[10].b=128;
slots[11].r=128;
slots[11].g=255;
slots[11].b=128;
slots[12].r=255;
slots[12].g=128;
slots[12].b=0;
slots[13].r=255;
slots[13].g=128;
slots[13].b=0;
slots[14].r=128;
slots[14].g=255;
slots[14].b=128;
slots[15].r=128;
slots[15].g=255;
slots[15].b=128;
CRGB background;
    background.r = 128;
    background.g = 255;
    background.b = 128;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}

///////////////
//PREMADE-----
void Blackout() {
  currentPalette = CRGBPalette16(CRGB::Black);
}

void FullWhitePalletteWithEnds(){
  currentPalette = CRGBPalette16(CRGB::White);
  currentPalette[0] = primaryColor;
  currentPalette[1] = primaryColor;
  currentPalette[14] = secondaryColor;
  currentPalette[15] = secondaryColor;
}

void FullWhitePallette() {
  currentPalette = CRGBPalette16(CRGB::White);
}

void Aqua(){
    currentBlending = LINEARBLEND;
    CRGB slots[15];
    slots[0].r=0;
    slots[0].g=255;
    slots[0].b=255;
    slots[1].r=128;
    slots[1].g=255;
    slots[1].b=255;
    slots[2].r=0;
    slots[2].g=255;
    slots[2].b=255;
    slots[3].r=128;
    slots[3].g=255;
    slots[3].b=255;
    slots[4].r=0;
    slots[4].g=255;
    slots[4].b=255;
    slots[5].r=128;
    slots[5].g=255;
    slots[5].b=255;
    slots[6].r=0;
    slots[6].g=255;
    slots[6].b=255;
    slots[7].r=128;
    slots[7].g=255;
    slots[7].b=255;
    slots[8].r=128;
    slots[8].g=255;
    slots[8].b=255;
    slots[9].r=0;
    slots[9].g=255;
    slots[9].b=255;
    slots[10].r=0;
    slots[10].g=255;
    slots[10].b=255;
    slots[11].r=0;
    slots[11].g=255;
    slots[11].b=255;
    slots[12].r=128;
    slots[12].g=255;
    slots[12].b=255;
    slots[13].r=0;
    slots[13].g=255;
    slots[13].b=255;
    slots[14].r=128;
    slots[14].g=255;
    slots[14].b=255;
    slots[15].r=0;
    slots[15].g=255;
    slots[15].b=255;
    currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
/////////////////////
//PREMADE BEAT SABER

void RightTurnOn(){
    currentBlending = NOBLEND;
    currentPalette = primaryColorPalette;
}

void LeftTurnOn(){
    currentBlending = NOBLEND;
    currentPalette = secondaryColorPalette;
}

void RightFlashAndLeaveOn(){
    currentBlending = NOBLEND;
    currentPalette = primaryColorPalette;
}

void LeftFlashAndLeaveOn(){
    currentBlending = NOBLEND;
    currentPalette = secondaryColorPalette;
}

void RightFlashAndTurnOff(){
    currentPalette = primaryColorPalette;
}

void LeftFlashAndTurnOff(){
    currentPalette = secondaryColorPalette;
}

void TurnOff(){
  Blackout();
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;
}

void SetupTwoColPalette()
{
  CRGB purple = CHSV( HUE_YELLOW, 255, 255);
  CRGB green  = CHSV( HUE_BLUE, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  black,  black,
                     purple, purple, black,  black,
                     green,  green,  black,  black,
                     purple, purple, black,  black );
}

CRGB newColor = CRGB::Black;
bool fadeIn = true;

CRGB FadeColorInAndOut(CRGB color) {
  int red = color.r;
  int green = color.g;
  int blue = color.b;
  if (red > 254) {
    fadeIn = false;
  }

  if (red <= 10) {
    fadeIn = true;
  }

  if (fadeIn == true) {
    red = red + 20;
  } else {
    red--;
  }

  CRGB newColor = CRGB(red, green, blue);
  return newColor;
}

void SetupChristmasTree() {
  CRGB previousColor = newColor;
  newColor = FadeColorInAndOut(previousColor);
  CRGB fadedColor = newColor;
  CRGB darkGreen = CHSV(112, 200, 200);
  fill_solid(currentPalette, NUM_LEDS, darkGreen);
  //int ySlot = random(0,16);
  //int gSlot = random(0,16);
  //int bSlot = random(0,16);
  //int rSlot = random(0,16);
  //currentPalette[ySlot] = fadedColor;
  //currentPalette[gSlot] = CRGB::White;
  //currentPalette[bSlot] = CRGB::White;
  //currentPalette[rSlot] = CRGB::White;
  currentPalette[1] = fadedColor;
}

void SetupChristmas() {
  fill_solid(currentPalette, NUM_LEDS, CRGB::Green);
  currentPalette[0] = CRGB::Red;
  currentPalette[3] = CRGB::White;
  currentPalette[7] = CRGB::Red;
  currentPalette[11] = CRGB::White;
}

void SetupRedAndWhite() {
  fill_solid( currentPalette, NUM_LEDS, CRGB::Red);
  currentPalette[0] = CRGB::White;
  currentPalette[8] = CRGB::White;
}

DEFINE_GRADIENT_PALETTE( heatmap1_gp ) {
  0,     0,  0,  0,   //black
  128,   255,  0,  0,   //red
  224,   255, 255,  0,  //bright yellow
  255,   255, 255, 255
}; //full white

DEFINE_GRADIENT_PALETTE( heatmap2_gp ) {
  0,   0,  0,  0,
  50,  255, 255,  0,
  100, 0,  0,  0,
  150, 255, 255,  0,
  200, 0,  0,  0,
  250, 255, 255,  0,
};

/////////////// ANIMATIONS ////////////////////
void AnimationSolid() {
  BRIGHTNESS = SelectedBRIGHTNESS;
  playAnimation = true;
  FillLEDsFromPaletteColors(solidOffset);
  ShowLED();
}

void AnimationBurst() {
  BRIGHTNESS = SelectedBRIGHTNESS;
  playAnimation = true;
  if (playAnimation == true) {
    //get old index and increase * speed
    animationTimer -= (1 * speedsArray[animationSpeedIndex]);
    if (animationTimer < -200 || animationTimer > 200) {
      playAnimation = false;
      Blackout();
      animationTimer = 0;
    }
  } else {
    //set index to zero to reset animation
    animationTimer = 0;
  }

  FillLEDsFromPaletteColors(animationTimer);
  ShowLED();
}

void AnimationSpots() {
  BRIGHTNESS = SelectedBRIGHTNESS;
  playAnimation = true;
  if (playAnimation == true) {
    //get old index and increase * speed
    animationTimer -= (1 * speedsArray[animationSpeedIndex]);
    if (animationTimer < -200 || animationTimer > 200) {
      playAnimation = false;
      Blackout();
      animationTimer = 0;
    }
  } else {
    //set index to zero to reset animation
    animationTimer = 0;
  }

  scaling = 4;

  FillLEDsFromPaletteColors(animationTimer);
  ShowLED();

  scaling = 1;
}

bool scaleUp = true;

void AnimationScale(){
  playAnimation = true;
  if (playAnimation == true) {
    //get old index and increase * speed
    animationTimer -= (1 * speedsArray[animationSpeedIndex]);
    if (animationTimer < -200 || animationTimer > 200) {
      playAnimation = false;
      animationTimer = 0;
    }
  } else {
    //set index to zero to reset animation
    animationTimer = 0;
  }

  if(scaleUp){
    scaling+0.1;
    if(scaling > 2){
      scaleUp = false;
    }
  }else{
    scaling-0.1;
    if(scaling < 0.005){
      scaleUp = true;
    }
  }
  
  FillLEDsFromPaletteColors(animationTimer);
  ShowLED();
}

bool breathIn = true;

void AnimationBreath(){
  if(breathIn){
    BRIGHTNESS++;
    if (BRIGHTNESS > BRIGHTNESS_MAX){
      breathIn = false;
    }
  }else{
    BRIGHTNESS--;
    if (BRIGHTNESS < BRIGHTNESS_MIN){
      breathIn = true;
    }
  }
  
  FillLEDsFromPaletteColors(0);
  ShowLED();
}

void ShowLED(){
  if(enableShowing){
      FastLED.show(); 
  }
}
