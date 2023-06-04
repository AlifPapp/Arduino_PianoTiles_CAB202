#include <LiquidCrystal.h>

// Setup Pins
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13; // LCD pins
const int ButtonPin_MainControls[4] = {2,3,4,5}; // Main game controls 
const int ButtonPinMatrix_Colums[3] = {A0,A1,A2}; // Button Matrix columns [left to right]
const int ButtonPinMatrix_Rows[2] = {6,7}; // Button Matrix rows [top to bottom]
const int ButtonStart = A3; // Start/Stop game
const int PotentioPin = A4; // Set Pitch
const int BuzzerPIN = A5; // Play sound

// Matrix Buttons Mapping
int MatrixButtonMapping[2][3] = {
  {1, 3, 5},
  {2, 4, 6}
};

// Custom characters gylphs
const int numGLYPHS = 7;
byte glyphs[numGLYPHS][8] = {
  // barier
   {B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001}
  // note  [left]
  ,{B11000,
    B11111,
    B00010,
    B01100,
    B00000,
    B00000,
    B00000,
    B00000}
  // note  [right]
  ,{B00000,
    B00000,
    B00000,
    B00000,
    B11000,
    B11111,
    B00010,
    B01100}
  // barier + note  [left]
  ,{B11001,
    B11111,
    B00011,
    B01101,
    B00001,
    B00001,
    B00001,
    B00001}
  // barier + note  [right]
  ,{B00001,
    B00001,
    B00001,
    B00001,
    B11001,
    B11111,
    B00011,
    B01101}
  // note exploding  [left]
  ,{B10101,
    B01110,
    B01110,
    B10101,
    B00000,
    B00000,
    B00000,
    B00000}
  // note exploding  [right]
  ,{B00000,
    B00000,
    B00000,
    B00000,
    B10101,
    B01110,
    B01110,
    B10101}
};

// Custom characters
const char blankchar = 32;
const char barier = 1;
const char note[4][2] = {
  {2, blankchar}, {3, blankchar}, {blankchar, 2}, {blankchar, 3}
};
const char barierNnote[4][2] = {
  {4, barier}, {5, barier}, {barier, 4}, {barier, 5}
};
const char noteExploding[4][2] = {
  {6, blankchar}, {7, blankchar}, {blankchar, 6}, {blankchar, 7}
};
const char barierNnoteExploding[4][2] = {
  {6, barier}, {7, barier}, {barier, 6}, {barier, 7}
};

// Song: Boci-Boci Tarka
int song_position = 0;
const int numNOTES = 36;
const int notes_frequency[4] = {
   174.61 // F
  , 196.00 // G
  , 220.00 // A
  , 246.94 // B
};
const int song_blueprint[numNOTES][4] = { // 0 = blank | 1 = note
  {1,0,0,0}, 
  {0,1,0,0}, 
  {0,0,0,0}, 

  {1,0,0,0},
  {0,1,0,0}, 
  {0,0,0,0}, 

  {0,0,1,0}, 
  {0,0,1,0},
  {0,0,0,0}, 


  {1,0,0,0}, 
  {0,1,0,0}, 
  {0,0,0,0}, 

  {1,0,0,0},
  {0,1,0,0}, 
  {0,0,0,0}, 

  {0,0,1,0}, 
  {0,0,1,0},
  {0,0,0,0}, 


  {0,0,0,1}, 
  {0,0,0,1}, 
  {0,0,0,0}, 

  {0,0,1,0},
  {0,0,1,0}, 
  {0,0,0,0}, 

  {0,1,0,0}, 
  {0,0,1,0},
  {0,0,0,0}, 


  {0,0,1,0}, 
  {0,1,0,0}, 
  {0,0,0,0}, 

  {0,1,0,0},
  {1,0,0,0}, 
  {0,0,0,0}, 

  {1,0,0,0}, 
  {1,0,0,0},
  {0,0,0,0}
};

// Used for drawframe()
int char_position[16][2] = {0}; // {0 = blank, 1 = note, 2 = explode} and {1,2,3,4 = position}
int char_position2[16][2] = {0}; // previous frame
char char_string[16]; // String to be printed on LCD

// Settings variables
int song_speed = 300; //ms
int barier_height = 6; // min = 0, max = 15
int game_over = 1; // 0 = off, 1 = on
int pitch = 1; // min = 1, max = 1 + 1023/500

// your score
int time = 0;

// Setup LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);

  // Setup buttons // input = 0, output = 1
  for (int i = 0; i < 4; i++) {
    pinMode(ButtonPin_MainControls[i], 0);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(ButtonPinMatrix_Colums[i], 1);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(ButtonPinMatrix_Rows[i], 0);
  }
  pinMode(ButtonStart, 0);
  pinMode(PotentioPin, 0);
  pinMode(BuzzerPIN, 1);

  // create custom characters for the LCD
  for (int i=0; i < numGLYPHS; i++) {
    lcd.createChar(i + 1, glyphs[i]);
  }

  drawFrame();
  startScreen(0);
};

void loop() {
  // if matrix buttons pressed
  int pressed_BMatrix = getBMatrix(); // 0 = not pressed
  
  // if speed button pressed
  if (pressed_BMatrix == 1) { // faster speed
    song_speed = song_speed - 10; // reduce loop delay
    if (song_speed < 10) { // max speed
      song_speed = 10;
    }
    Serial.println("Increased speed\n");
  }
  if (pressed_BMatrix == 2) { // slower speed
    song_speed = song_speed + 100; // increase loop delay
    Serial.println("Decreased speed\n");
  }

  // if barier button pressed
  if (pressed_BMatrix == 3) { // increase barier height
    barier_height = barier_height + 1;
    if (barier_height > 15) { // max height
      barier_height = 15;
    }
    Serial.println("Barier elevated\n");
  }
  if (pressed_BMatrix == 4) { // decrease barier height
    barier_height = barier_height - 1;
    if (barier_height < 0) { // min height
      barier_height = 0;
    }
    Serial.println("Barier lowered\n");
  }

  // if reset button pressed
  if (pressed_BMatrix == 5) { // reset settings variables
    song_speed = 300;
    barier_height = 6;
    // pitch = 1;
    // game_over = 1;
    Serial.println("Settings reset\n");
  }

  // if toggle game over button pressed
  if (pressed_BMatrix == 6) { // toggle game over
    if (game_over == 1) { // game over on
      game_over = 0;
    } else { // game over off
      game_over = 1;
    }
    Serial.println("Toggled: Game over\n");
  }

  // get pitch (potentiometer)
  pitch = 1 + (readPotentio()/500);

  // get 16 notes type and position for drawframe()
  for (int i=song_position; i < (song_position+16); i++) {
    int array_pos = i-song_position;
    if ((i-32) > numNOTES) {
        song_position = 0;
      }
    
    if (i > 16-song_position) { // offset song by 16
      // set to blank
      char_position[array_pos][0] = 0; // set type
      char_position[array_pos][1] = 0; // set position
      // find note
      for (int j=0; j < 4; j++) { 
        if (song_blueprint[i-16][j] == 1) { // note found
            char_position[array_pos][0] = 1; // set type
            char_position[array_pos][1] = j; // set position
        }
      }
      
      // if previous frame was exploding
      if (char_position2[array_pos+1][0] == 2){
        char_position[array_pos][0] = 2; // set type
      }
    } else {
      char_position[array_pos][0] = 0; // set type
      char_position[array_pos][1] = 0; // set position
    }
  }

  // if gamepad buttons pressed
  for (int i=0; i < 4; i++) {
    if (getBControl(i) == 1) { // if pressed
      for (int j=0; j < (barier_height+1); j++) { // loop through notes within barier
        if (char_position[j][0] == 1){ // note found
          if (char_position[j][1] == i) { // if note is in relative position
            char_position[j][0] = 2; // set type to explode
          }
          break;
        }
      }
    }
  }

  // If explosion, play sound
  if (char_position[0][0] == 2) { // explosion found
    playNote(char_position[0][1]); // plays respective audio
  }
  
  // end of loop
  drawFrame();
  song_position++; // increment song position
  memcpy(char_position2, char_position, sizeof(char_position)); // copy char_position to char_position2

  time = time + song_speed;
  delay(song_speed);

  // If note touched bottom of lcd, game over
  if (char_position[0][0] == 1) { // note found
    if (game_over == 1) { // game over on
      song_position = 0;
      gameoverScreen();
    }
  }
};


void startScreen(int stop) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("~ Piano Tiles ~");
  lcd.setCursor(0,1);
  lcd.print("  Press Start");
  if (stop == 0) {
    playNote(4);
  } else { // pressed to stop game
    playNote(5);
  }
  while (getBStart() == 0) {
    delay(10);
  }
  time = 0; // reset time score
};

void gameoverScreen(){
  Serial.println("Game Over\n");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" ~ Game Over ~");
  lcd.setCursor(0,1);
  lcd.print("   Time: ");
  lcd.print(time/1000);
  playNote(5);
  while (getBStart() == 0) {
    delay(10);
  }
  startScreen(0);
};

void playNote(int note) {
  // 4 main notes
  if (note == 0){ // F
    tone(BuzzerPIN, notes_frequency[0] * pitch, 250);
  } else if (note == 1){ // G
    tone(BuzzerPIN, notes_frequency[1] * pitch, 250);
  } else if (note == 2){ // A
    tone(BuzzerPIN, notes_frequency[2] * pitch, 250);
  } else if (note == 3){ // B
    tone(BuzzerPIN, notes_frequency[3] * pitch, 250);
  }
  // start game tune
  if (note == 4){
    tone(BuzzerPIN, 200 * pitch, 200);
    delay(250);
    tone(BuzzerPIN, 300 * pitch, 250);
    delay(250);
    tone(BuzzerPIN, 400 * pitch, 250);
    delay(250);
    tone(BuzzerPIN, 500 * pitch, 250);
  }
  // game over tune
  if (note == 5){
    tone(BuzzerPIN, 500 * pitch, 250);
    delay(250);
    tone(BuzzerPIN, 400 * pitch, 250);
    delay(250);
    tone(BuzzerPIN, 300 * pitch, 250);
    delay(250);
    tone(BuzzerPIN, 200 * pitch, 200);
  }
};

void drawFrame() { // draw frame using int char_position
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      int type = char_position[j][0];
      int pos = char_position[j][1];

      if (j == barier_height) { // is barier
        if (type == 0) { // blank
          char_string[j] = barier;
        } else if (type == 1) { // note
          char_string[j] = barierNnote[pos][i];
        } else if (type == 2) { // explode
          char_string[j] = barierNnoteExploding[pos][i];
        }
      } else { // not barier
        if (type == 0) { // blank
          char_string[j] = blankchar;
        } else if (type == 1) { // note
          char_string[j] = note[pos][i];
        } else if (type == 2) { // explode
          char_string[j] = noteExploding[pos][i];
        }
      }
    }

    lcd.setCursor(0, i);
    lcd.print(char_string);
  }
};


// get button presses
int readPotentio() { // 0 - 1023
  return analogRead(PotentioPin);
};

int getBControl(int button) {
  if (digitalRead(ButtonPin_MainControls[button]) == 1) {
    return 1;
  }
  return 0;
}

int getBStart() {
  if (digitalRead(ButtonStart) == 1) {
    return 1;
  }
  return 0;
}

int getBMatrix() { // BMatrixC1
  int pressed_BMatrix = 0;
  for(int c = 0; c < 3; c++){
    for(int r = 0; r < 2; r++){
      digitalWrite(ButtonPinMatrix_Colums[c],0);
      if(digitalRead(ButtonPinMatrix_Rows[r]) == 0) {
        pressed_BMatrix = MatrixButtonMapping[r][c]; // pressed matrix button
      }
      digitalWrite(ButtonPinMatrix_Colums[c],1);
    }
  }
  return pressed_BMatrix;
}
