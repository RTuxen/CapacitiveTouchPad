#include <stdio.h>
#include <string.h>

// Values for password-logic
bool countingCycleFinished = false;               // Flag when all sensors have been counted
int touchCode[4] = {-1,-1,-1,-1};
int password[5] = {1,3,2,3};
int key = -1;
int lastKey = -1;
int index = 0;
int lenCode= sizeof(password)/sizeof(password[0]);

// Values used for selecter-logic
bool flag = 0;              // Switch between HIGH and LOW
long frekvens = 2;          // Wanted frequency - Falls of at 5kHz when no prints
long T = 1000000/frekvens/2;// Period of the frequency
long tid = 0;               // Real time value
long tid_last = 0;          // Time

// Values for CVD
int row = 0;                // Current row number
int column = 0;             // Current column number
int columnLast = 0;
bool columnCountingFinished = false;

// Values for ROM
int sensor_ROM = 0;
int sensor_ROM_Last = 0;
int touchMatrix_ROM[7] = {0,0,0,0,0,0,0};
int benchmark_ROM[7] = {0,0,0,0,0,0,0};
int row_ROM[3] = {0,0,0};
int column_ROM[4] = {0,0,0,0};

// Values used in pin counting
int k,i;
int touchMatrix[3][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
int benchmark[3][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
int touch[3][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};
    
void setup() {
    Serial.begin(9600);

    // Sets pin 4 and 6 as outputs - rest as inputs
    DDRD = B10010000;

    // Callibrates selector
    set_selector();

    tid = 0;
    tid_last = 0;
}

void loop() {
  ///////////////////////
  //////////CVD//////////
  ///////////////////////
  
  row_column_counter_CVD(); // Counts rows and columns
  if((column != columnLast)){
    Serial.print("Row : ");
    Serial.print(row);
    Serial.print(" column: ");
    Serial.println(column);
    columnLast = column;
  }
  touchCounter_CVD();     // Counts tics on input pin
  

  ///////////////////////
  //////////ROM//////////
  ///////////////////////
  /*
  row_column_counter_ROM(); // Counts sensors
  touchCounter_ROM();       // Counts tics on input pin
  
  if(sensor_ROM != sensor_ROM_Last){
    Serial.print("Sensor : ");
    Serial.println(sensor_ROM);
    sensor_ROM_Last = sensor_ROM;
  }
  */
  
  if (countingCycleFinished){
    Serial.println("COUNTING CYCLE FINISHED");
    countingCycleFinished = false;
    key = determineWhatButtonWasPressed();

    if (( key >= 0) && (key <= 9) && (lastKey != key)){    // If input is between 0-9
      if(index == 4){
        index--;
      }
      touchCode[index] = key;
      index++;
    } else if((key == 10) && (lastKey !=key)){
      if (index == lenCode){    // If password attempt is finished
        if (memcmp(touchCode, password, sizeof(touchCode)) == 0){
          Serial.println("CORRECT CODE");
          //newCode(password,lenCode);
        } else{
          Serial.println("WRONG CODE");
        }
      } else{
        Serial.println("WRONG CODE");
      }
      index = 0;
      memset(touchCode,-1, lenCode*4);
    } else if((key == 11) && (lastKey !=key)){
      index--;
      if(index < 0){
        index = 0;
      }
      touchCode[index] = -1;
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
  PORTD = PORTD | B00010000;

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
  // Reset and clock LOW
  PORTD = PORTD & B01101111;
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
      PORTD = PORTD | B10000000;
      }
    else if (flag) { // HIGH
      // cnt - return value of current row
      PORTD = PORTD & B01111111;
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
  if (PIND & B00000010) touchMatrix[row][column]++;

  if(countingCycleFinished){
    for (i = 0; i <= 2; i++){
      for (k = 0; k <= 3; k++){
        if(touchMatrix[i][k] >= benchmark[i][k]){
          touch[i][k] = 1;
        } else{
          touch[i][k] = 0;
        }
      }
    }
    memset(touchMatrix, 0, sizeof(touchMatrix[0][0])*4*3); // Clears touch matrix
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
      PORTD = PORTD | B10000000;
      }
    else if (flag) { // HIGH
      // cnt - return value of current row
      PORTD = PORTD & B01111111;
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
  // Reads input on pin 2 (PD2)
  if (PIND & B00000010) touchMatrix_ROM[sensor_ROM]++;

  if(countingCycleFinished){
    for (i = 0; i <= 6; i++){
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
    }

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

int determineWhatButtonWasPressed(){
  int button = -1;
  if(touch[0][0] == 1){
    button = 7;
  }else if(touch[0][1] == 1){
    button = 8;
  }else if(touch[0][2] == 1){
    button = 9;
  }else if(touch[1][0] == 1){
    button = 4;
  }else if(touch[1][1] == 1){
    button = 5;
  }else if(touch[1][2] == 1){
    button = 6;
  }else if(touch[2][0] == 1){
    button = 1;
  }else if(touch[2][1] == 1){
    button = 2;
  }else if(touch[2][2] == 1){
    button = 3;
  }else if(touch[3][0] == 1){
    button = 10;
  }else if(touch[3][1] == 1){
    button = 0;
  }else if(touch[3][2] == 1){
    button = 11;
  }
  return button;
}

// Enables user to change password
void newCode(int buf[], int size) {
  int key;
  int j=0;

  for (int g = 0; g < size; g++) {
    buf[g] = -1;
  }

  while (j < size){
    printf("\nNEW PASSWORD:  ");
    scanf(" %d", &key);
    if ((key >= 0) && (key <= 9)){    // If input is between 0-9
      buf[j] = key;
      j++;
    }
  }
}
