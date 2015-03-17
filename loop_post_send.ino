#include <DHT11.h>
#include <SoftwareSerial.h>

#define SSID "HTC Denis"  //WiFi SSID
#define PASS "12345678"  //WiFi password

SoftwareSerial esp(7, 6);
DHT11 dht(4);

char serialbuffer[100];
String content = "";
char character;
boolean _connected=false;

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(5000);
  esp.begin(9600);
  esp.setTimeout(5000);
  delay(DHT11_RETRY_DELAY);
  for(int i=0;i<5;i++)
  {
    if(connectWiFi())
    {
      _connected = true;
      break;
    }
  }
  if (!_connected){
    while(1);
  }
  delay(1000);
}

void loop()
{
  delay(15*1000);
  WebRequest();
}

void WebRequest ()
{
  esp.flush();
  esp.println("AT+CIPSTART=\"TCP\",\"54.93.100.129\", 80");
  if (esp.find("DNS Fail"))
  {
    Serial.println("DNS Fail");
    return;
  }
  float temp = 0;
  float hum = 0;
  GetTempHum(temp, hum);
  String PostData="device_name=Arduino&temperature=";
  PostData+=temp;
  PostData+="&co2=";
  PostData+=400+600*hum/100;
  String command = "POST http://54.93.100.129/addData HTTP/1.0\r\nHost: 54.93.100.129\r\nUser-Agent: Arduino/1.0\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded;\r\nContent-Length: ";
  command+=PostData.length();//"Connection: close\r\n";
  command+="\r\n\r\n";
  command+=PostData;
  Serial.println(command);
  esp.print("AT+CIPSEND=");
  esp.println(command.length());
  if(esp.find(">"))
  {
    Serial.println(">");
  }
  else
  {
    esp.println("AT+CIPCLOSE");
    Serial.println("connect timeout");
    delay(1000);
    return;
  }
  esp.print(command);
}

boolean connectWiFi()
{
  esp.println("AT+CWMODE=1");
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.print("Connecting to ");
  Serial.println(SSID);
  esp.println(cmd);
  delay(2000);
  if(esp.find("OK"))
  {
    Serial.println("OK, Connected to WiFi.");
    esp.flush();
    return true;
  }
  else
  {
    Serial.println("error connect to the WiFi.");
    return false;
  }
}

float GetTempHum( float& temp, float& hum)
{
  int err;
  if((err=dht.read(hum, temp))!=0)
  {
    hum=0;
    temp=-273;
  }
  delay(DHT11_RETRY_DELAY);
}