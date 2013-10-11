/*
* This program displays the progression of the Sieve of 
* Eratosthenes on a 64x64 LED display driven by a MAX7219 serial
* common-cathode 7-segment 8-digit LED display driver.
*
* This progression starts with all 64 LEDs lit, then will 
* algorithmically disable all non-primes.
*
* The algorithm is as follows:
*   For an array A of booleans, indexed from 1 to 64, initialized
*     to true;
*   for i in 2...64
*     if A[i] == true
*       for j in i^2, 1^2 + i, i^2 + 2i,...n
*         A[j] <- false
*   Now all i where A[i] == true are prime
*
*/

// Pins

int clkPin = 6;
int chipSelectPin = 7;
int dinPin = 8;

// Prime array

byte primeRows[8];
byte writeRows[8];

///////////////////////////////////////////////

void setup() {
  pinMode(clkPin, OUTPUT);
  pinMode(chipSelectPin, OUTPUT);
  pinMode(dinPin, OUTPUT);
  delay(50);
  
  initMax7219();
}

void loop() { 
  resetPrimes();
  writeLEDs();
  delay(2000);
  for (int i = 0;i < 8;i++) {
    for (int j = 0;j < 8;j++) {
      //We ignore primeRows[0]@0, because that's 1 and not a prime
      //in the algorithm.
      if (!(0 == i && 0 == j)) {
        //Otherwise, if the number in question is high/1, remove
        //all of its multiples, starting at number^2
        if (1 == bitRead(primeRows[i], j)) {
          int numeral = (i*8) + j + 1; //the actual number in question
          int inc = 0; //factor by which to increase the number
          int k = numeral * numeral; //start at number^2, by the algorithm
          while (k <= 64) {
            int row = (k-1) / 8;
            //Bytes are sent MSB - LSB and received LSB - MSB,
            //so reverse the byte (which is the 7-x) for the row
            //to write out.  (Keep the row for calculations in order;
            //Two arrays means less chance of screwing up.)
            //Otherwise, k = 8*row + col, so col = k - 8*row, then
            //col = col - 1 to address 0-based numbering 
            bitClear(writeRows[row], (7 -(k - (8 * row) - 1)));
            bitClear(primeRows[row], (k - (8 * row) - 1));                     

            inc++; 
            k = (numeral * numeral) + (inc * numeral);
            writeLEDs();
            delay(1000);
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////

void writeLEDs() {
  for (int i=0;i < 8;i++) {
    writeMax7219(i+1, writeRows[i]);
  }
}

void resetPrimes() {
  for (int i = 0;i < 8;i++) {
    primeRows[i] = 255;
    writeRows[i] = 255;
  }
}

///////////////////////////////////////////////

void writeMax7219Byte(unsigned char data) {
  unsigned char i;
  digitalWrite(chipSelectPin, LOW);
  for (int i=8;i > 0;i--) {
      digitalWrite(clkPin, LOW);
      digitalWrite(dinPin, data&0x80);
      data = data << 1;
      digitalWrite(clkPin, HIGH);
  }
}

void writeMax7219(unsigned char addr, unsigned char data) {
  digitalWrite(chipSelectPin, LOW);
  writeMax7219Byte(addr);
  writeMax7219Byte(data);
  digitalWrite(chipSelectPin, HIGH);
}

void initMax7219() {
  writeMax7219(0x09, 0x00);       //set decoding：no-decode
  writeMax7219(0x0a, 0x03);       //set brightness 
  writeMax7219(0x0b, 0x07);       //set scanlimit；full LEDs (all 8)
  writeMax7219(0x0c, 0x01);       //set power-down mode：0，normal mode：1
  writeMax7219(0x0f, 0x00);       //set test display：1；EOT，display：0
}

///////////////////////////////////////////////
