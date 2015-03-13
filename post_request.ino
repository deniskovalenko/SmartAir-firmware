char serialbuffer[1000];//serial buffer for request url
#include <SoftwareSerial.h>
SoftwareSerial esp(7, 6); // RX, TX

void setup()
{

 Serial.begin(9600);
 esp.begin(9600);
  //set mode needed for new boards
  esp.println("AT+RST");
  esp.println("AT+CWMODE=1");
  delay(500);//delay after mode change
  esp.println("AT+RST");
  
  //connect to wifi network
  esp.println("AT+CWJAP=\"HTC Denis\",\"12345678\""); //here provide valid WI-FI SSID name and password
}

void loop()
{
  //output everything from ESP8266 to the Arduino Micro Serial output
  while (esp.available() > 0) {
    Serial.write(esp.read());
  }
  
  if (Serial.available() > 0) {
     //read from serial until terminating character
     int len = Serial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));
  
     //trim buffer to length of the actual message
     String message = String(serialbuffer).substring(0,len-1);
    // Serial.println("message: " + message);
 
     //check to see if the incoming serial message is a url or an AT command
     if(message.substring(0,2)=="AT"){
       //make command request
      // Serial.println("COMMAND REQUEST");
       esp.println(message); 
     }else{
      //make webrequest
     //  Serial.println("WEB REQUEST");
       WebRequest(message);
     }
  }
}

//web request needs to be sent without the http for now, https still needs some working
void WebRequest(String request){ //www.google.com
//     String domain = "54.93.100.129";
//     String path = "/addData";
     
     //output domain and path to verify
    // Serial.println("domain: |" + domain + "|"); //google.com
    // Serial.println("path: |" + path + "|");     //path
     
     //create start command
     String startcommand = "AT+CIPSTART=\"TCP\",\"54.93.100.129\", 80"; //443 is HTTPS, still to do
     
     esp.println(startcommand);
     Serial.println(startcommand);
     
     
     //test for a start error
     if(esp.find("Error")){
       Serial.println("error on start");
       return;
     }
     
     //create the request command
      String sendcommand = "POST /addData HTTP/1.1";//do we need domain??? AND WTF /n/n/n/n/n
     //String sendcommand = "GET http://"+ domain + " HTTP/1.0\r\n\r\n\r\n";//works for most cases
     
     Serial.println(sendcommand);
     esp.print("AT+CIPSEND=");
     esp.println(sendcommand.length()); // length of post or post + params>??   
      
     //debug the command
     Serial.print("AT+CIPSEND=");
     Serial.println(sendcommand.length());
     
     delay(2000);// wtf, maybe set much more?
     if(esp.find(">"))
     {
       Serial.println(">");
     }else
     {
       esp.println("AT+CIPCLOSE");
       Serial.println("connect timeout");
       delay(1000);
       return;
     }
     
     //Serial.print(getcommand);
     String host = "Host: 54.93.100.129";
     String form = "Content-Type: application/x-www-form-urlencoded";
     String _length = "Content-Length: ";
     String postData = "device_name=arduino_postrequest&temperature=33&co2=800";
     
     //send 
     esp.println(sendcommand);  //print or println??
     esp.println(host);
     esp.println(form);
     esp.println("Connection: close");
     esp.print(_length);
     esp.println(postData.length());
     esp.println();
     esp.println(postData);

}
