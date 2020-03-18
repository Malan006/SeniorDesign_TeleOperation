#include <Servo.h>


 Servo sv1, sv2, sv3, sv4, sv5, sv6, sv7, sv8;
 int ang1, ang2, ang3, ang4, nay = 0, led = 13, serialCounter = 1, incomingByte = 0;
void setup() {
  Serial.begin(38400);
  pinMode(led,OUTPUT);
  serialCounter = 1;
  
 
  
  // attach servos
  sv1.attach(3,500,1900); // base, 180 deg
  sv2.attach(4,1900,500); // base, 180 adj to sv1
  sv3.attach(5,1900,500); // arm
  sv4.attach(6,500,1900); // arm, adj to sv3
  sv5.attach(20,500,1900); // wrist
  sv6.attach(21,1900,500); // wrist, adj to sv5
  sv7.attach(22,500,1900); // gripper, very small angle
  sv8.attach(23,500,1900); // rotation, 0-180 deg

  delay (1000); // delay for no reason?
}

void loop() {
   //serial input is angles 1-4
   while(Serial.available()>0){
      digitalWrite(led,LOW);
      incomingByte = Serial.read();
      if (serialCounter == 1){
        ang1 = incomingByte;
      }
      if(serialCounter == 2){
        ang2 = incomingByte;
      }
      if(serialCounter == 3){
        ang3 = incomingByte;
      }
      if(serialCounter == 4){
        ang4 = incomingByte;
      }
   serialCounter ++;
   }
      
   sv1.write(ang4); // middle finger
   sv2.write(ang4); // middle finger
   sv3.write(ang2); // index finger
   sv4.write(ang2); // index finger
   sv5.write(ang1); // ring
   sv6.write(ang1); // ring
   sv7.write(ang3); // thumb
   sv8.write(ang3); // thumb
}
