 //Test code for sending data to raspberry pi
int incomingBit;
int sentByte = 100;
int recieved;
int led = 13;

void setup() {
  Serial.begin(38400);//serial USB port connection
  //Serial.write(sentByte);//send data over serial 1
  pinMode(led,OUTPUT);
}


void loop() {
  int  k = 4;
    //Serial.println(45);
    //Serial.println(k);
    while(Serial.available()>0){
      //Serial.println(incomingBit, DEC);
      incomingBit = Serial.read();
      if(incomingBit == '1'){
        k = 4;
        //flagrecieved = 1;
       //Serial.print("intialization Flag received: ");
       //Serial.println(incomingBit, DEC);
      while(k>0){
          int Byte = sentByte + k;
          Serial.write(Byte);//send data over serial
          //Serial.println(Byte);//don't run this the same time as serialWrite
          delay(2);
          digitalWrite(led,HIGH);
          delay(2);
          digitalWrite(led,LOW);
          delay(2);
          k=k-1;
        }
      } 
    }
}


    
 
