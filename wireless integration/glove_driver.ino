#include <Wire.h>
#include <MPU6050.h>
#include <ILI9341_t3.h>
#include <font_Arial.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CS 10
#define DC 9
#define RESET 8
#define T_CS 2

// flex sensor 1 - index finger - wrist
#define MIN_ADC_1 380 // no flex
#define MAX_ADC_1 250 // full flex
#define MAX_1 60 // max servo angle

// flex sensor 2 - middle finger - arm
#define MIN_ADC_2 380 // no flex
#define MAX_ADC_2 250 // full flex
#define MAX_2 60 // max servo angle

// flex sensor 3 - thumb - gripper
#define MIN_ADC_3 500 // no flex
#define MAX_ADC_3 320 // full flex
#define MAX_3 75 // max servo angle

// flex sensor 4 - ring finger - base
#define MIN_ADC_4 380 // no flex
#define MAX_ADC_4 250 // full flex
#define MAX_4 30 // max servo angle

// mpu y - roll - rotary
#define CCW_ROLL -16500 // ccw roll is min
#define CW_ROLL 16000 // cw roll is max
#define MAX_Y 30 // max servo angle

MPU6050 mpu;
ILI9341_t3 lcd = ILI9341_t3(CS, DC);
XPT2046_Touchscreen t(T_CS);
TS_Point tp;

const int fp1 = A0; // index finger
const int fp2 = A1; // middle finger
const int fp3 = A2; // thumb
const int fp4 = A3; // ring finger
int16_t ax, ay, az ;
int16_t gx, gy, gz ;

int ang1, ang2, ang3, ang4, nay = 0;

const int RECT_LENGTH = 200; // lcd rectangles are 200 px length
const int readings  = 30;

//Serial
int incomingBit = 0;

int i = 0; // index

int avg_fp1, tot_fp1 = 0;
int fp1_ADC[readings];
int avg_fp2, tot_fp2 = 0;
int fp2_ADC[readings];
int avg_fp3, tot_fp3 = 0;
int fp3_ADC[readings];
int avg_fp4, tot_fp4 = 0;
int fp4_ADC[readings];

int last_fp1_ADC, last_fp2_ADC, last_fp3_ADC, last_fp4_ADC, last_ay = 0;//, last_ax = 0;
int last_ang1, last_ang2, last_ang3, last_ang4, last_nay = 0;//, last_nax = 0;

int k = 0;
int flex1, flex2, flex3, flex4 = 0;

uint16_t x, y; // x and y location of touch input?
int mode = 0; // allowed modes: 0 - setup, 1 - control, 2 - freeze

void setup() {       
  //Serial
  Serial.begin(38400);
  

  
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < readings; thisReading++) {
    fp1_ADC[thisReading] = 0;
    fp2_ADC[thisReading] = 0;
    fp3_ADC[thisReading] = 0;
    fp4_ADC[thisReading] = 0;
  }
  Serial.begin(9600); 
// initialize MPU
  Wire.begin ( );
  mpu.initialize ( ); 
  Serial.println (mpu.testConnection ( ) ? "Successfully Connected" : "Connection failed");  
  
  //initialize_lcd();   
// *********************** initialize lcd *********************************************
  lcd.begin();
  t.begin();
  lcd.setRotation(3);  
  t.setRotation(3);
  lcd.fillScreen(ILI9341_BLACK);
  
  lcd.setTextColor(ILI9341_WHITE);
  lcd.setTextSize(1);

  lcd.setCursor(5,7); lcd.print("Sensor 1: ");
  lcd.setCursor(120,7); lcd.print("Servo 1: ");
  lcd.drawRect(4,18,RECT_LENGTH+3,15,ILI9341_WHITE);
  lcd.setCursor(5,37); lcd.print("Sensor 2: ");
  lcd.setCursor(120,37); lcd.print("Servo 2: ");
  lcd.drawRect(4,48,RECT_LENGTH+3,15,ILI9341_WHITE);
  lcd.setCursor(5,67); lcd.print("Sensor 3: ");
  lcd.setCursor(120,67); lcd.print("Servo 3: ");
  lcd.drawRect(4,78,RECT_LENGTH+3,15,ILI9341_WHITE);
  lcd.setCursor(5,97); lcd.print("Sensor 4: ");
  lcd.setCursor(120,97); lcd.print("Servo 4: ");
  lcd.drawRect(4,108,RECT_LENGTH+3,15,ILI9341_WHITE);
  lcd.setCursor(5,127); lcd.print("Sensor 5: ");
  lcd.setCursor(120,127); lcd.print("Servo 5: ");
  lcd.drawRect(4,138,RECT_LENGTH+3,15,ILI9341_WHITE);

  lcd.setCursor(5,200); lcd.print("Mode: ");
  
  lcd.setTextSize(2);
  lcd.setCursor(5,215); lcd.print("Setup");
  
  lcd.setCursor(235,25); lcd.print("Start");
  lcd.setCursor(240,83); lcd.print("Stop");
  lcd.setCursor(235,141); lcd.print("Reset");
  
  lcd.setTextSize(1);
  
  // button boxes
  // drawRect(x, y, x_length,y_length,color); coordinates are top left corners
  lcd.drawRect(220,8,90,50,ILI9341_WHITE);
  lcd.drawRect(220,66,90,50,ILI9341_WHITE);
  lcd.drawRect(220,124,90,50,ILI9341_WHITE);
  lcd.drawRect(220,182,90,50,ILI9341_WHITE);

// ****************************** end initialize lcd **************************************
  
// initialize flex sensors
  pinMode(fp1, INPUT);
  pinMode(fp2, INPUT);
  pinMode(fp3, INPUT);
  pinMode(fp4, INPUT);

  delay (1000); // delay for no reason?
}

void loop() {
// get touch input
  if (t.touched()) {
    tp = t.getPoint();
    x = map(tp.x,3760,150,0,320);
    y = map(tp.y,3880,200,0,240);
  }
  else {  
      x = 0;
      y = 0;
    }
/*
    Serial.print("x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
*/
    if ((220 <= x && x <= 310) && (8 <= y && y <= 58)) { // start button
      mode = 1;
      lcd.fillRect(5,215,100,30,ILI9341_BLACK);
      lcd.setTextColor(ILI9341_WHITE);
      lcd.setTextSize(2);
      lcd.setCursor(5,215); lcd.print("Control");
    }
    else if ((220 <= x && x <= 310) && (66 <= y && y <= 116)) { // stop button --> freeze mode
      mode = 2;
      lcd.fillRect(5,215,100,30,ILI9341_BLACK);
      lcd.setTextColor(ILI9341_WHITE);
      lcd.setTextSize(2);
      lcd.setCursor(5,215); lcd.print("Freeze");
    }
    else if ((220 <= x && x <= 310) && (124 <= y && y <= 174)) { // reset button
      mode = 0;
      lcd.fillRect(5,215,100,30,ILI9341_BLACK);
      lcd.setTextColor(ILI9341_WHITE);
      lcd.setTextSize(2);
      lcd.setCursor(5,215); lcd.print("Setup");
      
      lcd.setTextSize(1);
      lcd.setTextColor(ILI9341_BLACK);  
      lcd.setCursor(60,7); lcd.print(String(last_fp1_ADC));
      lcd.setCursor(168,7); lcd.print(String(last_ang1));  
      lcd.setCursor(60,37); lcd.print(String(last_fp2_ADC));
      lcd.setCursor(168,37); lcd.print(String(last_ang2));  
      lcd.setCursor(60,67); lcd.print(String(last_fp3_ADC));
      lcd.setCursor(168,67); lcd.print(String(last_ang3));
      lcd.setCursor(60,97); lcd.print(String(last_fp4_ADC));
      lcd.setCursor(168,97); lcd.print(String(last_ang4));
      lcd.setCursor(60,127); lcd.print(String(last_ay));
      lcd.setCursor(168,127); lcd.print(String(last_nay));
          
      lcd.fillRect(5,19,RECT_LENGTH,13,ILI9341_BLACK);
      lcd.fillRect(5,49,RECT_LENGTH,13,ILI9341_BLACK);
      lcd.fillRect(5,79,RECT_LENGTH,13,ILI9341_BLACK);
    }
    else if ((220 <= x && x <= 310) && (182 <= y && y <= 232)) {
      // 
    }
    else {
      // do nothing
    }
        
// mode check
  switch(mode) { 
    case 0: // set up
        // set servo angle variables to 0 over and over
        ang1 = 0;
        ang2 = 0;
        ang3 = 0;
        ang4 = 0;
        delay(100);      
        break;
        
    case 1: // normal operation       
        
      // read input
        mpu.getMotion6 (&ax, &ay, &az, &gx, &gy, &gz);
        
        tot_fp1 = tot_fp1 - fp1_ADC[i];
        fp1_ADC[i] = analogRead(fp1);
        tot_fp1 = tot_fp1 + fp1_ADC[i];
        
        tot_fp2 = tot_fp2 - fp2_ADC[i];
        fp2_ADC[i] = analogRead(fp2);
        tot_fp2 = tot_fp2 + fp2_ADC[i];
        
        tot_fp3 = tot_fp3 - fp3_ADC[i];
        fp3_ADC[i] = analogRead(fp3);
        tot_fp3 = tot_fp3 + fp3_ADC[i];

        tot_fp4 = tot_fp4 - fp4_ADC[i];
        fp4_ADC[i] = analogRead(fp4);
        tot_fp4 = tot_fp4 + fp4_ADC[i];
        
        i = i + 1;
        if (i >= readings)
          i = 0;
          
        avg_fp1 = tot_fp1 / readings;
        avg_fp2 = tot_fp2 / readings;
        avg_fp3 = tot_fp3 / readings;
        avg_fp4 = tot_fp4 / readings;
      
      // map servo angle
        ang1 = map(avg_fp1, MIN_ADC_1, MAX_ADC_1, 0, MAX_1);
        ang2 = map(avg_fp2, MIN_ADC_2, MAX_ADC_2, 0, MAX_2);
        ang3 = map(avg_fp3, MIN_ADC_3, MAX_ADC_3, 0, MAX_3);
        ang4 = map(avg_fp4, MIN_ADC_4, MAX_ADC_4, 0, MAX_4);
        nay = map(ay, CCW_ROLL, CW_ROLL, 0, MAX_Y); // y = rotary
      
      // bound checking
        if (ang1 < 0) ang1 = 0; if (ang1 > MAX_1) ang1 = MAX_1;
        if (ang2 < 0) ang2 = 0; if (ang2 > MAX_2) ang2 = MAX_2;
        if (ang3 < 0) ang3 = 0; if (ang3 > MAX_3) ang3 = MAX_3;
        if (ang4 < 0) ang4 = 0; if (ang4 > MAX_4) ang4 = MAX_4;
        if (nay < 0) nay = 0; if (nay > MAX_Y) nay = MAX_Y;

      // write servo angle
        /*/ set servo angles to 
        sv1.write(ang4); // middle finger
        sv2.write(ang4); // middle finger
        sv3.write(ang2); // index finger
        sv4.write(ang2); // index finger
        sv5.write(ang1); // ring finger
        sv6.write(ang1); // ring finger
        sv7.write(ang3); // thumbl
        //sv8.write(nay); // mpu y
        sv8.write(ang3);
*/
        if(k%40 == 0) {
          lcd.setTextSize(1);
          lcd.setTextColor(ILI9341_BLACK);  
          lcd.setCursor(60,7); lcd.print(String(last_fp1_ADC));
          lcd.setCursor(168,7); lcd.print(String(last_ang1));  
          lcd.setCursor(60,37); lcd.print(String(last_fp2_ADC));
          lcd.setCursor(168,37); lcd.print(String(last_ang2));  
          lcd.setCursor(60,67); lcd.print(String(last_fp3_ADC));
          lcd.setCursor(168,67); lcd.print(String(last_ang3));
          lcd.setCursor(60,97); lcd.print(String(last_fp4_ADC));
          lcd.setCursor(168,97); lcd.print(String(last_ang4));
          lcd.setCursor(60,127); lcd.print(String(last_ay));
          lcd.setCursor(168,127); lcd.print(String(last_nay));
        // *********************************** lcd-output **************************************
          lcd.setTextSize(1);
          // blue
          lcd.setTextColor(ILI9341_BLUE);
          lcd.setCursor(60,7); lcd.print(String(avg_fp1));
          lcd.setCursor(168,7); lcd.print(String(ang1));  
          flex1 = map(ang1,0,MAX_1,0,RECT_LENGTH);
          lcd.fillRect(6,19,RECT_LENGTH,13,ILI9341_BLACK);
          lcd.fillRect(6,19,flex1,13,ILI9341_BLUE);

          // red
          lcd.setTextColor(ILI9341_RED);
          lcd.setCursor(60,37); lcd.print(String(avg_fp2));
          lcd.setCursor(168,37); lcd.print(String(ang2));  
          flex2 = map(ang2,0,MAX_2,0,RECT_LENGTH);
          lcd.fillRect(6,49,RECT_LENGTH,13,ILI9341_BLACK);
          lcd.fillRect(6,49,flex2,13,ILI9341_RED);
        
          // green 
          lcd.setTextColor(ILI9341_GREEN);
          lcd.setCursor(60,67); lcd.print(String(avg_fp3));
          lcd.setCursor(168,67); lcd.print(String(ang3));
          flex3 = map(ang3,0,MAX_3,0,RECT_LENGTH);
          lcd.fillRect(6,79,RECT_LENGTH,13,ILI9341_BLACK);
          lcd.fillRect(6,79,flex3,13,ILI9341_GREEN);
        
          // yellow
          lcd.setTextColor(ILI9341_YELLOW);
          lcd.setCursor(60,97); lcd.print(String(avg_fp4));
          lcd.setCursor(168,97); lcd.print(String(ang4));
          flex4 = map(ang4,0,MAX_4,0,RECT_LENGTH);
          lcd.fillRect(6,109,RECT_LENGTH,13,ILI9341_BLACK);
          lcd.fillRect(6,109,flex4,13,ILI9341_YELLOW);
          
          // orange
          lcd.setTextColor(ILI9341_ORANGE);
          lcd.setCursor(60,127); lcd.print(String(ay));
          lcd.setCursor(168,127); lcd.print(String(nay));
          
// ******************************* end lcd-output **************************************
          last_fp1_ADC = avg_fp1; last_ang1 = ang1;
          last_fp2_ADC = avg_fp2; last_ang2 = ang2;
          last_fp3_ADC = avg_fp3; last_ang3 = ang3;
          last_fp4_ADC = avg_fp4; last_ang4 = ang4;
          last_ay = ay; last_nay = nay; 
        }
        
        delay(10);
        k++;
        break;
              
    case 2: // shutdown
       // servo angle stays at last written; no future signals are sent to servos
       // delay until long button press
       //if (touch) { delay(1000); if (still_touch) mode = 0; }
       delay(100);
       break;
       
    default:
        mode = 0;
        break;  

  }
  /* send serial data
     - ang1
     - ang2
     - ang3
     - ang4
  */
  int P = 0;
  while(Serial.available()>0){
      //Serial.println(incomingBit, DEC);
      P = 0;
      incomingBit = Serial.read();
      if(incomingBit == '1'){
        //flagrecieved = 1;
       //Serial.print("intialization Flag received: ");
       //Serial.println(incomingBit, DEC);
      while(P<5){
          if(P == 0){
            Serial.write(ang1);//send data over serial
          }
          if(P == 1){
            Serial.write(ang2);
          }
          if(P==2){
            Serial.write(ang3);
          }
          if(P==3){
            Serial.write(ang4);
          }
          P=P+1;
        }
      } 
    }
  
}
