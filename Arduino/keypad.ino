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
int cnt = -1;               // Row-counter
int row = 0;                // yes
int row_last = 0;           // yes-last

// Values used in pin counting
int sampleTime = 100;         // Samples every 100 ms
unsigned long timeMark = 0;   // 
unsigned long currentTime = 0;//
int k,i;
int touchMatrix[4][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};
int benchmark[4][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};
int touch[4][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};
    
void setup() {
    Serial.begin(9600);

    // Sets pin 4 and 7 as outputs - rest as inputs
    DDRD = B10010000;

    // Callibrates selector
    set_selector();

    tid = 0;
    tid_last = 0;
}

void loop() {
  // Finds current row
  row_last = row;
  row = row_counter();    

  touchCounter();     // Counts tics on input pins
  
  if (countingCycleFinished){
    countingCycleFinished = false;
    key = determineWhatButtonWasPressed();

    if (( key >= 0) && (key <= 9) && (lastKey != key)){    // If input is between 0-9
      touchCode[index] = key;
      index++;

      if (index == lenCode){    // If password attempt is finished
        if (memcmp(touchCode, password, sizeof(touchCode)) == 0){
          Serial.println("CORRECT CODE");
          //newCode(password,lenCode);
        } else{
          Serial.println("WRONG CODE");
        }
        index = 0;
      }
    }
    lastKey = key;
  }
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

// Counts rows
int row_counter() {
  // Gets time
  tid = micros() - tid_last;
  
  // If time to switch
  if (tid > T) {
    tid_last = micros();
      
    if (!flag){ // LOW
      flag = true;
      PORTD = PORTD | B10000000;
      }
    else if (flag) { // HIGH
      // cnt - return value of current row
      PORTD = PORTD & B01111111;
      flag = false;
      cnt++;
      if (cnt == 4) cnt = 0;    
      flag = false;
     }
  }
  return cnt;
}

void touchCounter(){
  // Displays message if new row
  if (row != row_last) {
    //Serial.print("Current row is: ");
    //Serial.println(row);
  }
  
  // Reads input on pin 4, 2 and 3
  // Reads input on d2, d3, d4 for cycles
  // 4 = d4, 1 = d3, 0 = d2
  // PIND = PIND & B00010011;
  if (PIND & B00010000) touchMatrix[row][0]++;
  if (PIND & B00000010) touchMatrix[row][1]++;
  if (PIND & B00000001) touchMatrix[row][2]++;

  currentTime = millis();
  if(currentTime-timeMark >= sampleTime){
    timeMark = currentTime;
    for (i = 0; i <= 3; i++){
      for (k = 0; k <= 2; k++){
        if(touchMatrix[i][k] >= benchmark[i][k]){
          touch[i][k] = 1;
        } else{
          touch[i][k] = 0;
        }
      }
    }
    memset(touchMatrix, 0, sizeof(touchMatrix[0][0])*4*3); // Clears touch matrix
    countingCycleFinished = true;
  }
}

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
