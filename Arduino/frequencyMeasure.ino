int value = 0;
int lastValue = 0;
int tics = 0;
unsigned long timeMark = 0;
int benchmark = 0;
int analogPin = A3;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
  if (analogRead(analogPin) >= 500){  // Reads value either HIGH or LOW
    value = 1;
  } else {
    value = 0;
  }

  if (lastValue != value){
   tics = tics + value;              // Counts tics 
   lastValue = value;
  }

  if ((millis()-timeMark) >= 100){
    benchmark = tics;
    timeMark = millis();
    tics = 0;
  }

  if (benchmark > 50){              // Decides if LED should be on
    digitalWrite(LED_BUILTIN, HIGH);
  } else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  //Serial.print(0);  // To freeze the lower limit
  //Serial.print(" ");
  //Serial.print(5);  // To freeze the upper limit
  //Serial.print(" ");
  //Serial.println(analogRead(analogPin)*5/1023);             // debug value
  Serial.println(benchmark);
  Serial.print(" ");

  
}
