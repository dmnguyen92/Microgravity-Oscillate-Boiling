// Demo Code for SerialCommand Library
// Steven Cogswell
// May 2011

#include <SoftwareSerial.h>   // We need this even if we're not using a SoftwareSerial object
                              // Due to the way the Arduino IDE compiles
#include <SerialCommand.h>



#define arduinoLED    13  // Arduino LED on board
#define exposureINPUT  3  // input from camera (exposure signal) as interupt, must be 2 or 3 on Arduino Uno
#define flashOUTPUT    4  // output to send a flash
#define cameraOUTPUT   5  // trigger the camera


SerialCommand SCmd;   // The demo SerialCommand object

int Gflash_delay = 1000; //Delay of the flash in us
int Gflash_duration = 10000; //Duration of the flash in us

void setup()
{  
  pinMode(arduinoLED,OUTPUT);      // Configure the onboard LED for output
  digitalWrite(arduinoLED,LOW);    // default to LED off

  pinMode(exposureINPUT, INPUT_PULLUP); // trigger input pin from camera
  pinMode(flashOUTPUT, OUTPUT);
  pinMode(cameraOUTPUT, OUTPUT);
  
  Serial.begin(9600); 

  // Setup callbacks for SerialCommand commands 
  SCmd.addCommand("ON",LED_on);       // Turns LED on
  SCmd.addCommand("OFF",LED_off);        // Turns LED off
  SCmd.addCommand("HELLO",SayHello);     // Echos the string argument back
  SCmd.addCommand("P",process_command);  // Converts two arguments to integers and echos them back 
  SCmd.addCommand("T",trigger_camera);
  SCmd.addCommand("F:ON", flash_on);
  SCmd.addCommand("F:OFF", flash_off);
//  SCmd.addCommand("F:TIME", flash_time);
//  SCmd.addCommand("F:DELAY", flash_delay);
  SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched  (says "What?") 
  Serial.println("Ready"); 
}

void loop()
{  
  SCmd.readSerial();     // We don't do much, just process serial commands
}

void trigger_camera(){
  Serial.println("Trigger Camera");
  digitalWrite(cameraOUTPUT, HIGH);
  delayMicroseconds(20);
  digitalWrite(cameraOUTPUT, LOW);
}
  

void flash(){
  delayMicroseconds(Gflash_delay);
  digitalWrite(arduinoLED,HIGH);
  delayMicroseconds(Gflash_duration);
  digitalWrite(arduinoLED,LOW);
}

void flash_on()
{
  Serial.println("Flash on");
  attachInterrupt(digitalPinToInterrupt(exposureINPUT), flash, RISING);
}

void flash_off()
{
  Serial.println("Flash off");
  detachInterrupt(digitalPinToInterrupt(exposureINPUT));
}



void LED_on()
{
  Serial.println("LED on"); 
  digitalWrite(arduinoLED,HIGH);  
}

void LED_off()
{
  Serial.println("LED off"); 
  digitalWrite(arduinoLED,LOW);
}

void SayHello()
{
  char *arg;  
  arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    Serial.print("Hello "); 
    Serial.println(arg); 
  } 
  else {
    Serial.println("Hello, whoever you are"); 
  }
}


void process_command()    
{
  int aNumber;  
  char *arg; 

  Serial.println("We're in process_command"); 
  arg = SCmd.next(); 
  if (arg != NULL) 
  {
    aNumber=atoi(arg);    // Converts a char string to an integer
    Serial.print("First argument was: "); 
    Serial.println(aNumber); 
  } 
  else {
    Serial.println("No arguments"); 
  }

  arg = SCmd.next(); 
  if (arg != NULL) 
  {
    aNumber=atol(arg); 
    Serial.print("Second argument was: "); 
    Serial.println(aNumber); 
  } 
  else {
    Serial.println("No second argument"); 
  }

}

// This gets set as the default handler, and gets called when no other command matches. 
void unrecognized()
{
  Serial.println("Command not recognized");
}

