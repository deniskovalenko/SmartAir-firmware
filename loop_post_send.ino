#include <DHT11.h>
#include <SoftwareSerial.h>

#define SSID F("TP-Link")  //WiFi SSID
#define PASS F("DFJFbsdfh534jsfbkDFK")  //WiFi password

#define DHT_PIN 4  //DHT11 sensor data pin
#define ESP_RX_PIN 6  //to esp8266 tx pin
#define ESP_TX_PIN 7  //to esp8266 rx pin

SoftwareSerial esp(ESP_TX_PIN, ESP_RX_PIN);
DHT11 dht(DHT_PIN);

boolean _connected=false;

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(5000);
  esp.begin(9600);
  esp.setTimeout(5000);
  delay(DHT11_RETRY_DELAY);
  ////////// TODO: endless waiting for connect
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
  //////////
}

void loop()
{
  WebRequest();
  delay(15*1000);
}

void WebRequest ()
{
  //esp.flush();
  esp.println(F("AT+CIPSTART=\"TCP\",\"54.93.100.129\", 80"));
  if (esp.find("DNS Fail"))
  {
    Serial.println(F("DNS Fail"));
    return;
  }
  float temp = 0;
  float hum = 0;
  GetTempHum(temp, hum);
  String PostData=F("device_id=serialNumber123&temperature=");
  PostData+=temp;
  PostData+=F("&co2=");
  PostData+=400+600*hum/100;
  String command = F("POST http://54.93.100.129/addData HTTP/1.0\r\nHost: 54.93.100.129\r\nUser-Agent: Arduino/1.0\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded;\r\nContent-Length: ");
  command+=PostData.length();
  command+=F("\r\n\r\n");
  command+=PostData;
  Serial.println(command);
  esp.print(F("AT+CIPSEND="));
  esp.println(command.length());
  if(esp.find(">"))
  {
    Serial.println(">");
  }
  else
  {
    esp.println(F("AT+CIPCLOSE"));
    Serial.println(F("connect timeout"));
    delay(1000);
    return;
  }
  esp.print(command);
}

boolean connectWiFi()
{
  esp.println(F("AT+CWMODE=1"));
  String cmd=F("AT+CWJAP=\"");
  cmd+=SSID;
  cmd+=F("\",\"");
  cmd+=PASS;
  cmd+=F("\"");
  Serial.print(F("Connecting to "));
  Serial.println(SSID);
  esp.println(cmd);
  delay(2000);
  if(esp.find("OK"))
  {
    Serial.println(F("OK, Connected to WiFi."));
    esp.flush();
    return true;
  }
  else
  {
    Serial.println(F("error connect to the WiFi."));
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
  //delay(DHT11_RETRY_DELAY);
}
