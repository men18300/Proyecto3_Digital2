unsigned char incomingByte; // for incoming serial data

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
 Serial2.begin(9600); // opens serial port, sets data rate to 9600 bps
  //Serial1.begin(9600, SERIAL_8N1, PC_4, PC_5);



 // while (!Serial);
//  while (!Serial1);
//pinMode(PC_4,INPUT);
//pinMode(PC_5,OUTPUT);

}

void loop() {
  // reply only when you receive data:
  //if (Serial.available() > 0) {
    // read the incoming byte:
   // incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(Serial2.read());
   // delay(100);
  //Serial.println(incomingByte, HEX);
  //}
}
