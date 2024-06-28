#ifndef Arduino_PianoTiles_CAB202 
#define Arduino_PianoTiles_CAB202 

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <LiquidCrystal.h>


#define SET_BIT(reg, pin)           (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)         (reg) &= ~(1 << (pin))
#define BIT_VALUE(reg, pin)         (((reg) >> (pin)) & 1)

// Uart definitions
#define BAUD (9600)
#define MYUBRR (F_CPU/16/BAUD-1)

// These buffers may be any size from 2 to 256 bytes.
#define  RX_BUFFER_SIZE  64
#define  TX_BUFFER_SIZE  64

// Uart definitions
unsigned char rx_buf;
static volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_head;
static volatile uint8_t tx_buffer_tail;
static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head;
static volatile uint8_t rx_buffer_tail;

// Uart functions
void uart_init(unsigned int ubrr);
void uart_putchar(uint8_t c);
void uart_putstring(unsigned char* s);

//////////////////////////////////////////////////////////////////////////////////////////

// Setup pins
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13; // LCD pins
// Matrix buttons
#define BMatrixC1 PC0
#define BMatrixC1_R PINC
#define BMatrixC1_R2 DDRC
#define BMatrixC1_R3 PORTC
#define BMatrixC2 PC1
#define BMatrixC2_R PINC
#define BMatrixC2_R2 DDRC
#define BMatrixC2_R3 PORTC
#define BMatrixC3 PC2
#define BMatrixC3_R PINC
#define BMatrixC3_R2 DDRC
#define BMatrixC3_R3 PORTC
#define BMatrixR1 PC5
#define BMatrixR1_R PINC
#define BMatrixR1_R2 DDRC
#define BMatrixR1_R3 PORTC
#define BMatrixR2 PD7
#define BMatrixR2_R PIND
#define BMatrixR2_R2 DDRD
#define BMatrixR2_R3 PORTD
// Main controls (for hitting the falling notes)
#define BControl1_R PIND
#define BControl1 PD2
#define BControl2_R PIND
#define BControl2 PD3
#define BControl3_R PIND
#define BControl3 PD4
#define BControl4_R PIND
#define BControl4 PD5
// LED (Blink when recive button press)
#define BLED PC3
#define BLED_R PORTC
#define BLED_R2 DDRC
// Potentiometer (for pitch)
#define BPot 4
// BuzzerPin (for sound)
#define BuzzerPin PD6
#define BuzzerPin_R DDRD

// Matrix Buttons Mapping
int BMatrixMapping[2][3] = {
  {1, 3, 5},
  {2, 4, 6}
};

// Custom characters gylphs
const int numGLYPHS = 7;
uint8_t glyphs[numGLYPHS][8] = {
  // barier
   {0B00001,
    0B00001,
    0B00001,
    0B00001,
    0B00001,
    0B00001,
    0B00001,
    0B00001}
  // note  [left]
  ,{0B11000,
    0B11111,
    0B00010,
    0B01100,
    0B00000,
    0B00000,
    0B00000,
    0B00000}
  // note  [right]
  ,{0B00000,
    0B00000,
    0B00000,
    0B00000,
    0B11000,
    0B11111,
    0B00010,
    0B01100}
  // barier + note  [left]
  ,{0B11001,
    0B11111,
    0B00011,
    0B01101,
    0B00001,
    0B00001,
    0B00001,
    0B00001}
  // barier + note  [right]
  ,{0B00001,
    0B00001,
    0B00001,
    0B00001,
    0B11001,
    0B11111,
    0B00011,
    0B01101}
  // note exploding  [left]
  ,{0B10101,
    0B01110,
    0B01110,
    0B10101,
    0B00000,
    0B00000,
    0B00000,
    0B00000}
  // note exploding  [right]
  ,{0B00000,
    0B00000,
    0B00000,
    0B00000,
    0B10101,
    0B01110,
    0B01110,
    0B10101}
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
  // F
   174.61
  // G
  , 196.00
  // A
  , 220.00
  // B
  , 246.94
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
int pitch = 1; 
uint16_t pot_value = 0; // min = 0, max = 1023

// Your score
int time = 0;

// Serial monitor | simulate gamepad buttons
int serial_input = 0; // 0 = none, 1 = BC1, 2 = BC2, 3 = BC3, 4 = BC4

// Setup LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Timer dependant
int timer2_counter = 0;
int timer2_counter2 = 0;
int playFreq_duration = 0;
int LED_duration = 0;

// Declarate functions
void loop();

void startScreen(int stop);
void gameoverScreen();
void playTune(int note);
void drawFrame();

int readPotentio();
int getBControl(int button);
int getBStart();
int getBMatrix();
int BMatrixRead(int row);
void BMatrixWrite(int col, int state);

void playFreq(int freq, int division_factor, int duration);

int main(){
  uart_init(MYUBRR);
  lcd.begin(16, 2);
  // create custom characters for the LCD
  for (int i=0; i < numGLYPHS; i++) {
    lcd.createChar(i + 1, glyphs[i]);
  }

  // Timer setup
  TCCR2A = 0;
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); 
  TIMSK2 = 1; 

  // potentiometer setup
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADMUX = (1 << REFS0) | BPot;

  // Switch matrix setup
  SET_BIT(BMatrixC1_R2, BMatrixC1); // Set Collums to output
  SET_BIT(BMatrixC2_R2, BMatrixC2);
  SET_BIT(BMatrixC3_R2, BMatrixC3);
  CLEAR_BIT(BMatrixR1_R2, BMatrixR1); // Set Rows to input
  CLEAR_BIT(BMatrixR2_R2, BMatrixR2);
  for(int c = 0; c < 3; c++){ // set initial state
      BMatrixWrite((c+1), 1);
    }
  
  // piezo setup
  SET_BIT(BuzzerPin_R, BuzzerPin);

  // LED setup
  SET_BIT(BLED_R2, BLED);

  startScreen(0);
  while (1) {
     loop();
  }
};

void loop() {
  // if matrix buttons pressed
  int pressed_BMatrix = getBMatrix(); // 0 = not pressed
  
  // if speed button pressed
  if (pressed_BMatrix == 1) { // faster speed
    song_speed = song_speed - 10; // reduce loop delay
    uart_putstring((unsigned char *) "Increased speed\n");
  }
  if (pressed_BMatrix == 2) { // slower speed
    song_speed = song_speed + 10; // increase loop delay
    uart_putstring((unsigned char *) "Decreased speed\n");
  }

  // if barier button pressed
  if (pressed_BMatrix == 3) { // increase barier height
    barier_height = barier_height + 1;
    if (barier_height > 15) { // max height
      barier_height = 15;
    }
    uart_putstring((unsigned char *) "Barier elevated\n");
  }
  if (pressed_BMatrix == 4) { // decrease barier height
    barier_height = barier_height - 1;
    if (barier_height < 0) { // min height
      barier_height = 0;
    }
    uart_putstring((unsigned char *) "Barier lowered\n");
  }

  // if reset button pressed
  if (pressed_BMatrix == 5) { // reset settings variables
    song_speed = 300;
    barier_height = 6;
    // pitch = 1;
    // game_over = 1;
    uart_putstring((unsigned char *) "Settings reset\n");
  }

  // if toggle game over button pressed
  if (pressed_BMatrix == 6) { // toggle game over
    if (game_over == 1) { // game over on
      game_over = 0;
    } else { // game over off
      game_over = 1;
    }
    uart_putstring((unsigned char *) "Toggled: Game over\n");
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
    playTune(char_position[0][1]); // plays respective audio
  }
  
  // end of loop
  drawFrame();
  song_position++; // increment song position
  memcpy(char_position2, char_position, sizeof(char_position)); // copy char_position to char_position2
  
  // reset serial_input
  serial_input = 0;

  // update score
  time = time + song_speed;

  // loop delay
  for (int i = 0; i < ((song_speed)/10); i++) {
      _delay_ms(10);
  }
  
  // If note touched bottom of lcd, game over
  if (char_position[0][0] == 1) { // note found
    if (game_over == 1) { // game over on
      song_position = 0;
      gameoverScreen();
    }
  }
};


// Game functionality
void startScreen(int stop) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("~ Piano Tiles ~");
  lcd.setCursor(0,1);
  lcd.print("  Press Start");
  if (stop == 0) {
    playTune(4);
  } else { // pressed to stop game
    playTune(5);
  }
  while (getBStart() == 0) {
    _delay_ms(10);
  }
  time = 0; // reset time score
};

void gameoverScreen(){
  uart_putstring((unsigned char *) "Game Over\n");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" ~ Game Over ~");
  lcd.setCursor(0,1);
  lcd.print("   Time: ");
  char time_string[5];
  itoa(time/1000, time_string, 10);
  lcd.print(time_string);
  playTune(5);
  while (getBStart() == 0) {
    _delay_ms(10);
  }
  startScreen(0);
};

void playTune(int note) {
  // 4 main notes
  if (note == 0){ // F
    playFreq(notes_frequency[0] * pitch, 256, 250);
  } else if (note == 1){ // G
    playFreq(notes_frequency[1] * pitch, 256, 250);
  } else if (note == 2){ // A
    playFreq(notes_frequency[2] * pitch, 256, 250);
  } else if (note == 3){ // B
    playFreq(notes_frequency[3] * pitch, 256, 250);
  }
  // start game tune
  if (note == 4){
    playFreq(200 * pitch, 256, 200);
    _delay_ms(250);
    playFreq(300 * pitch, 256, 250);
    _delay_ms(250);
    playFreq(400 * pitch, 256, 250);
    _delay_ms(250);
    playFreq(500 * pitch, 256, 250);
  }
  // game over tune
  if (note == 5){
    playFreq(500 * pitch, 256, 250);
    _delay_ms(250);
    playFreq(400 * pitch, 256, 250);
    _delay_ms(250);
    playFreq(300 * pitch, 256, 250);
    _delay_ms(250);
    playFreq(200 * pitch, 256, 200);
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
  char temp_buf[64];

  // Start single conversion by setting ADSC bit in ADCSRA
  ADCSRA |= (1 << ADSC);

  // Wait for ADSC bit to clear, signalling conversion complete.
  while ( ADCSRA & (1 << ADSC) ) {}

  // Result now available in ADC
  uint16_t pot = ADC;

  // convert uint16_t to string
  itoa(pot, (char *)temp_buf, 10);

  // if pot value changed, send to serial
  if (pot != pot_value) {
    pot_value = pot;
    uart_putstring((unsigned char *) "Pot: ");
    uart_putstring((unsigned char *) temp_buf);
    uart_putchar('\n');
    SET_BIT(BLED_R, BLED);
    timer2_counter2 = 0;
    LED_duration = 250;
  }

  return pot;
};

int getBControl(int button) {
  int return_value = 0;
  if (button == 0){
    if (BIT_VALUE(BControl1_R, BControl1) == 1 || serial_input == 1) {
      return_value = 1;
    }
  } else if (button == 1){
    if (BIT_VALUE(BControl2_R, BControl2) == 1 || serial_input == 2) {
      return_value = 1;
    }
  } else if (button == 2){
    if (BIT_VALUE(BControl3_R, BControl3) == 1 || serial_input == 3) {
      return_value = 1;
    }
  } else if (button == 3){
    if (BIT_VALUE(BControl4_R, BControl4) == 1 || serial_input == 4) {
      return_value = 1;
    }
  }
  if (return_value == 1) {
    uart_putstring((unsigned char *) "Pressed: gamepad button ");
    uart_putchar((button+1)+48);
    uart_putchar('\n');
    SET_BIT(BLED_R, BLED);
    timer2_counter2 = 0;
    LED_duration = 250;
  }
  return return_value;
}

int getBStart() {
  for (int i=0; i < 4; i++) {
    if (getBControl(i) == 1) { // if pressed
      return 1;
    }
  }
  return 0;
}

int getBMatrix() { // BMatrixC1
  int pressed_BMatrix = 0;
  for(int c = 0; c < 3; c++){
    for(int r = 0; r < 2; r++){
      BMatrixWrite((c+1), 0);
      if(BMatrixRead((r+1)) == 0){
        pressed_BMatrix = BMatrixMapping[r][c]; // pressed matrix button
      }
      BMatrixWrite((c+1), 1);
    }
  }
  if (pressed_BMatrix != 0) {
    SET_BIT(BLED_R, BLED);
    timer2_counter2 = 0;
    LED_duration = 250;
  }
  return pressed_BMatrix;
}

int BMatrixRead(int row) {
  int state = 0;
  if (row == 1) {
    state = BIT_VALUE(BMatrixR1_R, BMatrixR1);
  } else if (row == 2) {
    state = BIT_VALUE(BMatrixR2_R, BMatrixR2);
  }
  return state;
}

void BMatrixWrite(int col, int state) {
  if (col == 1) {
    if (state == 0) {
      CLEAR_BIT(BMatrixC1_R3, BMatrixC1);
    } else if (state == 1) {
      SET_BIT(BMatrixC1_R3, BMatrixC1);
    }
  } else if (col == 2) {
    if (state == 0) {
      CLEAR_BIT(BMatrixC2_R3, BMatrixC2);
    } else if (state == 1) {
      SET_BIT(BMatrixC2_R3, BMatrixC2);
    }
  } else if (col == 3) {
    if (state == 0) {
      CLEAR_BIT(BMatrixC3_R3, BMatrixC3);
    } else if (state == 1) {
      SET_BIT(BMatrixC3_R3, BMatrixC3);
    }
  }
}

// play frequency to speaker
void playFreq(int freq, int division_factor, int duration) {
    int ocr = F_CPU / freq / 2;

    TCCR0A = (1<<COM0A1)|(0<<COM0A0);
    if (division_factor == 1) {
      TCCR0B= (0<<FOC0A)|(0<<FOC0B)|(0<<WGM02)| (0<<CS02) | (0<<CS01) | (1<<CS00);
    } else if (division_factor == 8) {
      TCCR0B= (0<<FOC0A)|(0<<FOC0B)|(0<<CS02) | (1<<CS01) | (0<<CS00);
    }  else if (division_factor == 64) {
      TCCR0B= (0<<FOC0A)|(0<<FOC0B)| (0<<CS02) | (1<<CS01) | (1<<CS00);
    } else if (division_factor == 256) {
      TCCR0B= (0<<FOC0A)|(0<<FOC0B)|(1<<CS02) | (0<<CS01) | (0<<CS00);
    } else if (division_factor == 1024) {
      TCCR0B= (0<<FOC0A)|(0<<FOC0B)| (1<<CS02) | (0<<CS01) | (1<<CS00);
    }
    TCCR0A |= (1<<WGM01) | (1<<WGM00);
    TCCR0B &= ~(1<<WGM02);

    OCR0A = ocr;
    playFreq_duration = duration;
    timer2_counter = 0; // reset timer counter
};



//  |||||||||||||| Serial Uart Definitions ||||||||||||||  

// Initialize the UART
void uart_init(unsigned int ubrr) {
  cli();

	UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)(ubrr);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
  tx_buffer_head = tx_buffer_tail = 0;
	rx_buffer_head = rx_buffer_tail = 0;

	sei();
}

// Transmit a byte
void uart_putchar(uint8_t c) {
	uint8_t i;

	i = tx_buffer_head + 1;
	if ( i >= TX_BUFFER_SIZE ) i = 0;
	while ( tx_buffer_tail == i ); // wait until space in buffer
	//cli();
	tx_buffer[i] = c;
	tx_buffer_head = i;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << UDRIE0);
	//sei();
}

// Transmit a string
void uart_putstring(unsigned char* s){
  // transmit character until NULL is reached
  while(*s > 0) uart_putchar(*s++);
}

// Transmit Interrupt
ISR(USART_UDRE_vect) {
	uint8_t i;

	if ( tx_buffer_head == tx_buffer_tail ) {
		// buffer is empty, disable transmit interrupt
		UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	}
	else {
		i = tx_buffer_tail + 1;
		if ( i >= TX_BUFFER_SIZE ) i = 0;
		UDR0 = tx_buffer[i];
		tx_buffer_tail = i;
	}
}

// Receive Interrupt
ISR(USART_RX_vect) {
	uint8_t c, i;

	c = UDR0;
	i = rx_buffer_head + 1;
	if ( i >= RX_BUFFER_SIZE ) i = 0;
	if ( i != rx_buffer_tail ) {
		rx_buffer[i] = c;
		rx_buffer_head = i;
	}

  // set serial_input to c if its 1 || 2 || 3 || 4
  if (c == '1') {
    serial_input = 1;
    uart_putstring((unsigned char *) "Sent: gamepad button 1\n");
  } else if (c == '2') {
    serial_input = 2;
    uart_putstring((unsigned char *) "Sent: gamepad button 2\n");
  } else if (c == '3') {
    serial_input = 3;
    uart_putstring((unsigned char *) "Sent: gamepad button 3\n");
  } else if (c == '4') {
    serial_input = 4;
    uart_putstring((unsigned char *) "Sent: gamepad button 4\n");
  }
}

// Timer0 Interrupt
ISR(TIMER2_OVF_vect) {
  // For when to stop playing a frequency
  timer2_counter++;
  int time_elapsed = 1000.0 * ( timer2_counter * 256.0 + TCNT2 ) * 1024.0  / F_CPU;
  if (time_elapsed > playFreq_duration) {
    OCR0A = 0;
  }
  // For when to turn off LED
  timer2_counter2++;
  time_elapsed = 1000.0 * ( timer2_counter2 * 256.0 + TCNT2 ) * 1024.0  / F_CPU;
  if (time_elapsed > LED_duration) {
    LED_duration = 0;
    CLEAR_BIT(BLED_R, BLED);
  }
}

#endif
