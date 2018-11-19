unsigned char inByte = 0;         // incoming serial byte

void setup() 
{
  // start serial port at 19200 bps:
  Serial.begin(19200, SERIAL_8N1);
  // start serial 2  port at 115200 bps:
  Serial2.begin(115200, SERIAL_8N1);

  Serial.flush();
  Serial2.flush();
}

void loop() 
{
}

void serialEvent()
{
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) 
  {
    // get incoming byte:
    inByte = Serial.read();

    Serial2.write(inByte);
  }
}

void serialEvent2()
{  
  // if we get a valid byte, read analog ins:
  if (Serial2.available() > 0) 
  {
    // get incoming byte:
    inByte = Serial2.read();

    Serial.write(inByte);
  }
}

