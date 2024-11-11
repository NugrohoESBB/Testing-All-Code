#include <Keypad.h>
 
#define ROWS  4
#define COLS  4
 
char keyMap[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
 
uint8_t rowPins[ROWS] = {14, 27, 26, 25}; // GIOP14, GIOP27, GIOP26, GIOP25
uint8_t colPins[COLS] = {33, 32, 18, 19}; // GIOP33, GIOP32, GIOP18, GIOP19
 
Keypad keypad4x4 = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS );
 
void setup(){
  Serial.begin(115200);
}
 
void loop(){
  
  char key = keypad4x4.getKey();
  if (key) {
    Serial.println(key);
  }
}
