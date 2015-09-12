/*

 The circuit (make sure to power the LCD):
 * LCD RS pin to digital pin 12
 * LCD E pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD Ve pin (contrast) to digital pin 6
 
 */

#include <LiquidCrystal.h>

// You can define your own pins here.
#define RS 12
#define E  11
#define D4 5
#define D5 4
#define D6 3
#define D7 9
#define BR 6

#define VERSION 1

#define GPO1 8 // Not yet implemented. (pg. 18 of the manual)

int v;

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);


struct filter {
  
  signed long value,gain;

} ;


void filter_init(struct filter *f,signed long gain) {

  f->value=0;
  f->gain=gain;

}

signed long filter_do(struct filter *f,signed long data) {

  signed long error;
  error = - (f->value>>7) + data;
  f->value += error;
  return f->value>>7;
}


int speedt,speedo;
void speed() {
  
 
  speedt++;
   
}

int timer1_counter;
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  speedo = speedt;
  speedt=0;
  timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
}

void setup_timer(){
    // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64886;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

struct filter fuel;

void setup() {
  attachInterrupt(digitalPinToInterrupt(2), speed, CHANGE);
  Serial.begin(19200); // Default baudrate.
  lcd.begin(16, 2); // Change this for other screen sizes.

  setup_timer();

  analogWrite(BR, 0); // Set maximum brightness.

  pinMode(8,INPUT_PULLUP);
  filter_init(&fuel,1700);
  fuel.value = (long)analogRead(0)<<7;

  lcd.print("Speed");
  lcd.setCursor(0, 1);
  lcd.print("Fuel");
}



void loop() {
  lcd.setCursor(10, 1);
  lcd.print("     ");
  v = analogRead(0);
  lcd.setCursor(10, 1);
  long adc = filter_do(&fuel,v);
  adc *= 500;
  adc >>= 10;
  lcd.print(adc/ 100);lcd.print(".");lcd.print(adc%100);
  delay(80);

   lcd.setCursor(10, 0);
  lcd.print("     ");
  lcd.setCursor(10, 0);
  lcd.print(speedo*10/55);

  if (!digitalRead(8)) {
    lcd.setCursor(0, 0);
    lcd.print("                        ");
  }

  
}

void countTime() {
  
}

/*
 * Waits for a byte to be available, reads and returns it.
 */
byte serial_getch() {
  while (Serial.available() == 0);

  return Serial.read();
}


