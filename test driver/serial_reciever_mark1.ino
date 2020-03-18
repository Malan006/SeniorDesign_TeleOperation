 //Test code for recieveing serieal data from raspberry pi
int incomingByte;
int recieved;
int led = 13;

void setup() {
  Serial.begin(9600);//serial USB port connection
  pinMode(led,OUTPUT);
}



void loop() {
  digitalWrite(led,LOW);

     //serial USB comunication code
    while(Serial.available() > 0) {
      digitalWrite(led,LOW);
      delay(500);
        incomingByte = Serial.read();
        Serial.print("USB received: ");
        Serial.println(incomingByte, DEC);
        digitalWrite(led,HIGH);
        delay(2000);
        }
}


    
