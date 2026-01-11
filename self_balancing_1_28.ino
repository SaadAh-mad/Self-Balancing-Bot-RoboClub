#define DIR_PIN 3
#define STEP_PIN 2
#define ENA_PIN 4  // optional
#define DIR_PIN1 5
#define STEP_PIN1 6
#define ENA_PIN1 7 


void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(DIR_PIN1, OUTPUT);
  pinMode(STEP_PIN1, OUTPUT);
  pinMode(ENA_PIN1, OUTPUT);
  digitalWrite(ENA_PIN1, LOW); // Enable driver (LOW = active)
}

void loop() {
    digitalWrite(DIR_PIN, HIGH);
    digitalWrite(DIR_PIN1, HIGH);

    // Clockwise
     for (int i = 0; i < 3200; i++) {
     digitalWrite(STEP_PIN, HIGH);
     delayMicroseconds(800);
     digitalWrite(STEP_PIN, LOW);
     delayMicroseconds(800);
    digitalWrite(STEP_PIN1, HIGH);
    delayMicroseconds(800);
    digitalWrite(STEP_PIN1, LOW);
    delayMicroseconds(800);
  }
  delay(1000);
   }
    
 
 // digitalWrite(DIR_PIN1, HIGH);
  //for (int i = 0; i < 3200; i++) {
   // digitalWrite(STEP_PIN1, HIGH);
    //delayMicroseconds(800);
    //digitalWrite(STEP_PIN1, LOW);
    //delayMicroseconds(800);
 // }
  //delay(1000);
 
//}