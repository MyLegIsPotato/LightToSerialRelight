#include <FastLED.h>

#define LED_PIN     9
#define NUM_LEDS    205
int SelectedBRIGHTNESS = 120;
int BRIGHTNESS = 121;
int BRIGHTNESS_MAX = 255;
int BRIGHTNESS_MIN = 1;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

int UPDATES_PER_SECOND = 120;
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

CRGBPalette16 primaryColorPalette = CRGBPalette16(CRGB::Blue);
CRGBPalette16 secondaryColorPalette = CRGBPalette16(CRGB::Red);
CRGBPalette16 mixedColorPalette;
CRGBPalette16 mixedSpacedColorPalette;
CRGBPalette16 receivedPalette;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

const int buttonOnePin = 2;     // the number of the pushbutton pin
const int buttonTwoPin = 3;     // the number of the pushbutton pin
const int buttonThreePin = 4;     // the number of the pushbutton pin
const int microphonePin = 11;     // the number of the pushbutton pin

//Control LEDs pins:
const int paletteSelectionPin = 8;
const int useMicSelectionPin = 12;
const int brightnessSelectionPin = 11;
const int listenIndicatorPin = 10;

int buttonOne = 0;         // variable for reading the pushbutton status
int buttonTwo = 0;
int buttonThree = 0;

int menuPos = 1;
int menuNum = 0;
int menusCount = 2;

bool buttonOnePressed = false;
bool buttonTwoPressed = false;
bool buttonThreePressed = false;

bool animationReverse = true;
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
uint8_t bassTimer = 0;

bool receivingColorData = false;
bool receivingAnimationData = false;
bool enableShowing = true;
bool rainbowMode = false;
byte colorData[6];
byte byteBuffer[3];

CRGB primaryColor = CRGB::Blue;
CRGB secondaryColor = CRGB::Red;
CRGB newColor2 = CRGB::Red;

char inputChar;

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

void ClearBuffer() {
  for (int i; i < 64; i++) {
    int trash = Serial.read();
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

  pinMode(paletteSelectionPin, OUTPUT);
  pinMode(useMicSelectionPin, OUTPUT);
  pinMode(brightnessSelectionPin, OUTPUT);
  pinMode(listenIndicatorPin, OUTPUT);

  Serial.begin(1200);
  FastLED.setMaxRefreshRate(120);
  //Serial.println(newColor2.r);
}

bool bass = true;
bool useMic = true;

void loop() {
  // read the state of the pushbutton value:
  buttonOne = digitalRead(buttonOnePin);
  buttonTwo = digitalRead(buttonTwoPin);
  buttonThree = digitalRead(buttonThreePin);

  if (useMic) {
    digitalWrite(listenIndicatorPin, HIGH);
  } else {
    digitalWrite(listenIndicatorPin, LOW);
  }

  //Set the output of control LEDs:
  switch (menuNum) {
    case 0:
      digitalWrite(paletteSelectionPin, HIGH);
      digitalWrite(useMicSelectionPin, LOW);
      digitalWrite(brightnessSelectionPin, LOW);
      break;
    case 1:
      digitalWrite(paletteSelectionPin, LOW);
      digitalWrite(useMicSelectionPin, HIGH);
      digitalWrite(brightnessSelectionPin, LOW);
      break;
    case 2:
      digitalWrite(paletteSelectionPin, LOW);
      digitalWrite(useMicSelectionPin, LOW);
      digitalWrite(brightnessSelectionPin, HIGH);
      break;
    default:
      break;
  }


  if (useMic && (analogRead(A3) < 30) && bassTimer > 6) {
    int newColInt = random(0, 26);
    int newAnimInt = random(0, 3);
    while (newColInt == menuPos) {
      newColInt = random(0, 8);
    }
    menuPos = newColInt;
    currentAnimation = newAnimInt;
    bass = !bass;
    bassTimer = 0;
  }
  bassTimer++;

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
  if (buttonOne == HIGH && !buttonOnePressed && menuPos <= 25) {
    if (menuNum == 0) {
      menuPos++;
    } else if (menuNum == 1) {
      //IncreaseAnimationSpeedIndex(true);
      useMic = true;
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
      useMic = false;
      menuPos = 1;
      currentAnimation = 0;
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
  if (timer > 20) {
    input = Serial.read();
    inputChar = input;
    //Serial.println(inputChar);

    CheckForAnimationData();
    CheckForStaticPalette();
    CheckForRainbowMode();

    if (inputChar == '-') { //Feedback byte for LED Controller and Beat Saber mod
      Serial.write('a');
    }

    if (inputChar == '%') { //End receiving Beat Saber colors
      primaryColor.r = colorData[0];
      primaryColor.g = colorData[1];
      primaryColor.b = colorData[2];
      secondaryColor.r = colorData[3];
      secondaryColor.g = colorData[4];
      secondaryColor.b = colorData[5];
      SetupBSPalettes();
      BRIGHTNESS = 255;
      enableShowing = true;
    }

    if (!enableShowing) {
      Serial.readBytes(colorData, 6);
    }

    if (!receivingColorData && !receivingAnimationData) {
      //These are arduino commands. They are single characters so arduino can process them instantly.
      //This is basically code for your PC to communicate with arduino, if you want you can just send ASCII bytes equivalents
      //Serial.println(inputChar); // <--- You can debug things here
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

        case '@': menuPos = 1; currentAnimation = 0; ClearBuffer(); break; //Closes Connection and enables other effects.

        //BeatSaber mod palettes:
        case '#': menuPos = 20; break;
        case 'q': menuPos = 21; break;
        case 'w': menuPos = 22; break;
        case 'e': menuPos = 23; break;
        case 'r': menuPos = 24; break;
        case 't': menuPos = 25; break;
        case 'y': menuPos = 26; break;
        case 'p': menuPos = 29; break;
        case '^': SetupBSPalettesRainbow(); break;
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
    case 0: GeneratedPalette0(); break;
    case 1: GeneratedPalette1(); break;
    case 2: GeneratedPalette2(); break;
    case 3: Aqua(); break;
    case 4: GeneratedPalette3(); break;
    case 5: GeneratedPalette4(); break;
    case 6: GeneratedPalette5(); break;
    case 7: GeneratedPalette6(); break;
    case 8: GeneratedPalette7(); break;
    case 9: GeneratedPalette0(); break;
    case 10: StaticPalette(); break;
    case 11: GeneratedPalette8(); break;
    case 12: GeneratedPalette9(); break;
    case 13: GeneratedPalette10(); break;
    case 14: GeneratedPalette11(); break;
    case 15: GeneratedPalette12(); break;
    case 16: GeneratedPalette13(); break;
    case 17: GeneratedPalette14(); break;
    case 18: GeneratedPalette15(); break;
    case 19: GeneratedPalette16(); break;
    case 20: GeneratedPalette17(); break;
    //case 11:
    //case 12:

    //case 20: FullWhitePalletteWithEnds(); currentAnimation = 4; break;
    case 21: RightTurnOn(); currentAnimation = 0; break;
    case 22: LeftTurnOn(); currentAnimation = 0; break;
    case 23: RightFlashAndLeaveOn(); currentAnimation = 1; break;
    case 24: LeftFlashAndLeaveOn(); currentAnimation = 1; break;
    case 25: RightFlashAndTurnOff(); currentAnimation = 0; break;
    case 26: LeftFlashAndTurnOff(); currentAnimation = 0; break;
    case 29: TurnOff();
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

void CheckForAnimationData() {
  if (inputChar == '|') {
    receivingAnimationData = true;
    inputChar = Serial.read();
    int animationNumber = inputChar - '0';
    //Serial.print(animationNumber);
    currentAnimation = animationNumber;
  }
  if (inputChar == '\\') {
    receivingAnimationData = false;
  }
}

void CheckForRainbowMode() {
  if (inputChar == '^') {
    SetupBSPalettesRainbow();
    rainbowMode = true;
  }

  if (inputChar == '$') { //Start receiving Beat Saber colors
    rainbowMode = false;
    enableShowing = false;
  }
}

void CheckForStaticPalette() {
  if (inputChar == '+') {
    receivingColorData = true;
    for (int i = 0; i < 16; i++) {
      Serial.readBytes(byteBuffer, 3);
      CRGB newColor2;
      newColor2.r = byteBuffer[0];
      newColor2.g = byteBuffer[1];
      newColor2.b = byteBuffer[2];
      //Serial.println(newColor2.r);
      receivedPalette[i] = newColor2;
    }
    delay(20);
    input = Serial.read();
    inputChar = input;
    //Serial.println(inputChar);
    if (inputChar == '1') {
      currentBlending = LINEARBLEND;
    } else {
      currentBlending = NOBLEND;
    }
  }

  if (receivingColorData) {
    if (inputChar == '=') {
      //Serial.println(inputChar);
      receivingColorData = false;
      menuPos = 10;
      currentAnimation = 0;
      ClearBuffer();
    }
  }
}

CRGB rainbowColorBank[16] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Yellow,
                             CRGB::Chartreuse, CRGB::Crimson, CRGB::Cyan, CRGB::DarkMagenta,
                             CRGB::DeepPink, CRGB::DeepSkyBlue, CRGB::Honeydew, CRGB::LightCoral,
                             CRGB::LightSeaGreen, CRGB::MediumOrchid, CRGB::OrangeRed, CRGB::Teal
                            };

void SetupBSPalettesRainbow() {
  primaryColor = rainbowColorBank[random(0, 16)];
  secondaryColor = rainbowColorBank[random(0, 16)];
  SetupBSPalettes();
}

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

void SetupBSPalettes() {
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

CRGBPalette16 fillPalette(CRGB colorSlots[], CRGBPalette16 currentPalette, CRGB backgroundColor) {
  for (int i = 0; i < sizeof(colorSlots); i++) {
    currentPalette[i] = colorSlots[i];
  };
  for (int j = 0; j < (16 - sizeof(colorSlots)); j++) {
    int x = sizeof(colorSlots) + j;
    currentPalette[x] = backgroundColor;
  };
  return currentPalette;
}

void LoadDefaultSettings() {
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
//LIVE CONTROL
void StaticPalette() {
  currentPalette = receivedPalette;
}

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

void GeneratedPalette2() {
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


void GeneratedPalette3() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 255;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 255;
  slots[1].b = 0;
  slots[2].r = 255;
  slots[2].g = 128;
  slots[2].b = 0;
  slots[3].r = 255;
  slots[3].g = 128;
  slots[3].b = 64;
  slots[4].r = 255;
  slots[4].g = 255;
  slots[4].b = 0;
  slots[5].r = 255;
  slots[5].g = 255;
  slots[5].b = 0;
  slots[6].r = 255;
  slots[6].g = 128;
  slots[6].b = 64;
  slots[7].r = 255;
  slots[7].g = 128;
  slots[7].b = 64;
  slots[8].r = 255;
  slots[8].g = 128;
  slots[8].b = 0;
  slots[9].r = 255;
  slots[9].g = 255;
  slots[9].b = 0;
  slots[10].r = 255;
  slots[10].g = 255;
  slots[10].b = 0;
  slots[11].r = 255;
  slots[11].g = 255;
  slots[11].b = 0;
  slots[12].r = 255;
  slots[12].g = 128;
  slots[12].b = 64;
  slots[13].r = 255;
  slots[13].g = 128;
  slots[13].b = 0;
  slots[14].r = 255;
  slots[14].g = 255;
  slots[14].b = 0;
  slots[15].r = 255;
  slots[15].g = 255;
  slots[15].b = 0;
  CRGB background;
  background.r = 255;
  background.g = 255;
  background.b = 0;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
void GeneratedPalette4() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 0;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 128;
  slots[1].b = 0;
  slots[2].r = 255;
  slots[2].g = 0;
  slots[2].b = 0;
  slots[3].r = 255;
  slots[3].g = 128;
  slots[3].b = 0;
  slots[4].r = 255;
  slots[4].g = 0;
  slots[4].b = 0;
  slots[5].r = 255;
  slots[5].g = 128;
  slots[5].b = 0;
  slots[6].r = 255;
  slots[6].g = 0;
  slots[6].b = 0;
  slots[7].r = 255;
  slots[7].g = 128;
  slots[7].b = 0;
  slots[8].r = 255;
  slots[8].g = 0;
  slots[8].b = 0;
  slots[9].r = 255;
  slots[9].g = 128;
  slots[9].b = 0;
  slots[10].r = 255;
  slots[10].g = 0;
  slots[10].b = 0;
  slots[11].r = 255;
  slots[11].g = 128;
  slots[11].b = 0;
  slots[12].r = 255;
  slots[12].g = 0;
  slots[12].b = 0;
  slots[13].r = 255;
  slots[13].g = 128;
  slots[13].b = 0;
  slots[14].r = 255;
  slots[14].g = 0;
  slots[14].b = 0;
  slots[15].r = 255;
  slots[15].g = 128;
  slots[15].b = 0;
  CRGB background;
  background.r = 255;
  background.g = 0;
  background.b = 0;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
void GeneratedPalette5() {
  CRGB slots[15];

  slots[0].r = 0;
  slots[0].g = 128;
  slots[0].b = 192;
  slots[1].r = 0;
  slots[1].g = 128;
  slots[1].b = 255;
  slots[2].r = 0;
  slots[2].g = 128;
  slots[2].b = 192;
  slots[3].r = 0;
  slots[3].g = 128;
  slots[3].b = 255;
  slots[4].r = 0;
  slots[4].g = 128;
  slots[4].b = 128;
  slots[5].r = 0;
  slots[5].g = 128;
  slots[5].b = 128;
  slots[6].r = 0;
  slots[6].g = 128;
  slots[6].b = 128;
  slots[7].r = 0;
  slots[7].g = 128;
  slots[7].b = 128;
  slots[8].r = 0;
  slots[8].g = 128;
  slots[8].b = 255;
  slots[9].r = 0;
  slots[9].g = 128;
  slots[9].b = 192;
  slots[10].r = 0;
  slots[10].g = 128;
  slots[10].b = 255;
  slots[11].r = 0;
  slots[11].g = 128;
  slots[11].b = 192;
  slots[12].r = 0;
  slots[12].g = 128;
  slots[12].b = 128;
  slots[13].r = 0;
  slots[13].g = 128;
  slots[13].b = 128;
  slots[14].r = 0;
  slots[14].g = 128;
  slots[14].b = 128;
  slots[15].r = 0;
  slots[15].g = 128;
  slots[15].b = 128;
  CRGB background;
  background.r = 0;
  background.g = 128;
  background.b = 128;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
void GeneratedPalette6() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 128;
  slots[0].b = 255;
  slots[1].r = 0;
  slots[1].g = 0;
  slots[1].b = 0;
  slots[2].r = 0;
  slots[2].g = 0;
  slots[2].b = 0;
  slots[3].r = 0;
  slots[3].g = 0;
  slots[3].b = 0;
  slots[4].r = 255;
  slots[4].g = 0;
  slots[4].b = 255;
  slots[5].r = 0;
  slots[5].g = 0;
  slots[5].b = 0;
  slots[6].r = 0;
  slots[6].g = 0;
  slots[6].b = 0;
  slots[7].r = 0;
  slots[7].g = 0;
  slots[7].b = 0;
  slots[8].r = 255;
  slots[8].g = 128;
  slots[8].b = 255;
  slots[9].r = 0;
  slots[9].g = 0;
  slots[9].b = 0;
  slots[10].r = 0;
  slots[10].g = 0;
  slots[10].b = 0;
  slots[11].r = 0;
  slots[11].g = 0;
  slots[11].b = 0;
  slots[12].r = 255;
  slots[12].g = 0;
  slots[12].b = 255;
  slots[13].r = 0;
  slots[13].g = 0;
  slots[13].b = 0;
  slots[14].r = 0;
  slots[14].g = 0;
  slots[14].b = 0;
  slots[15].r = 0;
  slots[15].g = 0;
  slots[15].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
void GeneratedPalette7() {
  CRGB slots[15];

  slots[0].r = 0;
  slots[0].g = 128;
  slots[0].b = 255;
  slots[1].r = 0;
  slots[1].g = 128;
  slots[1].b = 255;
  slots[2].r = 128;
  slots[2].g = 255;
  slots[2].b = 128;
  slots[3].r = 128;
  slots[3].g = 255;
  slots[3].b = 128;
  slots[4].r = 255;
  slots[4].g = 255;
  slots[4].b = 0;
  slots[5].r = 255;
  slots[5].g = 255;
  slots[5].b = 0;
  slots[6].r = 128;
  slots[6].g = 255;
  slots[6].b = 128;
  slots[7].r = 128;
  slots[7].g = 255;
  slots[7].b = 128;
  slots[8].r = 0;
  slots[8].g = 128;
  slots[8].b = 255;
  slots[9].r = 0;
  slots[9].g = 128;
  slots[9].b = 255;
  slots[10].r = 128;
  slots[10].g = 255;
  slots[10].b = 128;
  slots[11].r = 128;
  slots[11].g = 255;
  slots[11].b = 128;
  slots[12].r = 255;
  slots[12].g = 128;
  slots[12].b = 0;
  slots[13].r = 255;
  slots[13].g = 128;
  slots[13].b = 0;
  slots[14].r = 128;
  slots[14].g = 255;
  slots[14].b = 128;
  slots[15].r = 128;
  slots[15].g = 255;
  slots[15].b = 128;
  CRGB background;
  background.r = 128;
  background.g = 255;
  background.b = 128;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}


void GeneratedPalette8() {
  CRGB slots[9];

  slots[0].r = 255;
  slots[0].g = 0;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 0;
  slots[1].b = 0;
  slots[2].r = 0;
  slots[2].g = 0;
  slots[2].b = 0;
  slots[3].r = 0;
  slots[3].g = 0;
  slots[3].b = 0;
  slots[4].r = 0;
  slots[4].g = 0;
  slots[4].b = 0;
  slots[5].r = 255;
  slots[5].g = 0;
  slots[5].b = 0;
  slots[6].r = 255;
  slots[6].g = 0;
  slots[6].b = 0;
  slots[7].r = 0;
  slots[7].g = 0;
  slots[7].b = 0;
  slots[8].r = 0;
  slots[8].g = 0;
  slots[8].b = 0;
  slots[9].r = 0;
  slots[9].g = 0;
  slots[9].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette9() {
  CRGB slots[15];

  slots[0].r = 0;
  slots[0].g = 255;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 255;
  slots[1].b = 0;
  slots[2].r = 255;
  slots[2].g = 255;
  slots[2].b = 255;
  slots[3].r = 255;
  slots[3].g = 255;
  slots[3].b = 0;
  slots[4].r = 0;
  slots[4].g = 255;
  slots[4].b = 0;
  slots[5].r = 255;
  slots[5].g = 255;
  slots[5].b = 0;
  slots[6].r = 255;
  slots[6].g = 255;
  slots[6].b = 255;
  slots[7].r = 255;
  slots[7].g = 255;
  slots[7].b = 0;
  slots[8].r = 0;
  slots[8].g = 255;
  slots[8].b = 0;
  slots[9].r = 255;
  slots[9].g = 255;
  slots[9].b = 0;
  slots[10].r = 255;
  slots[10].g = 255;
  slots[10].b = 255;
  slots[11].r = 255;
  slots[11].g = 255;
  slots[11].b = 0;
  slots[12].r = 0;
  slots[12].g = 255;
  slots[12].b = 0;
  slots[13].r = 255;
  slots[13].g = 255;
  slots[13].b = 0;
  slots[14].r = 255;
  slots[14].g = 255;
  slots[14].b = 255;
  slots[15].r = 255;
  slots[15].g = 255;
  slots[15].b = 0;
  CRGB background;
  background.r = 255;
  background.g = 255;
  background.b = 255;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette10() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 0;
  slots[0].b = 0;
  slots[1].r = 128;
  slots[1].g = 64;
  slots[1].b = 64;
  slots[2].r = 128;
  slots[2].g = 0;
  slots[2].b = 0;
  slots[3].r = 64;
  slots[3].g = 0;
  slots[3].b = 0;
  slots[4].r = 0;
  slots[4].g = 0;
  slots[4].b = 0;
  slots[5].r = 0;
  slots[5].g = 0;
  slots[5].b = 0;
  slots[6].r = 0;
  slots[6].g = 0;
  slots[6].b = 0;
  slots[7].r = 0;
  slots[7].g = 0;
  slots[7].b = 0;
  slots[8].r = 0;
  slots[8].g = 0;
  slots[8].b = 0;
  slots[9].r = 0;
  slots[9].g = 0;
  slots[9].b = 0;
  slots[10].r = 0;
  slots[10].g = 0;
  slots[10].b = 0;
  slots[11].r = 0;
  slots[11].g = 0;
  slots[11].b = 0;
  slots[12].r = 0;
  slots[12].g = 0;
  slots[12].b = 0;
  slots[13].r = 64;
  slots[13].g = 0;
  slots[13].b = 0;
  slots[14].r = 128;
  slots[14].g = 0;
  slots[14].b = 0;
  slots[15].r = 255;
  slots[15].g = 0;
  slots[15].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette11() {
  CRGB slots[15];

  slots[0].r = 128;
  slots[0].g = 0;
  slots[0].b = 64;
  slots[1].r = 255;
  slots[1].g = 0;
  slots[1].b = 128;
  slots[2].r = 128;
  slots[2].g = 0;
  slots[2].b = 64;
  slots[3].r = 255;
  slots[3].g = 0;
  slots[3].b = 128;
  slots[4].r = 128;
  slots[4].g = 0;
  slots[4].b = 64;
  slots[5].r = 255;
  slots[5].g = 0;
  slots[5].b = 128;
  slots[6].r = 128;
  slots[6].g = 0;
  slots[6].b = 64;
  slots[7].r = 255;
  slots[7].g = 0;
  slots[7].b = 128;
  slots[8].r = 128;
  slots[8].g = 0;
  slots[8].b = 64;
  slots[9].r = 255;
  slots[9].g = 0;
  slots[9].b = 128;
  slots[10].r = 128;
  slots[10].g = 0;
  slots[10].b = 64;
  slots[11].r = 255;
  slots[11].g = 0;
  slots[11].b = 128;
  slots[12].r = 128;
  slots[12].g = 0;
  slots[12].b = 64;
  slots[13].r = 255;
  slots[13].g = 0;
  slots[13].b = 128;
  slots[14].r = 128;
  slots[14].g = 0;
  slots[14].b = 64;
  slots[15].r = 255;
  slots[15].g = 0;
  slots[15].b = 128;
  CRGB background;
  background.r = 255;
  background.g = 0;
  background.b = 128;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette12() {
  CRGB slots[8];

  slots[0].r = 0;
  slots[0].g = 0;
  slots[0].b = 160;
  slots[1].r = 0;
  slots[1].g = 0;
  slots[1].b = 255;
  slots[2].r = 0;
  slots[2].g = 0;
  slots[2].b = 128;
  slots[3].r = 128;
  slots[3].g = 128;
  slots[3].b = 255;
  slots[4].r = 0;
  slots[4].g = 128;
  slots[4].b = 192;
  slots[5].r = 128;
  slots[5].g = 128;
  slots[5].b = 192;
  slots[6].r = 0;
  slots[6].g = 255;
  slots[6].b = 255;
  slots[7].r = 128;
  slots[7].g = 255;
  slots[7].b = 255;
  slots[8].r = 0;
  slots[8].g = 64;
  slots[8].b = 128;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette13() {
  CRGB slots[7];

  slots[0].r = 64;
  slots[0].g = 128;
  slots[0].b = 128;
  slots[1].r = 0;
  slots[1].g = 128;
  slots[1].b = 0;
  slots[2].r = 0;
  slots[2].g = 255;
  slots[2].b = 0;
  slots[3].r = 0;
  slots[3].g = 128;
  slots[3].b = 0;
  slots[4].r = 64;
  slots[4].g = 128;
  slots[4].b = 128;
  slots[5].r = 0;
  slots[5].g = 128;
  slots[5].b = 0;
  slots[6].r = 0;
  slots[6].g = 255;
  slots[6].b = 0;
  slots[7].r = 0;
  slots[7].g = 128;
  slots[7].b = 0;
  CRGB background;
  background.r = 64;
  background.g = 128;
  background.b = 128;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette14() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 0;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 0;
  slots[1].b = 0;
  slots[2].r = 255;
  slots[2].g = 0;
  slots[2].b = 0;
  slots[3].r = 255;
  slots[3].g = 128;
  slots[3].b = 128;
  slots[4].r = 255;
  slots[4].g = 128;
  slots[4].b = 128;
  slots[5].r = 255;
  slots[5].g = 128;
  slots[5].b = 128;
  slots[6].r = 255;
  slots[6].g = 128;
  slots[6].b = 192;
  slots[7].r = 255;
  slots[7].g = 128;
  slots[7].b = 192;
  slots[8].r = 255;
  slots[8].g = 255;
  slots[8].b = 255;
  slots[9].r = 255;
  slots[9].g = 255;
  slots[9].b = 255;
  slots[10].r = 255;
  slots[10].g = 255;
  slots[10].b = 255;
  slots[11].r = 255;
  slots[11].g = 255;
  slots[11].b = 255;
  slots[12].r = 255;
  slots[12].g = 255;
  slots[12].b = 255;
  slots[13].r = 255;
  slots[13].g = 255;
  slots[13].b = 255;
  slots[14].r = 255;
  slots[14].g = 255;
  slots[14].b = 255;
  slots[15].r = 255;
  slots[15].g = 255;
  slots[15].b = 255;
  CRGB background;
  background.r = 255;
  background.g = 255;
  background.b = 255;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette15() {
  CRGB slots[15];

  slots[0].r = 0;
  slots[0].g = 255;
  slots[0].b = 0;
  slots[1].r = 128;
  slots[1].g = 255;
  slots[1].b = 128;
  slots[2].r = 0;
  slots[2].g = 255;
  slots[2].b = 128;
  slots[3].r = 0;
  slots[3].g = 255;
  slots[3].b = 64;
  slots[4].r = 0;
  slots[4].g = 255;
  slots[4].b = 128;
  slots[5].r = 255;
  slots[5].g = 255;
  slots[5].b = 255;
  slots[6].r = 255;
  slots[6].g = 255;
  slots[6].b = 255;
  slots[7].r = 255;
  slots[7].g = 255;
  slots[7].b = 255;
  slots[8].r = 255;
  slots[8].g = 255;
  slots[8].b = 255;
  slots[9].r = 255;
  slots[9].g = 255;
  slots[9].b = 255;
  slots[10].r = 255;
  slots[10].g = 255;
  slots[10].b = 255;
  slots[11].r = 255;
  slots[11].g = 255;
  slots[11].b = 255;
  slots[12].r = 128;
  slots[12].g = 255;
  slots[12].b = 128;
  slots[13].r = 128;
  slots[13].g = 255;
  slots[13].b = 128;
  slots[14].r = 0;
  slots[14].g = 255;
  slots[14].b = 0;
  slots[15].r = 128;
  slots[15].g = 255;
  slots[15].b = 128;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette16() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 45;
  slots[0].b = 45;
  slots[1].r = 0;
  slots[1].g = 0;
  slots[1].b = 0;
  slots[2].r = 242;
  slots[2].g = 175;
  slots[2].b = 19;
  slots[3].r = 0;
  slots[3].g = 0;
  slots[3].b = 0;
  slots[4].r = 116;
  slots[4].g = 238;
  slots[4].b = 57;
  slots[5].r = 0;
  slots[5].g = 0;
  slots[5].b = 0;
  slots[6].r = 77;
  slots[6].g = 234;
  slots[6].b = 187;
  slots[7].r = 0;
  slots[7].g = 0;
  slots[7].b = 0;
  slots[8].r = 64;
  slots[8].g = 162;
  slots[8].b = 234;
  slots[9].r = 0;
  slots[9].g = 0;
  slots[9].b = 0;
  slots[10].r = 56;
  slots[10].g = 92;
  slots[10].b = 235;
  slots[11].r = 0;
  slots[11].g = 0;
  slots[11].b = 0;
  slots[12].r = 146;
  slots[12].g = 64;
  slots[12].b = 236;
  slots[13].r = 0;
  slots[13].g = 0;
  slots[13].b = 0;
  slots[14].r = 226;
  slots[14].g = 35;
  slots[14].b = 193;
  slots[15].r = 0;
  slots[15].g = 0;
  slots[15].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
void GeneratedPalette17() {
  CRGB slots[15];

  slots[0].r = 255;
  slots[0].g = 128;
  slots[0].b = 0;
  slots[1].r = 255;
  slots[1].g = 128;
  slots[1].b = 0;
  slots[2].r = 255;
  slots[2].g = 128;
  slots[2].b = 0;
  slots[3].r = 255;
  slots[3].g = 128;
  slots[3].b = 0;
  slots[4].r = 0;
  slots[4].g = 0;
  slots[4].b = 0;
  slots[5].r = 0;
  slots[5].g = 0;
  slots[5].b = 0;
  slots[6].r = 0;
  slots[6].g = 0;
  slots[6].b = 0;
  slots[7].r = 0;
  slots[7].g = 0;
  slots[7].b = 0;
  slots[8].r = 255;
  slots[8].g = 128;
  slots[8].b = 0;
  slots[9].r = 255;
  slots[9].g = 128;
  slots[9].b = 0;
  slots[10].r = 255;
  slots[10].g = 128;
  slots[10].b = 0;
  slots[11].r = 255;
  slots[11].g = 128;
  slots[11].b = 0;
  slots[12].r = 0;
  slots[12].g = 0;
  slots[12].b = 0;
  slots[13].r = 0;
  slots[13].g = 0;
  slots[13].b = 0;
  slots[14].r = 0;
  slots[14].g = 0;
  slots[14].b = 0;
  slots[15].r = 0;
  slots[15].g = 0;
  slots[15].b = 0;
  CRGB background;
  background.r = 0;
  background.g = 0;
  background.b = 0;
  currentPalette = fillPalette(slots, currentPalette, background);
}
///////////////
//PREMADE-----
void Blackout() {
  currentPalette = CRGBPalette16(CRGB::Black);
}

void FullWhitePalletteWithEnds() {
  currentPalette = CRGBPalette16(CRGB::White);
  currentPalette[0] = primaryColor;
  currentPalette[1] = primaryColor;
  currentPalette[14] = secondaryColor;
  currentPalette[15] = secondaryColor;
}

void FullWhitePallette() {
  currentPalette = CRGBPalette16(CRGB::White);
}

void Aqua() {
  currentBlending = LINEARBLEND;
  CRGB slots[15];
  slots[0].r = 0;
  slots[0].g = 255;
  slots[0].b = 255;
  slots[1].r = 128;
  slots[1].g = 255;
  slots[1].b = 255;
  slots[2].r = 0;
  slots[2].g = 255;
  slots[2].b = 255;
  slots[3].r = 128;
  slots[3].g = 255;
  slots[3].b = 255;
  slots[4].r = 0;
  slots[4].g = 255;
  slots[4].b = 255;
  slots[5].r = 128;
  slots[5].g = 255;
  slots[5].b = 255;
  slots[6].r = 0;
  slots[6].g = 255;
  slots[6].b = 255;
  slots[7].r = 128;
  slots[7].g = 255;
  slots[7].b = 255;
  slots[8].r = 128;
  slots[8].g = 255;
  slots[8].b = 255;
  slots[9].r = 0;
  slots[9].g = 255;
  slots[9].b = 255;
  slots[10].r = 0;
  slots[10].g = 255;
  slots[10].b = 255;
  slots[11].r = 0;
  slots[11].g = 255;
  slots[11].b = 255;
  slots[12].r = 128;
  slots[12].g = 255;
  slots[12].b = 255;
  slots[13].r = 0;
  slots[13].g = 255;
  slots[13].b = 255;
  slots[14].r = 128;
  slots[14].g = 255;
  slots[14].b = 255;
  slots[15].r = 0;
  slots[15].g = 255;
  slots[15].b = 255;
  currentPalette = CRGBPalette16(slots[0], slots[1], slots[2], slots[3], slots[4], slots[5], slots[6], slots[7], slots[8], slots[9], slots[10], slots[11], slots[12], slots[13], slots[14], slots[15] );
}
/////////////////////
//PREMADE BEAT SABER

void RightTurnOn() {
  currentBlending = NOBLEND;
  currentPalette = primaryColorPalette;
}

void LeftTurnOn() {
  currentBlending = NOBLEND;
  currentPalette = secondaryColorPalette;
}

void RightFlashAndLeaveOn() {
  currentBlending = NOBLEND;
  currentPalette = primaryColorPalette;
}

void LeftFlashAndLeaveOn() {
  currentBlending = NOBLEND;
  currentPalette = secondaryColorPalette;
}

void RightFlashAndTurnOff() {
  currentPalette = primaryColorPalette;
}

void LeftFlashAndTurnOff() {
  currentPalette = secondaryColorPalette;
}

void TurnOff() {
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
///////////////////////////////////////////////
/////////////// ANIMATIONS ////////////////////
///////////////////////////////////////////////
void AnimationSolid() {
  BRIGHTNESS = SelectedBRIGHTNESS;
  playAnimation = true;
  FillLEDsFromPaletteColors(solidOffset);
  ShowLED();
}

void AnimationBurst() {
  BRIGHTNESS = SelectedBRIGHTNESS;
  //get old index and increase * speed

  if (animationReverse) {
    animationTimer += (1 * speedsArray[animationSpeedIndex]);
  } else {
    animationTimer -= (1 * speedsArray[animationSpeedIndex]);
  }

  if (animationTimer < -255 || animationTimer > 255) {
    animationReverse = !animationReverse;
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
    if (animationTimer < -128 || animationTimer > 128) {
      playAnimation = false;
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

void AnimationScale() {
  //get old index and increase * speed
  animationTimer -= (1 * speedsArray[animationSpeedIndex]);
  if (animationTimer < -255 || animationTimer > 255) {
    animationTimer = 0;
  }

  if (scaleUp) {
    scaling + 0.1;
    if (scaling > 2) {
      scaleUp = false;
    }
  } else {
    scaling - 0.1;
    if (scaling < 0.005) {
      scaleUp = true;
    }
  }

  FillLEDsFromPaletteColors(animationTimer);
  ShowLED();
}

bool breathIn = true;

void AnimationBreath() {
  if (breathIn) {
    BRIGHTNESS++;
    if (BRIGHTNESS > BRIGHTNESS_MAX) {
      breathIn = false;
    }
  } else {
    BRIGHTNESS--;
    if (BRIGHTNESS < BRIGHTNESS_MIN) {
      breathIn = true;
    }
  }

  FillLEDsFromPaletteColors(0);
  ShowLED();
}

void ShowLED() {
  if (enableShowing) {
    FastLED.show();
  }
}
