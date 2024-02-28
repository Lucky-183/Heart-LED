此程序用于修改蓝牙设备名字,设备名与APP运行相关联,勿改
HardwareSerial Serial2(PA1, PA0);
void setup() {
  pinMode(PB_2, OUTPUT);
  digitalWrite(PB_2, HIGH);
  Serial.begin(115200);
  Serial2.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial2.println("AT+STORE=0");
  delay(300);
  Serial2.println("AT+NAME=Heart_LED");
  delay(3000);
  Serial2.println("AT+STORE=1");
  while(1){
    Serial2.println("AT+NAME?");
      if (Serial2.available()) {
      String dataBuffer = Serial2.readStringUntil('.'); 
      Serial.println(dataBuffer); }
  }
}
