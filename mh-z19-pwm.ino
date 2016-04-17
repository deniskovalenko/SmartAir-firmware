#define pwmPin 5
#define LedPin 13
//   LED   //
#define LED_R 11
#define LED_G 10
#define LED_B 9

int prevVal = LOW;
long th, tl, h, l, ppm;

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin, INPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(LED_R, OUTPUT);   
  pinMode(LED_G, OUTPUT);   
  pinMode(LED_B, OUTPUT);
}

void loop() {
   int r, g, b;
  long tt = millis();
  int myVal = digitalRead(pwmPin);

  //Если обнаружили изменение
  if (myVal == HIGH) {
    digitalWrite(LedPin, HIGH);
    if (myVal != prevVal) {
      h = tt;
      tl = h - l;
      prevVal = myVal;
    }
  }  else {
    digitalWrite(LedPin, LOW);
    if (myVal != prevVal) {
      l = tt;
      th = l - h;
      prevVal = myVal;
      ppm = 5000 * (th - 2) / (th + tl - 4);
      Serial.println("PPM = " + String(ppm));
if (ppm < 600 ) {
r = 0; g = 255; 
} else if (ppm < 900) {
      r = 200; g = 50;
} else {
     r = 255; g = 0;
}  SetColor(r, g, 0);
    }
  }
}

void SetColor(int r, int g, int b)
{
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);
  
  analogWrite(LED_R, 255-r);
  analogWrite(LED_G, 255-g);
  analogWrite(LED_B, 255-b);
}
