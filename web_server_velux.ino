#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "velux_codes.h"
#include <WiFi.h>
#include <SHT21.h>  

// Replace with your network credentials
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";

//Temperature - SCL to PIN 22, SDA to PIN 21
SHT21 sht;
float temp;   // variable to store temperature
float humidity; // variable to store hemidity

//Infrared sender
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

struct Action {
  char * action_string;
  uint16_t * codes;
  char * url;
  char * css_class;
};

Action m1_up_a = {"Rollo auf", m1_up, "/c/rollo/up", "button-52"};
Action m1_stop_a = {"Rollo stop", m1_stop, "/c/rollo/stop", "button-52"};
Action m1_down_a = {"Rollo zu", m1_down, "/c/rollo/down", "button-52"};

Action m2_up_a = {"Fenster auf", m2_up, "/c/fenster/up", "button-51"};
Action m2_stop_a = {"Fenster stop", m2_stop, "/c/fenster/stop", "button-51"};
Action m2_down_a = {"Fenster zu", m2_down, "/c/fenster/down", "button-51"};

Action m3_up_a = {"Sonnenblende auf", m3_up, "/c/sonnenblende/up", "button-52"};
Action m3_stop_a = {"Sonnenblende stop", m3_stop, "/c/sonnenblende/stop", "button-52"};
Action m3_down_a = {"Sonnenblende zu", m3_down, "/c/sonnenblende/down", "button-52"};

Action all_up_a = {"Alle auf", all_up, "/c/all/up", "button-51"};
Action all_stop_a = {"Alle stop", all_stop, "/c/all/stop", "button-51"};
Action all_down_a = {"Alle zu", all_down, "/c/all/down", "button-51"};

Action aktionen[] = {
  m1_up_a, m1_stop_a, m1_down_a,
  m2_up_a, m2_stop_a, m2_down_a,
  m3_up_a, m3_stop_a, m3_down_a,
  all_up_a, all_stop_a, all_down_a
};

size_t anzahl_aktionen = sizeof(aktionen) / sizeof(aktionen[0]);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
const long timeoutTime = 2000;
String ip_address = "";

void windowAction() {
  Serial.println("Rollo up");
  irsend.sendRaw(m1_up, 96, 38);
}

void setup() {
  //Setup for IR sender
  irsend.begin();
#if ESP8266
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(115200, SERIAL_8N1);
#endif  // ESP8266

  Wire.begin();    // begin Wire(I2C)

  Serial.println("Hello World");
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  ip_address = WiFi.localIP().toString();
  Serial.println(ip_address);

  //Show yellow light for 5 seconds after wifi connected
  digitalWrite(output26, HIGH);
  delay(5 * 1000);
  digitalWrite(output26, LOW);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen

  if (client) {                             // New client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";                // incoming client data
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client is connected
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else {
              int i = 0;
              for (i = 0; i < anzahl_aktionen; i++) {
                //"GET /rollo/up"
                if (header.indexOf(aktionen[i].url) >= 0) {
                  Serial.println(aktionen[i].action_string);
                  irsend.sendRaw(aktionen[i].codes, 96, 38);
                  irsend.sendRaw(aktionen[i].codes, 96, 38); //repeat command to be sure

                  digitalWrite(output26, HIGH);
                  delay(200);
                  digitalWrite(output26, LOW);
                }
              }
            }

            if (header.indexOf("GET /c/") >= 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              break;

            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
            }



            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}");
            client.println(".button-52 {font-size: 16px;font-weight: 200;letter-spacing: 1px;padding: 13px 20px 13px;outline: 0;border: 1px solid black;cursor: pointer;position: relative;background-color: rgba(0, 0, 0, 0);user-select: none;-webkit-user-select: none;touch-action: manipulation;}.button-52:after {content: \"\";background-color: #ffe54c;width: 100%;z-index: -1;position: absolute;height: 100%;top: 7px;left: 7px;transition: 0.2s;}.button-52:hover:after {top: 0px;left: 0px;}@media (min-width: 768px) {.button-52 {padding: 13px 50px 13px;}}");
            client.println(".button-51 {background-color: transparent;border: 1px solid #266DB6;box-sizing: border-box;color: #00132C;font-family: \"Avenir Next LT W01 Bold\",sans-serif;font-size: 16px;font-weight: 700;line-height: 24px;padding: 16px 23px;position: relative;text-decoration: none;user-select: none;-webkit-user-select: none;touch-action: manipulation;}.button-51:hover, .button-51:active {outline: 0;}.button-51:hover {background-color: transparent;cursor: pointer;}.button-51:before {background-color: #D5EDF6;content: \"\";height: calc(100% + 3px);position: absolute;right: -7px;top: -9px;transition: background-color 300ms ease-in;width: 100%;z-index: -1;}.button-51:hover:before {background-color: #6DCFF6;}@media (min-width: 768px) {.button-51 {padding: 16px 32px;}}");
            client.println("</style></head>");
            client.println("<script>");
            client.println("function execAction(n){let e=new XMLHttpRequest;e.open('GET',n),e.send()}");
            client.println("</script>");
            client.println("<body><h1><a href=\"http://" + ip_address + "\" style=\"all: unset;\">Velux windows server</a></h1>");

            temp = sht.getTemperature();  // get temp from SHT
            humidity = sht.getHumidity(); // get temp from SHT

            client.print("<p><b>Temperatur: ");      // print readings
            client.print(temp);
            client.print(" \xB0""C\t Luftfeuchtigkeit: ");
            client.print(humidity);
            client.println("%</b></p>");

            int i = 0;
            for (i = 0; i < anzahl_aktionen; i++) {
              client.print("<p>");
              client.print("<button onclick=\"execAction('");
              client.print(aktionen[i].url);
              client.print("')\" class=\"");
              client.print(aktionen[i].css_class);
              client.print("\">");
              client.print(aktionen[i].action_string);
              client.print("</button></p>");
              client.println("");
            }

            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            client.println();
            break;
          } else { // if newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
