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

//#define DBG

#include <LiquidCrystal.h>
#include <EEPROM.h>

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


#define int8 byte
#define int16 word
#define int32 unsigned long
double k = 0.0277016860218373*10;
int32 km;

int v;

#define MIN_KM 2348954 // value km *10

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);


struct filter {
  
  signed long value,gain;

};


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

long imp_count=0;
long speedt,speedo;
int save=0;
void speed() {
  
 
  speedt++;
  imp_count++;

  if (speedt > 5000) {
    float t = (float)speedt*k;
   // Serial.print(t);
    km += (int)t/100;
    speedt=0;
    save = 1;
    //io_odo(0);
  }
   
}

int timer1_counter;
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  speedo = speedt;
  //speedt=0;
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
void io_odo(char c);

int32 trip_start = 0;

// 129595 imp / 3590m
// 0,0277016860218373 m/imp

int error = 0;

void writeKm(int16 base,int32 val) {
  int8 c1,c2,c3,cs;
  c1=(int32)((int32)val>>16)&0xFF;
  c2=(int32)((int32)val>>8)&0xFF;
  c3=(int32)val&0xFF;
  
  cs = ~(c1+c2+c3)+1;   

#ifdef DBG
  Serial.print("WR: ");

  Serial.print(c1,HEX);Serial.print(" ");
  Serial.print(c2,HEX);Serial.print(" ");
  Serial.print(c3,HEX);Serial.print(" ");
  Serial.print(cs,HEX);Serial.print(" \n");

#endif

 EEPROM.write(base,~c1);
 EEPROM.write(base+1,~c2);
 EEPROM.write(base+2,~c3); 
 EEPROM.write(base+3,~cs);      
}

long readKm(int16 base) {
  int8 c1,c2,c3,cs;
  c1= ~EEPROM.read(base);
  c2= ~EEPROM.read(base+1);
  c3= ~EEPROM.read(base+2);        
  cs= ~EEPROM.read(base+3); 
  cs+=c1+c2+c3;

#ifdef DBG
  Serial.print("RD: ");

  Serial.print(c1,HEX);Serial.print(" ");
  Serial.print(c2,HEX);Serial.print(" ");
  Serial.print(c3,HEX);Serial.print(" ");
  Serial.print(cs,HEX);Serial.print(" \n");

#endif
  if (!cs) {
    error = 1;
  }
  
  return ((int32)c1<<16)+((int32)c2<<8)+(int32)c3; 
}

int32 readTrip() {
  error = 0;
  trip_start = readKm(0x128)*10;

#ifdef DBG

  Serial.print("TRIPST:");
  Serial.print(trip_start);
  Serial.print(" ERR ");
  Serial.print(error);
  Serial.print("\n");

#endif  

  if (error) {
  //  trip_start = km;
  }
}

void resetTrip() {
  noInterrupts();
  trip_start = km;
  writeKm(0x128,trip_start/10);
  interrupts();


#ifdef DBG

  Serial.print("TRIPST:");
  Serial.print(trip_start);
  Serial.print(" ERR ");
  Serial.print(readKm(0x128));
  Serial.print("\n");

#endif  
  
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(2), speed, CHANGE);
  Serial.begin(9600); // Default baudrate.
  lcd.begin(16, 2); // Change this for other screen sizes.

  delay(1000);
  Serial.print("Debug");

  setup_timer();

  analogWrite(BR, 0); // Set maximum brightness.

  pinMode(8,INPUT_PULLUP);
  filter_init(&fuel,1700);
  fuel.value = (long)analogRead(0)<<7;

  noInterrupts();
  io_odo(1);
  interrupts();

  lcd.print("Trip");
  lcd.setCursor(0, 1);
  lcd.print("ODO");

  error = 0;
  readTrip();

  writeKm(0xFF,123456);
  Serial.print("KM ");Serial.print(readKm(0xFF));
  
}

void printKm() {
   lcd.print((long)km/100);lcd.print(".");lcd.print((long)(km%100)/10);
}

void printTrip() {
  signed long trip=km-trip_start;

  if (trip < 0) {
    trip=0;
  }

#ifdef DBG

Serial.print("TRIP: ");
Serial.print(trip);
Serial.print(" KM:");
Serial.print(km);
Serial.print(" ST:");
Serial.print(trip_start);
Serial.print(" KM:");
Serial.print(readKm(0x128));
Serial.print("\n");

#endif
  lcd.setCursor(10, 0);
  int t = trip/100;
  Serial.print(t);
  lcd.print(t);lcd.print(".");lcd.print((long)(trip%100)/10);

#ifdef DBG

  double v = (double)speedt * k/100;
  lcd.setCursor(0, 0);
  lcd.print("        ");
  lcd.setCursor(0, 0);
  lcd.print(imp_count);lcd.print(" ");lcd.print(speedt);

#endif 
}

//long adc = filter_do(&fuel,v);
//adc *= 500;
//adc >>= 10;
//adc = 500 - adc;
//lcd.print(adc/ 100);lcd.print(".");lcd.print(adc%100); volume
  

void loop() {
 // io_odo(1);

  if (save) {
    io_odo(0);
    save=0;
  }
 
  lcd.setCursor(10, 1);
  lcd.print("     ");
 // v = analogRead(0);
  lcd.setCursor(7, 1);
  
  printKm();

  delay(80);

  lcd.setCursor(10, 0);
  lcd.print("          ");
  lcd.setCursor(10, 0);
  
  printTrip();
  
  if (!digitalRead(8)) { // trip reset was hit
   // lcd.setCursor(0, 0);
   // lcd.print("                        ");
   resetTrip();
  }

  
}



void io_odo(char Rw) {
  
  int8 i;
  signed int max;
  int16 base;
 
  int32 cnt1,cnt2;
  
   cnt2=0;
   max=-1;
  
  for (i=0;i<16;i++) {
        base = i*4;
        error = 0;
        cnt1=readKm(base);      
 
        Serial.print(i);Serial.print(" ");Serial.println(cnt1);       
        
        if (error) {

                if (cnt1>=cnt2) {
                        cnt2=cnt1;
                        max=i;
                };
        };        
        
  }; 
  if (Rw) {
    if ((max>=0)) {  
      km = cnt2 * 10;
    } else  {
      km= MIN_KM * 10;
    }
    if ((cnt2 < MIN_KM) || (cnt2 > 9999990)) {
       km= MIN_KM * 10; // otherwise write default value
    }
    
  } else {          
        if (cnt2>km) km=cnt2;
        //write operation  
        max++;
        if (max>15) max=0;
        if (max<0) max=0;
        base = max*4;
        writeKm(base,km/10); 
  };
  
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


