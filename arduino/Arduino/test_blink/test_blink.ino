/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {          
  Serial.begin(9600);  
   printFreeMemory("0"); 
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);    
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}
// Free Memory request
extern unsigned long __bss_end;
extern void *__brkval;

// Free memory request
int memoryFree()
{
  //  long myValue;
  int freeValue;
  freeValue = 0;

  if ((unsigned long)__brkval == 0)
  { 
    freeValue = ((unsigned long)&freeValue) - ((unsigned long)&__bss_end);
    //       Serial.println("here and there");
  }
  else
  { 
    freeValue = ((unsigned long)&freeValue) - ((unsigned long)__brkval);
    //      Serial.println("here  2");
  }

  return freeValue;

}//end memoryFree()

void printFreeMemory(String message) {
  Serial.println("  ");  
  Serial.print("Free Memory:  ");
  Serial.print(message); 
  Serial.print(" ");
  Serial.print(memoryFree()); 
  Serial.println("  ");  
}
