#include <stdint.h>
#include <stdio.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_BIT(reg, pin)		    (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)		  (reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value)   (reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin)		  (((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin)	     (BIT_VALUE((reg),(pin))==1)

#define LCD_USING_4PIN_MODE (1)

#define LCD_DATA4_DDR (DDRD)
#define LCD_DATA5_DDR (DDRD)
#define LCD_DATA6_DDR (DDRD)
#define LCD_DATA7_DDR (DDRD)

#define LCD_DATA4_PORT (PORTD)
#define LCD_DATA5_PORT (PORTD)
#define LCD_DATA6_PORT (PORTD)
#define LCD_DATA7_PORT (PORTD)

#define LCD_DATA4_PIN (4)
#define LCD_DATA5_PIN (5)
#define LCD_DATA6_PIN (6)
#define LCD_DATA7_PIN (7)

#define LCD_RS_DDR (DDRB)
#define LCD_ENABLE_DDR (DDRB)

#define LCD_RS_PORT (PORTB)
#define LCD_ENABLE_PORT (PORTB)

#define LCD_RS_PIN (1)
#define LCD_ENABLE_PIN (0)

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00


void lcd_init(void);
void lcd_write_string(uint8_t x, uint8_t y, char string[]);
void lcd_write_char(uint8_t x, uint8_t y, char val);


void lcd_clear(void);
void lcd_home(void);

void lcd_createChar(uint8_t, uint8_t[]);
void lcd_setCursor(uint8_t, uint8_t); 

void lcd_noDisplay(void);
void lcd_display(void);
void lcd_noBlink(void);
void lcd_blink(void);
void lcd_noCursor(void);
void lcd_cursor(void);
void lcd_autoscroll(void);
void lcd_noAutoscroll(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);

size_t lcd_write(uint8_t);
void lcd_command(uint8_t);


void lcd_send(uint8_t, uint8_t);
void lcd_write4bits(uint8_t);
void lcd_pulseEnable(void);

uint8_t _lcd_displayfunction;
uint8_t _lcd_displaycontrol;
uint8_t _lcd_displaymode;

// END Definitions

void setup_lcd(void);
void loop(void);


void setup_lcd(void) {
  // set up the LCD in 4-pin or 8-pin mode
  //lcd_clear();
  lcd_init();


  // Print a message to the LCD 
  lcd_write_string(0, 0, "Welcome to the");
  lcd_write_string(0, 1, "Elevator!");
  _delay_ms(3000);
  
	lcd_clear();
 	lcd_write_string(0, 0, "Click on button1");
  	lcd_write_string(0, 1, "to start!"); 


  lcd_blink();

}

void lcd_init(void){
  //dotsize
  lcd_clear();
  if (LCD_USING_4PIN_MODE){
    _lcd_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  } else {
    _lcd_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  }
  
  _lcd_displayfunction |= LCD_2LINE;

  // RS Pin
  LCD_RS_DDR |= (1 << LCD_RS_PIN);
  // Enable Pin
  LCD_ENABLE_DDR |= (1 << LCD_ENABLE_PIN);
  
  #if LCD_USING_4PIN_MODE
    //Set DDR for all the data pins
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);    
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);

    //Set DDR for all the data pins
  
  #endif 
  _delay_us(50000); 
  // Now we pull both RS and Enable low to begin commands (R/W is wired to ground)
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  
  //put the LCD into 4 bit or 8 bit mode
    if (LCD_USING_4PIN_MODE) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms

    // second try
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms
    
    // third go!
    lcd_write4bits(0b0111); 
    _delay_us(150);

    // finally, set to 4-bit interface
    lcd_write4bits(0b0010); 
  } else {

    // Send function set command sequence
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(150);

    // third go
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
  }

  // finally, set # lines, font size, etc.
  lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);  

  // turn the display on with no cursor or blinking default
  _lcd_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  lcd_display();

  // clear it off
  lcd_clear();

}

void lcd_write_string(uint8_t x, uint8_t y, char string[]){
  lcd_setCursor(x,y);
  for(int i=0; string[i]!='\0'; ++i){
    lcd_write(string[i]);
  }
}

void lcd_write_char(uint8_t x, uint8_t y, char val){
  lcd_setCursor(x,y);
  lcd_write(val);
}

void lcd_clear(void){
  lcd_command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

void lcd_home(void){
  lcd_command(LCD_RETURNHOME);  // set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}


// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  lcd_command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    lcd_write(charmap[i]);
  }
}


void lcd_setCursor(uint8_t col, uint8_t row){
  if ( row >= 2 ) {
    row = 1;
  }
  
  lcd_command(LCD_SETDDRAMADDR | (col + row*0x40));
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void) {
  _lcd_displaycontrol &= ~LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_display(void) {
  _lcd_displaycontrol |= LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor(void) {
  _lcd_displaycontrol &= ~LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_cursor(void) {
  _lcd_displaycontrol |= LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void) {
  _lcd_displaycontrol &= ~LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_blink(void) {
  _lcd_displaycontrol |= LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
  _lcd_displaymode |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
  _lcd_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}


inline void lcd_command(uint8_t value) {
  //
  lcd_send(value, 0);
}

inline size_t lcd_write(uint8_t value) {
  lcd_send(value, 1);
  return 1; // assume sucess
}


// write either command or data, with automatic 4/8-bit selection
void lcd_send(uint8_t value, uint8_t mode) {
  //RS Pin
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_RS_PORT |= (!!mode << LCD_RS_PIN);
  
  if (LCD_USING_4PIN_MODE) {
    lcd_write4bits(value>>4);
    lcd_write4bits(value);
  } else {
    
  } 
}

void lcd_pulseEnable(void) {
  //Enable Pin
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(1);    
  LCD_ENABLE_PORT |= (1 << LCD_ENABLE_PIN);
  _delay_us(1);    // enable pulse must be >450ns
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(100);   // commands need > 37us to settle
}

void lcd_write4bits(uint8_t value) {
  //Set each wire one at a time

  LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
  LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
  value >>= 1;

  LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
  LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
  value >>= 1;

  LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
  LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
  value >>= 1;

  LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
  LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);

  lcd_pulseEnable();
  
}





void setup() {

	// Set Output B5 and B4
	SET_BIT(DDRC, 1);// level 1
    SET_BIT(DDRC, 2);
  	SET_BIT(DDRC, 3);
  	SET_BIT(DDRC, 4);// level 2
  	SET_BIT(DDRC, 5);
  	SET_BIT(DDRD, 2);
  	SET_BIT(DDRD, 1);// level 3
  
  SET_BIT(DDRB, 5); // clock wise lift
  SET_BIT(DDRB, 4); // anti clock wise lift

	// Input from the left and right buttons
	CLEAR_BIT(DDRD, 3); //(switch 1)
	CLEAR_BIT(DDRB, 2); //(switch 2)
  	CLEAR_BIT(DDRB, 3); //(switch 3)
  	CLEAR_BIT(DDRC,0); //(dc motor enable)
}

//this is the fucntion that is executed every loop iteration and contains 
//the main operation the program is intended to do
void process() {

	// Otherwise, if left button is pressed, turn on GREEN LED.
if ( BIT_IS_SET(PIND, 3) ) {// 1111111111111111
  
  	if( BIT_IS_SET(PORTD, 1) ){
      	SET_BIT(PORTB, 4);// anti clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
      	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 1");
      	CLEAR_BIT(PORTD,1);//CLEAR THE TOP MOST LED LVL 3
      	_delay_ms(200);
      	SET_BIT(PORTD, 2); 
    	_delay_ms(400);
    	CLEAR_BIT(PORTD,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 5);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,5);
    	_delay_ms(400);
    	SET_BIT(PORTC, 4);
      	_delay_ms(400);
    	CLEAR_BIT(PORTC,4);
    	_delay_ms(400);
    	SET_BIT(PORTC, 3);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,3);
    	_delay_ms(400);
    	SET_BIT(PORTC, 2);
      	_delay_ms(400);
    	CLEAR_BIT(PORTC,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 1);
      	CLEAR_BIT(PORTC, 0);//disable dc motor
      	CLEAR_BIT(PORTB, 4);//disable rotation
      	lcd_clear();
    	lcd_write_string(0, 0, "You are on LVL 1");
    }
  else if( BIT_IS_SET(PORTC, 4) ){
    	SET_BIT(PORTB, 4);// anti clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
    	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 1");
    	CLEAR_BIT(PORTC,4);// CLEAR THE MIDDLE LED LVL 2
    	_delay_ms(400);
    	SET_BIT(PORTC, 3);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,3);
    	_delay_ms(400);
    	SET_BIT(PORTC, 2);
      	_delay_ms(400);
    	CLEAR_BIT(PORTC,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 1);
    	CLEAR_BIT(PORTC, 0);// disable dc motor
      	CLEAR_BIT(PORTB, 4);// disable rotation
    	lcd_clear();
    	lcd_write_string(0, 0, "You are on LVL 1");
  }else {
    	SET_BIT(PORTC, 1);
    lcd_clear();
    lcd_write_string(0, 0, "You are on LVL 1");
        }
  
}//11111111111111
  
  
if ( BIT_IS_SET(PINB, 2) ) { //222222222222
  
    if( BIT_IS_SET(PORTC, 1) ){
      	SET_BIT(PORTB, 5);// clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
      	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 2");
      	CLEAR_BIT(PORTC,1); //CLEAR THE BOTTOM MOST LED LVL 1
      	_delay_ms(200);
      	SET_BIT(PORTC, 2); 
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 3);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,3);
    	_delay_ms(400);
    	SET_BIT(PORTC, 4);
      	CLEAR_BIT(PORTC, 0);// disable dc motor
      	CLEAR_BIT(PORTB, 5);// disable rotation
      	lcd_clear();
 		lcd_write_string(0, 0, "You are on LVL 2");
    }
  else if ( BIT_IS_SET(PORTD, 1) ){
    	SET_BIT(PORTB, 4);// anti clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
    	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 2");
  		CLEAR_BIT(PORTD,1);//CLEAR THE TOP MOST LED LVL 3 
      	_delay_ms(200);
      	SET_BIT(PORTD, 2); 
    	_delay_ms(400);
    	CLEAR_BIT(PORTD,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 5);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,5);
    	_delay_ms(400);
    	SET_BIT(PORTC, 4);
    	CLEAR_BIT(PORTC, 0);// disable dc motor
      	CLEAR_BIT(PORTB, 4);// disable rotation
    	lcd_clear();
 		lcd_write_string(0, 0, "You are on LVL 2");
  }else {
  	//otherwise it doesnt do anything
  }
      
      
}//222222222
  
  
if(  BIT_IS_SET(PINB, 3)  ){ //333333333333
  
    if( BIT_IS_SET(PORTC, 1) ){
      	SET_BIT(PORTB, 5);// clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
      	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 3");
      	CLEAR_BIT(PORTC,1); //CLEAR THE BOTTOM MOST LED LVL 1
      	_delay_ms(200);
      	SET_BIT(PORTC, 2); 
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,2);
    	_delay_ms(400);
    	SET_BIT(PORTC, 3);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,3);
    	_delay_ms(400);
    	SET_BIT(PORTC, 4);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,4);
    	_delay_ms(400);
    	SET_BIT(PORTC, 5);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,5);
    	_delay_ms(400);
    	SET_BIT(PORTD, 2);
    	_delay_ms(400);
    	CLEAR_BIT(PORTD,2);
    	_delay_ms(400);
    	SET_BIT(PORTD, 1);
      	CLEAR_BIT(PORTC, 0);// disable dc motor
      	CLEAR_BIT(PORTB, 5);// disable rotation
      	lcd_clear();
 		lcd_write_string(0, 0, "You are on LVL 3");
  }
  else if (BIT_IS_SET(PORTC, 4))
  {
    	SET_BIT(PORTB, 5);// clockwise
    	SET_BIT(PORTC, 0);// turn on dc motor
    	lcd_clear();
 		lcd_write_string(0, 0, "Going to LVL 3");
    	CLEAR_BIT(PORTC,4);//CLEAR THE MIDDLE LED LVL 2
    	_delay_ms(400);
    	SET_BIT(PORTC, 5);
    	_delay_ms(400);
    	CLEAR_BIT(PORTC,5);
    	_delay_ms(400);
    	SET_BIT(PORTD, 2);
    	_delay_ms(400);
    	CLEAR_BIT(PORTD,2);
    	_delay_ms(400);
    	SET_BIT(PORTD, 1);
    	CLEAR_BIT(PORTC, 0);// disable dc motor
      	CLEAR_BIT(PORTB, 5);// disable rotation
    	lcd_clear();
 		lcd_write_string(0, 0, "You are on LVL 3");
    }else{
    	//otherwise it doesnt do anything
    }
  
}//333333333333
  
  
}//process
int main(void) {

	//setup avr
	setup();
	  setup_lcd();
  while(1){
    process();
   	
  }

	
}
