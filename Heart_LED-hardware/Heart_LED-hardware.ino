// 注意修改芯片运行频率,最高.

int ledPin = PB_0;
HardwareSerial Serial2(PA1, PA0);

unsigned long previousMillis = 0;
unsigned long previousMillisSensor = 0;
unsigned long interval = 20;        // 时间间隔（毫秒），初始设置为8
const long intervalSensor = 2000;
int brightness = 0;
int fadeAmount = 3;
int state = 0;
volatile bool motionDetected = false; // 用于指示是否检测到运动
int mode = 1; // 默认为模式1
int Brightness = 10; // 模式2下的指定亮度
int sensorValue = 0;
int lastSensorValue = 0; // 上一次记录的光强传感器值
int deadZone = 50; // 光强死区阈值
int upperThreshold = 105, lowerThreshold = 56, upperThreshold2 = 180;
int sensorLowerBound = 400, sensorUpperBound = 1023;
int brightnessLowerBound = 0, brightnessUpperBound = 90;

void motionDetectedISR() {
  motionDetected = true;
  Serial2.println("motionDetected");
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(PB_2, OUTPUT);
  pinMode(PA7, INPUT); // 将PA7设置为输入
  // 附加中断，触发模式为上升沿（从低变高）
  attachInterrupt(digitalPinToInterrupt(PA7), motionDetectedISR, RISING);
  digitalWrite(PB_2, LOW);
  Serial.begin(115200);
  Serial2.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();

 if (mode == 1 && motionDetected || mode == 0){
      // LED PWM控制
   if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // 更新时间

    if (state == 0) { // 亮度逐渐增加
      brightness += fadeAmount;
      if (brightness >= upperThreshold) {
        state = 1; // 切换到下降状态
      }
    } else if (state == 1) { // 亮度逐渐减少
      brightness -= fadeAmount;
      if (brightness <= lowerThreshold) {
        state = 2; // 切换到再次增加状态
      }
    } else if (state == 2) { // 亮度再次增加
      brightness += fadeAmount;
      if (brightness >= upperThreshold2) {
        state = 3; // 切换到暂停状态
        previousMillis += 500; // 增加吸气末尾的停顿
      }
    } else if (state == 3) { // 渐渐减少亮度
      brightness -= fadeAmount;
      if (brightness <= 0) {
        state = 4; // 切换到暂停状态
        previousMillis += 500; // 增加呼气末尾的停顿
      }
    } else if (state == 4 && brightness <= 0) { // 重置状态为上升
      state=0;
      motionDetected = false; // 重置标志
    }
    analogWrite(ledPin, brightness); // 更新LED亮度
  }
  }

  else if (mode == 2){
    analogWrite(ledPin, Brightness);
  }
  else if (mode == 3) {
    // 根据光强反馈调节亮度
    int sensorValue = analogRead(PA_4);
    if (abs(sensorValue - lastSensorValue) > deadZone) {
    lastSensorValue = sensorValue; // 更新传感器值
    int mappedBrightness = map(sensorValue, sensorLowerBound, sensorUpperBound, brightnessLowerBound, brightnessUpperBound); // 映射到0-255范围
    analogWrite(ledPin, mappedBrightness);
  }
  }  
  else if (mode == 4) {
    // 根据光强反馈调节亮度
    int sensorValue = analogRead(PA_4);
    if (abs(sensorValue - lastSensorValue) > deadZone) {
    lastSensorValue = sensorValue; // 更新传感器值
    int mappedBrightness = map(sensorValue, sensorLowerBound, sensorUpperBound, brightnessUpperBound, brightnessLowerBound); // 映射到0-255范围
    analogWrite(ledPin, mappedBrightness);
  }

  }

  // 读取串口2的数据
  if (Serial2.available()) {
    String dataBuffer = Serial2.readStringUntil('.'); 
    Serial.println(dataBuffer); 
    if (dataBuffer.startsWith("F:")) {
      fadeAmount = dataBuffer.substring(2).toInt(); // 修正为从冒号之后提取数字
      Serial2.println("Fade changed");
    }
    else if (dataBuffer.startsWith("I:")) {
      interval = dataBuffer.substring(2).toInt();  // 提取并转换间隔时间
      Serial2.println("Interval changed");
    }
    else if (dataBuffer.startsWith("SET1:")) {
        int commaIndex1 = dataBuffer.indexOf(',', 5);
        int commaIndex2 = dataBuffer.indexOf(',', commaIndex1 + 1);
        
        // 提取并更新阈值
        if (commaIndex1 != -1 && commaIndex2 != -1) {
            upperThreshold = dataBuffer.substring(5, commaIndex1).toInt();
            lowerThreshold = dataBuffer.substring(commaIndex1 + 1, commaIndex2).toInt();
            upperThreshold2 = dataBuffer.substring(commaIndex2 + 1).toInt();
            Serial2.println("Mode 1 thresholds updated");
        }
    }
    else if (dataBuffer.startsWith("M:")) {
      mode = dataBuffer.substring(2).toInt();
      Serial2.println("Mode changed");
    }
    else if (dataBuffer.startsWith("SET2:")) {
      Brightness = dataBuffer.substring(5).toInt();
      Serial2.println("Bright changed");
    }
    else if (dataBuffer.startsWith("SETMAP:")) {
    int firstComma = dataBuffer.indexOf(',', 7);
    int secondComma = dataBuffer.indexOf(',', firstComma + 1);
    int thirdComma = dataBuffer.indexOf(',', secondComma + 1);

    if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
        sensorLowerBound = dataBuffer.substring(7, firstComma).toInt();
        sensorUpperBound = dataBuffer.substring(firstComma + 1, secondComma).toInt();
        brightnessLowerBound = dataBuffer.substring(secondComma + 1, thirdComma).toInt();
        brightnessUpperBound = dataBuffer.substring(thirdComma + 1).toInt();
        Serial2.println("Map parameters updated");
      }
    }
    else if (dataBuffer.startsWith("DZ:")) {
    deadZone = dataBuffer.substring(3).toInt();
    Serial2.println("Dead zone changed");
    }
    // else if (dataBuffer.startsWith("PB2:")) {
    //   int pb2State = dataBuffer.substring(4).toInt();
    //   digitalWrite(PB_2, pb2State); // 设置PB2引脚的状态
    //   Serial2.println("PB2 state changed");
    // }
    dataBuffer = "";
  }

  // 读取传感器数据
  if (currentMillis - previousMillisSensor >= intervalSensor) {
    previousMillisSensor = currentMillis;
    int sensorValue = analogRead(PA_4);
    Serial2.println(sensorValue);
  }
}
