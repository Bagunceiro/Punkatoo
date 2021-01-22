#include <Arduino.h>
#include "spdt.h"

SPDT::SPDT()
{
}

void SPDT::setPins(int t1, int t2)  // GPIO pins driving the two throws
{
  throw1 = t1;
  throw2 = t2;
  pinMode(throw1, OUTPUT);
  pinMode(throw2, OUTPUT);
  // pos(0);
}

void SPDT::pos(const int p) {
  switch (p)
  {
    case 1:
      digitalWrite(throw2, HIGH);
      digitalWrite(throw1, LOW);
      break;
    case 2:
      digitalWrite(throw1, HIGH);
      digitalWrite(throw2, LOW);
      break;
    case 3:
      digitalWrite(throw1, LOW);
      digitalWrite(throw2, LOW);
      break;
    case 0:  // off
    default: // something has gone wrong, turn it off anyway for safety
      digitalWrite(throw1, HIGH);
      digitalWrite(throw2, HIGH);
  }
}

int SPDT::stat() {
  return ((digitalRead(throw2) == LOW ? 2 : 0) | (digitalRead(throw1) == LOW ? 1 : 0));
}
