int tics[3] = {0,0,0};
unsigned long timeMark = 0;
byte sensors[3] = {A1,A2,A3};     // Pins used to measure sensors
int lenTics= sizeof(tics)/sizeof(tics[0]);  // Length of benchmark-vector
int benchmark = 50;         // Benchmark for determining touch
int i;
int sampleTime = 1000;       // Sample time = 100 ms
int row = 0;                // Matrix rows
int touchMatrix[4][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(13, OUTPUT);    // sets the digital pin 13 as output
  pinMode(12, OUTPUT);    // sets the digital pin 12 as output
  pinMode(11, OUTPUT);    // sets the digital pin 11 as output
  pinMode(10, OUTPUT);    // sets the digital pin 10 as output

}

void loop() {
  
  for (i = 0; i < sizeof(sensors); i++){
    if (analogRead(sensors[i]) >= 900){  // Reads value either HIGH or LOW
      tics[i]++;                      // Counts tics
    }
  }

  if ((millis()-timeMark) >= sampleTime){     // Samples every 100 ms
    timeMark = millis();                      // Creates time stamp
    graph();                                  // Draws graph
    for (i = 0; i < lenTics; i++){
      if (tics[0] > benchmark){               // Decides if LED should be on
        digitalWrite(LED_BUILTIN, HIGH);
      } else{
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    memset(tics,0,sizeof(tics));              // Sets tics to 0
    
    if (row == 3){                            // Changes rows
      row = 0;
      digitalWrite(10+row, HIGH);             // sets the digital pin 1 on
      digitalWrite(13+row, LOW);              // sets the digital pin 4 off
    } else{
      row++;
      digitalWrite(10+row, HIGH);             // sets the digital pin 10+row on
      digitalWrite(9+row, LOW);               // sets the digital pin 9+row off
    }
  }
}


// Draws graph of all tics in Serial Plotter
void graph() {
  Serial.print(tics[0]);
  Serial.print(" ");
  Serial.print(tics[1]);
  Serial.print(" ");
  Serial.println(tics[2]);
}
