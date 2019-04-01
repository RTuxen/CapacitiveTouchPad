int value[2] = {0,0};
int lastValue[2] = {0,0};
int tics[2] = {0,0};
unsigned long timeMark = 0;
int benchmark[2] = {0,0};
byte pins[2] = {A3,A4};     // Pins used
int lenBenchmark = sizeof(benchmark)/sizeof(benchmark[0]);  // Length of benchmark-vector

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
  countTics();

  // Prints all benchmarks
  for (int x=0; x<lenBenchmark;x++){
    if (benchmark[x] > 50){              // Decides if LED should be on
      digitalWrite(LED_BUILTIN, HIGH);
    } else{
      digitalWrite(LED_BUILTIN, LOW);
    }
    Serial.print("Pin : A");
    Serial.print(x+3);
    Serial.print(" : ");
    Serial.print(benchmark[x]);
    Serial.print(" ");
  }  
  Serial.println();
}

// Count tics and updates benchmark at a given rate
int countTics(){
  for (int i=0; i < sizeof(pins); i++){
    if (analogRead(pins[i]) >= 500){  // Reads value either HIGH or LOW
      value[i] = 1;
    } else {
      value[i] = 0;
    }
    if (lastValue[i] != value[i]){              // Only count if value has changed
      tics[i] = tics[i] + value[i];             // Counts tics 
      lastValue[i] = value[i];
    }
    if ((millis()-timeMark) >= 100){            // Samples every 100 ms
      benchmark[i] = tics[i];
      timeMark = millis();
      memset(tics,0,sizeof(tics));              // Sets tics to 0
    }
  }
}
