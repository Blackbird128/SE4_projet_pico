// Test des 5 leds sur le shield 

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A3, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(1, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(A0, HIGH);
  digitalWrite(A3, HIGH);
  delay(1000);              
  digitalWrite(1, LOW);
  digitalWrite(4, LOW); 
  digitalWrite(7, LOW);  
  digitalWrite(A0, LOW);    
  digitalWrite(A3, LOW);  
  delay(1000);
}
