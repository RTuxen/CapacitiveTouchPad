#include <stdio.h>
#include <string.h>
#include <avr/io.h> // Enables IDE to recognise ATTiny85 port commands

// Values for password-logic
bool countingCycleFinished = false; // Flag when all sensors have been counted
bool selectingPassword = false;     // Flag when setting new password
int touchCode[4] = {-1,-1,-1,-1};   // Code typed by the user
int password[5] = {1,3,2,3};        // Password
int key = -1;                       // Current key pressed by user
int lastKey = -1;                   // Used to prevent user from holding down key
int index = 0;                      // Current index of the touchCode

// Values used for selecter-logic
bool flag = false;              // Switch between HIGH and LOW
float frekvens = 700;         // Wanted frequency - Falls of at 5kHz when no prints
long tolerance = 23;        // Number of -1 keys before ready to new value
long T = 1000/frekvens/2;// Period of the frequency
//long T = 1;
long recalibrateReady = 1000000;// When system re-calibrates
long tid = 0;               // Real time value
long tid_last = 0;          // Time

// Values for CVD
int row = 0;                          // Current row number
int column = 0;                       // Current column number
int columnLast = 0;                   // Last column, used for debugging
bool columnCountingFinished = false;  // Logic used for counting
int touchMatrix_CVD[3][4] = {         // Array of tic counting
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
int benchmark_CVD[3][4] = {           // Array of benchmark tics
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

// Values for ROM
int sensor_ROM = 0;
int sensor_ROM_Last = 0;
long touchMatrix_ROM[7] = {0,0,0,0,0,0,0};
long benchmark_ROM[7] = {47619,47619,47619,43478,45454,41666,43478};  // From Breadboard measurements
long sensitivity_ROM[7] = {7619,4787,5952,6478,7454,6666,6478};       // From Breadboard measurements

//long benchmark_ROM[7] = {43478,45454,43478,45454,45454,45454,45454};  // From PCB measurements
//long touch_freq_ROM[7] = {35714,37037,26315,31250,31250,32258,34482}; // From PCB measurements
long touch_freq_ROM[7] = {37000,39000,28000,33000,33000,34000,36000};   // From PCB measurements

// 
int row_ROM[3] = {0,0,0};
int column_ROM[4] = {0,0,0,0};
int touches = 0;

// Values used in pin counting
int k,i,g;
int minusOnes = 0;
int touch[3][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

// pulse counting
int high_time;
int low_time;
float time_period;
float frequency;
long LED_uptime = 0;
long LED_time = 0;
long LED_timeOn = 100; // In millis
bool LED_UP = false;
int multiTouchCounter = 0;
    
void setup() {
  //Serial.begin(9600);
    
  DDRB |= (1 << PB1);   // Sets pin 1 to output (clk);
  DDRB |= (1 << PB0);   // Sets pin 2 to output (reset);
  
  pinMode(PB3,INPUT);   // Sets pin 0 as input;
  DDRB |= (1 << PB4);   // Sets pin 4 to output (LED);

  digitalWrite(PB4,HIGH);

  // Callibrates selector
  set_selector();

  tid = 0;
  tid_last = 0;
  digitalWrite(PB4,LOW);
}

void loop() {  
  row_column_counter_ROM(); // Counts sensors

  
  if((sensor_ROM != sensor_ROM_Last)){
    sensor_ROM_Last = sensor_ROM;
    frequencyMeasure_ROM();   // Measures frequency
  }

  
  
  if(countingCycleFinished){
    touchCounter_ROM();   // Determines if there are touches
    if (touches <= 1){ // One or zero touches
      key = determineWhatButtonWasPressed();
      
      if(selectingPassword){
        newPassword();
      } else{
        userCode();
      }

      if(key == -1){
        minusOnes++;
      } else{
        minusOnes = 0;
        digitalWrite(PB4, HIGH);
        LED_UP = true;
        LED_uptime = millis();
      }
      if(LED_UP){
        LED_time = millis()-LED_uptime;
        if(LED_time >= LED_timeOn){
          digitalWrite(PB4,LOW);
          LED_UP = false;
        }
      }
      multiTouchCounter = 0;
      lastKey = key;
      
    } else{ // Multiple touches registered
      lastKey = -1;
      minusOnes++;
      multiTouchCounter++;
      if(multiTouchCounter > 10){
        multiTouchBlink();
        multiTouchCounter = 0;
      }
    }
    countingCycleFinished = false;
  }
}

///////////////////////////////////////////////////////////
//////////////////INITIALIZATION///////////////////////////
///////////////////////////////////////////////////////////

// Initializes counter pins
void set_selector() {
  PORTB |= (1 << PB0);  // Set PB0 (reset) HIGH
  

  // Clocking with 100 Hz for 1 s
  while (millis() < 100) {
    tid = millis() - tid_last;
  
    if (tid > 10) {
      tid_last = millis();
    
      if (flag){
        flag = false;
        PORTB |= (1 << PB1);  // Set PB1 (clk) HIGH
       }
      else if (!flag) {
        flag = true;
        PORTB &= ~(1 << PB1);  // Set PB1 (clk) LOW
       }
     }
  }
  flag = true;
  // Clock LOW
  PORTB &= ~(1 << PB1);  // Set PB1 (clk) LOW

  // Reset LOW
  PORTB &= ~(1 << PB0);  // Set PB2 (reset) LOW
}

///////////////////////////////////////////////////////////
////////////RELAXATION OSCILLATOR//////////////////////////
///////////////////////////////////////////////////////////


// Sensor counter for ROM
void row_column_counter_ROM(){
  // Gets time
  tid = millis(); // Var micros
  
  // If time to switch
  if ((tid - tid_last > T)) {
    tid_last = millis(); // Var micros
      
    if (!flag){ // LOW
      flag = true;
      PORTB &= ~(1 << PB1);  // Set PB1 (clk) LOW
      }
    else if (flag) { // HIGH
      PORTB |= (1 << PB1);  // Set PB1 (clk) HIGH
      flag = false;
      sensor_ROM++;
      if(sensor_ROM == 7){
        countingCycleFinished = true;
      } else if(sensor_ROM == 8){
        sensor_ROM = 0;
      }
    }
  }
}

void frequencyMeasure_ROM(){
  if(sensor_ROM != 7){
    high_time = pulseIn(PB3,HIGH);
    low_time = pulseIn(PB3,LOW);
    time_period = high_time + low_time;
    frequency = 1000000/time_period;
    touchMatrix_ROM[sensor_ROM] = frequency; // In Hz
  }  
}

void touchCounter_ROM(){  
  touches = 0;
  for (i = 0; i <= 6; i++){
    // benchmark_ROM[i]-touchMatrix_ROM[i] >= sensitivity_ROM[i]
    if(touchMatrix_ROM[i] <= touch_freq_ROM[i]){
      if(i < 3){
        row_ROM[i] = 1;
      } else{
        column_ROM[i-3] = 1;
      }
    } else{
      if(i < 3){
        row_ROM[i] = 0;
      } else{
        column_ROM[i-3] = 0;
      }
    }
  }

  for (i = 0; i <= 2; i++){
    for (k = 0; k <= 3; k++){
      if((row_ROM[i] == 1) && (column_ROM[k] == 1)){
        touches++;
        touch[i][k] = 1;
      } else{
        touch[i][k] = 0;
      }
    }
  }

  if(minusOnes > recalibrateReady){ // When it is time to recalibrate
    for(i = 0; i <= 6; i++){
      benchmark_ROM[i] = touchMatrix_ROM[i];
    }
  }
  memset(touchMatrix_ROM, 0, sizeof(touchMatrix_ROM)); // Clears touch matrix
  memset(row_ROM, 0, sizeof(row_ROM));
  memset(column_ROM, 0, sizeof(column_ROM));
}


///////////////////////////////////////////////////////////
//////////////////PASSWORD LOGIC///////////////////////////
///////////////////////////////////////////////////////////

// Determines what key value was pressed
int determineWhatButtonWasPressed(){  
  int button = -1;
  if(touch[0][0] == 1){
    button = 7;
  }else if(touch[0][1] == 1){
    button = 8;
  }else if(touch[0][2] == 1){
    button = 9;
  }else if(touch[0][3] == 1){
    button = 11;
  }else if(touch[1][0] == 1){
    button = 4;
  }else if(touch[1][1] == 1){
    button = 5;
  }else if(touch[1][2] == 1){
    button = 6;
  }else if(touch[1][3] == 1){
    button = 0;
  }else if(touch[2][0] == 1){
    button = 1;
  }else if(touch[2][1] == 1){
    button = 2;
  }else if(touch[2][2] == 1){
    button = 3;
  }else if(touch[2][3] == 1){
    button = 10;
  }
  return button;
}

// When setting new password
void newPassword(){
  if(( key >= 0) && (key <= 9) && (lastKey != key) && ((minusOnes >= tolerance) && (lastKey = -1))){
    password[index] = key;
    index++;
    if(index == 4){
      selectingPassword = false;
      index = 0;
      resetPasswordBlink();
    }
  }
}

// When user types code
void userCode(){
  if(( key >= 0) && (key <= 9) && (lastKey != key) && ((minusOnes >= tolerance) && (lastKey = -1))){  // Number is pressed
    if((index >= 0) && (index <= 3)){
      touchCode[index] = key;
      index++;
    }
  } else if((key == 10) && (lastKey !=key) && ((minusOnes >= tolerance) && (lastKey = -1))){  // Enter is pressed
    if(memcmp(touchCode, password, sizeof(touchCode)) == 0){  // Password is correct
      selectingPassword = true;
      memset(password,-1, sizeof(password));
      blinkSomeTime();
    } else{   // Wrong password
      digitalWrite(PB4, HIGH);
      delay(2000);
      digitalWrite(PB4, LOW);
    }
    index = 0;
    memset(touchCode,-1, sizeof(touchCode));
  } else if((key == 11) && (lastKey !=key) && ((minusOnes >= tolerance) && (lastKey = -1))){  // Delete is pressed
    if((index >= 1) && (index <= 4)){
      index --;
      touchCode[index] = -1;
    }
  }
}

void blinkSomeTime(){
  for(g = 0; g <= 9; g++){
    digitalWrite(PB4, HIGH);
    delay(200);
    digitalWrite(PB4, LOW);
    delay(200);
  }
}
void multiTouchBlink(){
  for(g = 0; g <= 40; g++){
    digitalWrite(PB4, HIGH);
    delay(20);
    digitalWrite(PB4, LOW);
    delay(20);
  }
}
void resetPasswordBlink(){
  for(g = 0; g <= 3; g++){
    digitalWrite(PB4, HIGH);
    delay(200);
    digitalWrite(PB4, LOW);
    delay(200);
  }
  for(g = 0; g <= 50; g++){
    digitalWrite(PB4, HIGH);
    delay(20);
    digitalWrite(PB4, LOW);
    delay(20);
  }
}
