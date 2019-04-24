bool flag = 0;              // Switch between HIGH and LOW
long frekvens = 1;          // Wanted frequency
long T = 10; // Period of the frequency
long tid = 0;               // Real time value
long tid_last = 0;          // Time
int cnt = -1;               // Row-counter
int row = 0;                // yes
int row_last = 0;           // yes-last

int counters[3] = {0, 0, 0}; // Array of counted cycles

void setup() {  
  Serial.begin(9600);

  // Sets pin 4 and 7 as outputs - rest as inputs
  DDRD = B10010000;

  // Callibrates selector
  set_selector();

  // Need reference counter
  // ref = counter_reference()

  tid = 0;
  tid_last = 0;
}

void loop() {
  // Finds current row
  row_last = row;
  row = row_counter();

  // Displays message if new row
  if (row != row_last) {
    Serial.print("Current row is: ");
    Serial.println(row);
    }

  // Reads input on d2, d3, d4 for cycles
  // 4 = d4, 1 = d3, 0 = d2
  // PIND = PIND & B00010011;
  if (PIND & B00010000) counters[0]++;
  if (PIND & B00000010) counters[1]++;
  if (PIND & B00000001) counters[2]++;

  Serial.println(counters[0]);
  Serial.println(counters[1]);
  Serial.println(counters[2]);
}


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
        cnt++;
        if (cnt == 4) cnt = 0;
        
        flag = false;
        PORTD = PORTD & B01111111;
        }
      }

      return cnt;
    }
