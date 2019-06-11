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
long frekvens = 1;          // Wanted frequency - Falls of at 5kHz when no prints
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
long benchmark_ROM[7] = {0,0,0,0,0,0,0};
int row_ROM[3] = {0,0,0};
int column_ROM[4] = {0,0,0,0};

// Values used in pin counting
int k,i;
int touch[3][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
    
void setup() {
    Serial.begin(9600);

    // Sets pin 4 and 6 as outputs - rest as inputs
    DDRD = B10010000;
    //pinMode(2, INPUT);

    // Callibrates selector
    set_selector();

    tid = 0;
    tid_last = 0;
}

void loop() {
  ///////////////////////
  //////////CVD//////////
  ///////////////////////
  /*
  row_column_counter_CVD(); // Counts rows and columns
  if((column != columnLast)){
    Serial.print("Row : ");
    Serial.print(row);
    Serial.print(" column: ");
    Serial.println(column);
    columnLast = column;
  }
  touchCounter_CVD();     // Counts tics on input pin
  */
  
  ///////////////////////
  //////////ROM//////////
  ///////////////////////
  
  row_column_counter_ROM(); // Counts sensors
  touchCounter_ROM();       // Counts tics on input pin
  
  if(sensor_ROM != sensor_ROM_Last){
    //Serial.print("Sensor : ");
    //Serial.println(sensor_ROM);
    sensor_ROM_Last = sensor_ROM;
  }
  

  if (countingCycleFinished && !calibrated){
    calibrated = true;
  }

  if(countingCycleFinished){
    key = determineWhatButtonWasPressed();
    countingCycleFinished = false;
    //Serial.println("COUNTING CYCLE FINISHED");
    if(selectingPassword){  // Setting new password
      newPassword();
    } else{                 // When not selecting password
      userCode();
    }
    lastKey = key;
  }
  
}

///////////////////////////////////////////////////////////
//////////////////INITIALIZATION///////////////////////////
///////////////////////////////////////////////////////////

// Initializes counter pins
void set_selector() {
  // Reset HIGH
  //PORTD = PORTD | B00010000;

  // Reset LOW
  PORTD = PORTD & B11101111;

  // Clocking with 100 Hz for 1 s
  while (millis() < 100) {
    tid = millis() - tid_last;
  
    if (tid > 10) {
      tid_last = millis();
    
      if (flag){
        flag = false;
        PORTD = PORTD | B10000000;
       }
      else if (!flag) {
        flag = true;
        PORTD = PORTD & B01111111;
       }
     }
  }
  flag = true;
  // Reset and clock LOW
  //PORTD = PORTD & B01101111;
  PORTD = PORTD & B01111111;

  // Reset HIGH
  PORTD = PORTD | B00010000;
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
      PORTD = PORTD & B01111111;
      }
    else if (flag) { // HIGH
      // cnt - return value of current row
      PORTD = PORTD | B10000000;
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
        if(!calibrated){
          benchmark_CVD[i][k] = touchMatrix_CVD[i][k];
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
      PORTD = PORTD & B01111111;
      }
    else if (flag) { // HIGH
      // cnt - return value of current row
      PORTD = PORTD | B10000000;
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
  // Reads input on pin 2 (PD1)
  if (PIND & B00000010) touchMatrix_ROM[sensor_ROM]++;
  //if(digitalRead(2) == HIGH){
    //touchMatrix_ROM[sensor_ROM]++;
  //}
  //if (analogRead(A3) > 700){
  //  touchMatrix_ROM[sensor_ROM]++;
  //}

  if(countingCycleFinished){
    for (i = 0; i <= 6; i++){
      //Serial.print(touchMatrix_ROM[i]);
      //Serial.print(" ");
      if(touchMatrix_ROM[i] >= benchmark_ROM[i]){
        if(i < 3){
          row_ROM[i] = 1;
        } else{
          column_ROM[i] = 1;
        }
      } else{
        if(i < 3){
          row_ROM[i] = 0;
        } else{
          column_ROM[i] = 0;
        }
      }
      if(!calibrated){
        benchmark_ROM[i] = touchMatrix_ROM[i];
      }
    }
    //Serial.println(" ");

    for (i = 0; i <= 2; i++){
      for (k = 0; k <= 3; k++){
        if((row_ROM[i] == 1) && (column_ROM[k] == 1)){
          touch[i][k] = 1;
        } else{
          touch[i][k] = 0;
        }
      }
    }
    memset(touchMatrix_ROM, 0, sizeof(touchMatrix_ROM)); // Clears touch matrix
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
  if(( key >= 0) && (key <= 9) && (lastKey != key)){
    password[index] = key;
    index++;
    if(index == 4){
      selectingPassword = false;
      index = 0;
    }
  }
}

// When user types code
void userCode(){
  if(( key >= 0) && (key <= 9) && (lastKey != key)){  // Number is pressed
    if((index >= 0) || (index <= 3)){
      touchCode[index] = key;
      index++;
    }
  } else if((key == 10) && (lastKey !=key)){
    if(memcmp(touchCode, password, sizeof(touchCode)) == 0){  // Enter is pressed
      Serial.println("CORRECT CODE");
      selectingPassword = true;
    } else{
      Serial.println("WRONG CODE");
    }
    index = 0;
    memset(touchCode,-1, sizeof(touchCode));
  } else if((key == 11) && (lastKey !=key)){  // Delete is pressed
    if((index >= 1) && (index <= 4)){
      index --;
      touchCode[index] = -1;
    }
  }
}
