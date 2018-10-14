void setup() {
  // put your setup code here, to run once:

// initialize digital pin 13 as an output.
//  pinMode(13, OUTPUT);
 pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an outputs
}

void loop() {
//  // put your main code here, to run repeatedly:
//  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);              // wait for a second
//  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);              // wait for a second

digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(2000);                      // Wait for two seconds (to demonstrate the active low LED)
 
}


