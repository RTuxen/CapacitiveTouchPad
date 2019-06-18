#include <stdio.h>
#include <string.h>

// Values for password-logic
bool countingCycleFinished = false; // Flag when all sensors have been counted
bool selectingPassword = false;     // Flag when setting new password
bool calibrated = true;            // Flag when system is calibrated
int touchCode[4] = {-1,-1,-1,-1};   // Code typed by the user
int password[5] = {1,3,2,3};        // Password
int key = -1;                       // Current key pressed by user
int lastKey = -1;                   // Used to prevent user from holding down key
int index = 0;                      // Current index of the touchCode

// Values used for selecter-logic
bool flag = 0;              // Switch between HIGH and LOW
float frekvens = 100;         // Wanted frequency - Falls of at 5kHz when no prints
long tolerance = 10;        // Number of -1 keys before ready to new value
long T = 1000000/frekvens/2;// Period of the frequency
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

//long benchmark_ROM[7] = {41666,41666,38461,41666,43478,38461,40000};
//long sensitivity_ROM[7] = {5866,};

long benchmark_ROM[7] = {47619,47619,47619,43478,45454,41666,43478};
long sensitivity_ROM[7] = {7619,4787,5952,6478,7454,6666,6478};

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
    
void setup() {
    Serial.begin(9600);

    DDRD |= (1 << PD7);   // Sets PD7 to output (clk)  (pin 6);
    DDRD |= (1 << PD4);   // Sets PD7 to output (reset)(pin 4);
    pinMode(PD2,INPUT);   // Sets PD2 as input         (pin 2);
    pinMode(PD3, OUTPUT); // Led der lyser og shit     (pin 3);

    // Callibrates selector
    set_selector();

    tid = 0;
    tid_last = 0;
}

void loop() {  
  row_column_counter_ROM(); // Counts sensors
  
  if(sensor_ROM != sensor_ROM_Last){
    //Serial.print("Sensor : ");
    //Serial.println(sensor_ROM);
    sensor_ROM_Last = sensor_ROM;
    touchCounter_ROM();       // Counts tics on input pin
  }
  

  if (countingCycleFinished && !calibrated){
    calibrated = true;
  }

  if(countingCycleFinished){
    //Serial.println("COUNTING CYCLE FINISHED");
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
      }
      lastKey = key;
      
    } else{ // Multiple touches registered
      lastKey = -1;
      minusOnes++;
      Serial.println("Multiple touches");
    }
    countingCycleFinished = false;
  }
  
}

///////////////////////////////////////////////////////////
//////////////////INITIALIZATION///////////////////////////
///////////////////////////////////////////////////////////

// Initializes counter pins
void set_selector() {
  PORTD &= ~(1 << PD4);  // Set PD4 (reset) LOW
  

  // Clocking with 100 Hz for 1 s
  while (millis() < 100) {
    tid = millis() - tid_last;
  
    if (tid > 10) {
      tid_last = millis();
    
      if (flag){
        flag = false;
        PORTD |= (1 << PD7);  // Set PD7 (clk) HIGH
       }
      else if (!flag) {
        flag = true;
        PORTD &= ~(1 << PD7);  // Set PD7 (clk) LOW
       }
     }
  }
  flag = true;
  // Clock LOW
  PORTD &= ~(1 << PD7);  // Set PD7 (clk) LOW

  // Reset HIGH
  PORTD |= (1 << PD4);  // Set PD4 (reset) HIGH
}

///////////////////////////////////////////////////////////
//////////////////INTEGRATOR///////////////////////////////
///////////////////////////////////////////////////////////

// Counts columns
void row_column_counter_CVD() {
  // Gets time
  tid = micros();
  
  // If time to switch
  if ((tid - tid_last > T)) {
    tid_last = micros();
      
    if (!flag){ // LOW
      flag = true;
      PORTD &= ~(1 << PD7);  // Set PD7 (reset) LOW
      }
    else if (flag) { // HIGH
      PORTD |= (1 << PD7);  // Set PD7 (reset) HIGH
      flag = false;
      column++;
      if(column == 4){
        columnCountingFinished = true; 
        column = 0;
      } else if ((row == 2) && (column == 3)){
        countingCycleFinished = true;
      }
    }
  }
  // Switches sensor
  if(columnCountingFinished){
    row++;
    if(row == 3){
      row = 0;
    }
    columnCountingFinished = false;
  }
}

void touchCounter_CVD(){  
  // Reads input on pin 2 (PD2)
  if (PIND & B00000010) touchMatrix_CVD[row][column]++;

  if(countingCycleFinished){
    for (i = 0; i <= 2; i++){
      for (k = 0; k <= 3; k++){
        if(touchMatrix_CVD[i][k] >= benchmark_CVD[i][k]){
          touch[i][k] = 1;
        } else{
          touch[i][k] = 0;
        }
      }
    }
    memset(touchMatrix_CVD, 0, sizeof(touchMatrix_CVD)); // Clears touch matrix
  }
}

///////////////////////////////////////////////////////////
////////////RELAXATION OSCILLATOR//////////////////////////
///////////////////////////////////////////////////////////


// Sensor counter for ROM
void row_column_counter_ROM(){
  // Gets time
  tid = micros();
  
  // If time to switch
  if ((tid - tid_last > T)) {
    tid_last = micros();
      
    if (!flag){ // LOW
      flag = true;
      PORTD &= ~(1 << PD7);  // Set PD7 (reset) LOW
      }
    else if (flag) { // HIGH
      PORTD |= (1 << PD7);  // Set PD7 (reset) HIGH
      flag = false;
      sensor_ROM++;
      if(sensor_ROM == 6){
        countingCycleFinished = true;
      } else if(sensor_ROM == 7){
        sensor_ROM = 0;
      }
    }
  }
}

void touchCounter_ROM(){  
    high_time = pulseIn(PD2,HIGH);
    low_time = pulseIn(PD2,LOW);
    time_period = high_time + low_time;
    frequency = 1000000/time_period;
    touchMatrix_ROM[sensor_ROM] = frequency; // In Hz

  if(countingCycleFinished){
    touches = 0;
    for (i = 0; i <= 6; i++){
      //Serial.print(touchMatrix_ROM[i]);
      //Serial.print(" ");
      if(benchmark_ROM[i]-touchMatrix_ROM[i] >= sensitivity_ROM[i]){
        //Serial.print("Touch på sensor : ");
        //Serial.println(i);
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
    //Serial.println(" ");

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
    memset(touchMatrix_ROM, 0, sizeof(touchMatrix_ROM)); // Clears touch matrix
    memset(row_ROM, 0, sizeof(row_ROM));
    memset(column_ROM, 0, sizeof(column_ROM));
  }
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
    printPassword();
    if(index == 4){
      Serial.println("NEW PASSWORD COMPLETE");
      digitalWrite(PD3, LOW);
      selectingPassword = false;
      index = 0;
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
    printCode();
  } else if((key == 10) && (lastKey !=key) && ((minusOnes >= tolerance) && (lastKey = -1))){  // Enter is pressed
    Serial.println("Key pressed: Enter");
    if(memcmp(touchCode, password, sizeof(touchCode)) == 0){  // Enter is pressed
      Serial.println("CORRECT CODE");
      Serial.println("--------------------");
      Serial.println("TYPE NEW PASSWORD");
      selectingPassword = true;
      memset(password,-1, sizeof(password));
      digitalWrite(PD3, HIGH);
    } else{
      Serial.println("WRONG CODE");
    }
    index = 0;
    memset(touchCode,-1, sizeof(touchCode));
  } else if((key == 11) && (lastKey !=key) && ((minusOnes >= tolerance) && (lastKey = -1))){  // Delete is pressed
    Serial.println("Key pressed: Delete");
    if((index >= 1) && (index <= 4)){
      index --;
      touchCode[index] = -1;
    }
    printCode();
  }
}

void printCode(){
  Serial.print("Code: ");
  for(g = 0; g <= 3; g++){
    if(touchCode[g] != -1){
      Serial.print(touchCode[g]);
      Serial.print(" ");
    }
  }
  Serial.println();
}

void printPassword(){
  Serial.print("Password: ");
  for(g = 0; g <= 3; g++){
    if(password[g] != -1){
      Serial.print(password[g]);
      Serial.print(" ");
    }
  }
  Serial.println();
}
