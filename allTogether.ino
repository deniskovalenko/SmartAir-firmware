#include <SoftwareSerial.h>
#include <dht11.h>

//   LED   //
#define LED_R 11
#define LED_G 10
#define LED_B 9

#define NORM_PPM 1000 
#define MAX_PPM 10000

double RED_a = 255.0 / (MAX_PPM-NORM_PPM);
double RED_b = -RED_a * NORM_PPM;
double GREEN_a = 255.0 / (NORM_PPM - MAX_PPM);
double GREEN_b = -GREEN_a * MAX_PPM;
//   LED   //


//   DHT   //
#define DHT_DATAPIN 2

dht11 DHT;
//   DHT   //

//   ESP   //
#define ESP_RX_PIN 6  
#define ESP_TX_PIN 7  

#define WiFi_SSID "school" 
#define WiFi_PASS "4T7CkjYD"  

SoftwareSerial esp(ESP_TX_PIN, ESP_RX_PIN);

boolean _connected = false;
//   ESP   //

//   MG   //
#define         MG_PIN                       (0)
#define         DC_GAIN                      (8.5)

#define         READ_SAMPLE_INTERVAL         (50)
#define         READ_SAMPLE_TIMES            (10) 
                                                    
#define         ZERO_POINT_VOLTAGE           (0.381) //at least 0.4, maybe even more
#define         REACTION_VOLTGAE             (0.030)

float           CO2Curve[3]  =  {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE/(2.602-3))};   
//   MG   //


int _i = 0;

void setup()
{
  pinMode(LED_R, OUTPUT);   
  pinMode(LED_G, OUTPUT);   
  pinMode(LED_B, OUTPUT);
  
  Serial.begin(9600);
  Serial.setTimeout(5000);
  
  esp.begin(9600);
  esp.setTimeout(5000);
  
  esp.println("AT+RST");
  delay(1000);
  
  while(!_connected)
    _connected = connectWiFi(); //maybe, revert to for 1 .. 5, and if not connected start storing data to memory
}
long co2Sum;
int count;
void loop()
{
  int temp;
  
  int hum;
  int mgPercentage;
  float mgVolts;
  int r, g, b;

  GetDHTData(temp, hum);
  mgVolts = MGRead(MG_PIN);  
  mgPercentage = MGGetPercentage(mgVolts, CO2Curve);
  
  co2Sum+=mgPercentage;
  count++;
  
  if (mgPercentage < 700 ) {

    r = 0; g = 255; } else

    if (mgPercentage < 1100) {

      r = 200; g = 50;} else {

        r = 255; g = 0;

      }
  //r = (int)(RED_a * (double)mgPercentage + RED_b);

  //g = (int)(GREEN_a * (double)mgPercentage + GREEN_b);
     
  SetColor(r, g, 0);
  
  Serial.print("Humidity : ");
  Serial.print(hum);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C       Voltage: ");
  Serial.print(mgVolts);
  Serial.print(" V       CO2: ");
  Serial.print(mgPercentage);
  Serial.println(" ppm");
  
  if (_i == 19)
  {
    int co2Avg = co2Sum/count;
    WebRequest(temp, co2Avg, hum);
    _i = -1;
    count=0;
    co2Sum=0;
  }
  
  _i++;
  delay(3000);
}

void WebRequest (int t, int p, int h)
{
  esp.println("AT+CIPSTART=\"TCP\",\"smart-air.herokuapp.com\",80");
  delay(1000);
  
  if (esp.find("DNS Fail"))
  {
    Serial.println("DNS Fail");
    return;
  }
 
  String PostData="deviceId=40665d90-268d-4857-90fa-21b4016a52d3&temperature=";
  PostData+=t;
  PostData+="&humidity=";
  PostData+=h;
  PostData+="&co2=";
  PostData+=p;
  
  String command = "POST http://smart-air.herokuapp.com/api/addData HTTP/1.0\r\nHost: smart-air.herokuapp.com\r\nUser-Agent: Arduino/1.0\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded;\r\nContent-Length: ";
  command+=PostData.length();
  command+="\r\n\r\n";
  command+=PostData;
  
  Serial.println(command);
  
  esp.print("AT+CIPSEND=");
  esp.println(command.length());
  delay(1000);
  
  if(esp.find(">"))
  {
    Serial.println("OK");
    esp.print(command);
    delay(1000);
    esp.println("AT+CIPCLOSE");
  }
  else
  {
    esp.println("AT+CIPCLOSE");
    Serial.println("connect timeout");
    return;
  }
}

boolean connectWiFi()
{
  esp.println("AT+CWMODE=1");
  delay(1000);
  
  String cmd="AT+CWJAP=\"";
  cmd+=WiFi_SSID;
  cmd+="\",\"";
  cmd+=WiFi_PASS;
  cmd+="\"";
  
  Serial.print("Connecting to ");
  Serial.println(WiFi_SSID);
  
  esp.println(cmd);
  delay(1000);
  
  if(esp.find("OK"))
  {
    Serial.println("OK, Connected to WiFi.");
    esp.flush();
    return true;
  }
  else
  {
    Serial.println("Error connect to the WiFi.");
    return false;
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

void GetDHTData(int& t, int& h)
{
  DHT.read(DHT_DATAPIN);
  
  t = DHT.temperature;
  h = DHT.humidity;
}

float MGRead(int mg_pin)
{
    int i;
    float v = 0;

    for (i = 0; i < READ_SAMPLE_TIMES; i++) 
    {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) * 5 / 1024 ;
    return v;  
}

int  MGGetPercentage(float volts, float *pcurve)
{
  int result;
  
   if ((volts/DC_GAIN ) >= ZERO_POINT_VOLTAGE)
   {
      result = 400;
   } 
   else 
   { 
      result = pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
   
   if ((result > 10000) || (result < 400))
     result = 10000;
     
   return (result);
}
