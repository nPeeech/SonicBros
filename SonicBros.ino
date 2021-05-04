#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <Servo.h>
#include "MYSSID.h"
#include "index_html.h"
#define SRVPIN 14
#define ECHOPIN 12
#define TRIGPIN 13
#define RIGHTMOTER1PIN 16
#define LEFTMOTER1PIN 5
#define MOTER2PIN 4

Servo myservo;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void move_forward() {
  Serial.printf("mae ni susumu\n");
  digitalWrite(RIGHTMOTER1PIN, HIGH);
  digitalWrite(LEFTMOTER1PIN, HIGH);
  digitalWrite(MOTER2PIN, LOW);
}

void move_backward() {
  Serial.printf("ushiro ni susumu\n");
  digitalWrite(RIGHTMOTER1PIN, LOW);
  digitalWrite(LEFTMOTER1PIN, LOW);
  digitalWrite(MOTER2PIN, HIGH);
}

void turn_right() {
  Serial.printf("migi ni mawaru\n");
  digitalWrite(RIGHTMOTER1PIN, LOW);
  digitalWrite(LEFTMOTER1PIN, HIGH);
  digitalWrite(MOTER2PIN, LOW);
}

void turn_left() {
  Serial.printf("hidari ni mawaru\n");
  digitalWrite(RIGHTMOTER1PIN, HIGH);
  digitalWrite(LEFTMOTER1PIN, LOW);
  digitalWrite(MOTER2PIN, LOW);
}
void wheel_stop() {
  Serial.printf("tomaru\n");
  digitalWrite(RIGHTMOTER1PIN, HIGH);
  digitalWrite(LEFTMOTER1PIN, HIGH);
  digitalWrite(MOTER2PIN, HIGH);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      if (strcmp((const char*)payload, "move-forward") == 0) move_forward();
      else if (strcmp((const char*)payload, "move-backward") == 0) move_backward();
      else if (strcmp((const char*)payload, "turn-right") == 0) turn_right();
      else if (strcmp((const char*)payload, "turn-left") == 0) turn_left();
      else if (strcmp((const char*)payload, "stop") == 0) wheel_stop();
      else Serial.printf("[%u] get Text: %s\n", num, payload);
      break;
  }

}

int measure_distance() {
  double duration = 0;
  double distance = 0; //みりめーとる
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH); //超音波照射
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH, 30000);
  if (duration > 0) {
    duration /= 2;
    distance = duration * 340 / 1000;
    if (distance < 2000) return (int)distance;
  }
  return -1;
}

void setup() {
  //Serial.begin(921600);
  Serial.begin(115200);

  //Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(MYSSID_ssid, MYSSID_pass);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(MYSSID_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // handle index
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", html_page);
  });

  server.begin();
  Serial.println("HTTP server started");

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  myservo.attach(SRVPIN);
  myservo.write(0);
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  pinMode(RIGHTMOTER1PIN, OUTPUT);
  pinMode(LEFTMOTER1PIN, OUTPUT);
  pinMode(MOTER2PIN, OUTPUT);
  digitalWrite(RIGHTMOTER1PIN, HIGH);
  digitalWrite(LEFTMOTER1PIN, HIGH);
  digitalWrite(MOTER2PIN, HIGH);
  delay(1000);
}

unsigned long last_10sec = 0;
unsigned int counter = 0;
int deg = 0;
int sign = 1;
int inc = 10;

void loop() {
  unsigned long t = millis();
  webSocket.loop();
  server.handleClient();
  myservo.write(deg);
  delay(80);
  int dist = measure_distance();

  JSONVar json_obj;
  json_obj["deg"] = deg;
  json_obj["dist"] = dist;
  String json_str = JSON.stringify(json_obj);
  webSocket.broadcastTXT((const char *)json_str.c_str());
  //Serial.println(json_obj);
//  delay(20);

  if (deg + inc > 180) sign = -1;
  else if (deg - inc < 0) sign = 1;
  deg += sign * inc;

  if ((t - last_10sec) > 10 * 1000) {
    counter++;
    bool ping = (counter % 2);
    int i = webSocket.connectedClients(ping);
    Serial.printf("%d Connected websocket clients ping: %d\n", i, ping);
    last_10sec = millis();
  }
}
