unsigned char incomingByte; // for incoming serial data

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
 Serial2.begin(9600); // opens serial port, sets data rate to 9600 bps
  //Serial1.begin(9600, SERIAL_8N1, PC_4, PC_5);



  while (!Serial);
  while (!Serial2);
//pinMode(PC_4,INPUT);
//pinMode(PC_5,OUTPUT);

}

void loop() {
  // reply only when you receive data:
  //if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial2.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte);
  //Serial.println(incomingByte, HEX);
  //}
}
