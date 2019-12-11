/* * Port 80 by default
 * 
 * You just need to connect to ESP32 Access Point
 * password = "your-password"
 * 
 * Then, you can reprogram ESP32 via IP at Arduino IDE
 * 
 * SENDING COMMNADS
 * To send commands to Node via Browser, just type:
 * 192.168.4.1/ligaled   and wait for the reply
 * 192.168.4.1/motor?speed=25     sends number 25
 * 
 * To send commands to Node via Terminal, just type:
 * curl 192.168.4.1/ligaled   and wait for the reply
 * curl 192.168.4.1/motor?speed=25     sends number 25
 * 
 * To send more than one argument:
 * 192.168.4.1/motor?speed1=1023&speed2=1023&dir1=1&dir2=0
 */


#define LED_BUILTIN 2
#include <ArduinoOTA.h>   // Library to allow programing via WIFI (Choose ip port in Arduino IDE
#include <WiFi.h>        // Include the Wi-Fi library
#include <WebServer.h>
#include <analogWrite.h>  // Aparently, you can analogRead, but not analogWrite without a library with ESP32 !!!

#define speedM1 12
#define dirM1 13

#define speedM2 14
#define dirM2 27

#define speed 60


//Para os  dirs 1 é frente 0 é tras
int dir1 = 0;
int dir2 = 0;

const char *ssid = "TELESCOPIO"; // The name of the Wi-Fi network that will be created
const char *password = "telescopio";   // The password required to connect to it, leave blank for an open

WebServer server(80);


void handleRoot() {
  server.send(200, "text/plain", "Conectado");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void motor() {
  if(server.hasArg("rotate") && server.hasArg("dir1")) {
    Serial.println("ENTROU EM ROTATE: ");
    int rotate = server.arg("rotate").toInt();
    Serial.println(rotate);
    int dir1 = server.arg("dir1").toInt();
    
    if(dir1 == 1){
      Serial.println("RECEBEU DIR1 == 1 !!!");
      analogWrite(dirM1,LOW);
      analogWrite(speedM1, speed);
    } 
    else if(dir1 == 0){
      Serial.println("RECEBEU DIR1 == 0!!!");
      analogWrite(dirM1, speed);
      analogWrite(speedM1,LOW);
    }
      
    delay(rotate);
    analogWrite(dirM1, 0);
    analogWrite(speedM1, 0);
  }
  
  if(server.hasArg("ascencion") && server.hasArg("dir2")) {
    Serial.println("ENTROU EM ASCENCION: !!!");
    int ascencion = server.arg("ascencion").toInt();
    Serial.println(ascencion);
    int dir2 = server.arg("dir2").toInt();
    
    if(dir2 == 0){
     Serial.println("RECEBEU DIR2 == 0 !!!");
     analogWrite(dirM2,LOW);
     analogWrite(speedM2, speed);
    } 
    else if(dir2 == 1){
      Serial.println("RECEBEU DIR2 == 1 !!!");
      analogWrite(dirM2, speed);
      analogWrite(speedM2, LOW);
    }
    
    delay(ascencion);
    analogWrite(speedM2, 0);
    analogWrite(dirM2, 0);
  }
    
    
  server.send(200, "text/plain", "motor");

}

void setup() {
  //LED
  pinMode(LED_BUILTIN, OUTPUT);

  //PWM
  pinMode(speedM1, OUTPUT);     // Initialize the Motor PWM pin as an output
  pinMode(speedM2, OUTPUT);
  pinMode(dirM1, OUTPUT);
//  pinMode(m1CCW, OUTPUT);
  pinMode(dirM2, OUTPUT);
//  pinMode(m2CCW, OUTPUT);

  analogWriteFrequency(200);  

 
  // Serial Begin
  Serial.begin(115200);
  Serial.println("Booting");

  // WIFI Server AP (This Create a WIFI Server in ESP32 with IP: 192.168.4.1  by default) 
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP32 to the computer

  server.on("/", handleRoot);  
  server.on("/motor", HTTP_GET, motor);
  server.on("/para", HTTP_GET, para);

  server.onNotFound(handleNotFound);

  server.on("/ligaled", []() {
      server.send(200, "text/plain", "ligou");
      digitalWrite(LED_BUILTIN, HIGH);
      });

  server.on("/desligaled", []() {
      server.send(200, "text/plain", "apagou");
      digitalWrite(LED_BUILTIN, LOW);
      });

  server.begin();
  Serial.println("HTTP server started");
  //END OF WIFI Server AP

  //BEGIN OF OTA SETUP
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
      } else { 
      type = "filesystem";
      }

      Serial.println("Start updating " + type);
      });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
      });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      });
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
      }
      });
  ArduinoOTA.begin();
  Serial.println("Ready");
  //  Serial.print("IP address: ");
  //  Serial.println(WiFi.localIP());

  // END OF OTA SETUP
}

void loop() {
  ArduinoOTA.handle();

  server.handleClient();

  delay(100);
}
