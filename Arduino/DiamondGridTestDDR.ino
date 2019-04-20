int tics[3] = {0,0,0};
unsigned long timeMark = 0;
unsigned long currentTime = 0;
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
  // DDRX -- Data Direction Re  gister 
  // -determines which way data should flow for each pin on the port
  // (0 is input, 1 is output)
  DDRD = B10000000; // sets port 7 to output and port 6-0 to input

}

void loop() {
  PORTD = B10000000;    // Sets pin D6 HIGH

  // It might be possible to improve this section with DDRD register or pinMode
  for (i = 0; i < sizeof(sensors); i++){
    if (analogRead(sensors[i]) >= 900){       // Reads value either HIGH or LOW
      tics[i]++;                              // Counts tics
    }
  }

  currentTime = millis();
  if ((currentTime-timeMark) >= sampleTime){     // Samples every 100 ms
    timeMark = currentTime;                      // Creates time stamp
    graph();                                     // Draws graph
    for (i = 0; i < lenTics; i++){
      if (tics[0] > benchmark){                  // Decides if LED should be on
        digitalWrite(LED_BUILTIN, HIGH);
      } else{
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    memset(tics,0,sizeof(tics));              // Sets tics to 0
  }
  PORTD = PORTD & B01111111;    // Sets pin D6 LOW
}


// Draws graph of all tics in Serial Plotter (Press Ctrl+Shift+L to enable)
void graph() {
  Serial.print(tics[0]);
  Serial.print(" ");
  Serial.print(tics[1]);
  Serial.print(" ");
  Serial.println(tics[2]);
}
