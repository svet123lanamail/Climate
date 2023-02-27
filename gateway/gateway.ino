#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <stdio.h>
#include "memory.h"

#define DEBUG(a, b) \
  Serial.print(a); Serial.println(b);

FlashMemory mymem;

/* Установите здесь свои SSID и пароль */
const char* ssid = "greenhouse";       // SSID
const char* password = "greenhouse";  // пароль

/* Настройки IP адреса */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

void setup()
{
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handleIndex);
  server.on("/meas", handleMeas);
  server.on("/measurements.xml", handleMeasXhr);
  server.on("/getMeasurements.xml", handleGetMeas);
  server.on("/start", handleStart);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}

void handleMeasXhr() {
  char buf[320] = { 0 };
  char newBuf[320] = { 0 };
  mymem.getFile("tableXml", buf);
  float temp, hum;
  bool water = false;
  calcMeas(temp, hum);
  sprintf(newBuf, buf, String(temp, 2).c_str(), String(hum, 2).c_str(), water ? "Water" : "No water");
  Serial.println(buf);
  Serial.println(newBuf);
  server.send(200, "application/xml", newBuf);
}

void calcMeas(float &temp, float &hum) {
  int cnt = 0;
  stateRecord* arr = mymem.selectStateRecord("temp", cnt);
  DEBUG("count temp records for calc: ", cnt);
  if (cnt) {
    int maxInd1 = 1, maxInd2 = 0;
    for (int i = 2; i < cnt; i++) {
      if ((String(arr[i].time).toInt() > String(arr[maxInd1].time).toInt())) {
        maxInd2 = maxInd1;
        maxInd1 = i;
      }
    }
    temp = (String(arr[maxInd1].value).toFloat() + String(arr[maxInd2].value).toFloat()) / 2;
    DEBUG("calculated temp: ", String(temp, 2));
    if (arr[maxInd1].stationId == arr[maxInd2].stationId) { // 1 из датчиков вышел из строя
      Serial.println("Error: only 1 station sends temp!");
    }

    delete[] arr;
  }
  else temp = -273;
  
  arr = mymem.selectStateRecord("hum", cnt);
  DEBUG("count hum records for calc: ", cnt);
  if (cnt) {
    int maxInd1 = 1, maxInd2 = 0;
    for (int i = 2; i < cnt; i++) {
      if ((String(arr[i].time).toInt() > String(arr[maxInd1].time).toInt())) {
        maxInd2 = maxInd1;
        maxInd1 = i;
      }
    }
    hum = (String(arr[maxInd1].value).toFloat() + String(arr[maxInd2].value).toFloat()) / 2;
    DEBUG("calculated hum: ", String(hum, 2));
    if (arr[maxInd1].stationId == arr[maxInd2].stationId) { // 1 из датчиков вышел из строя
      Serial.println("Error: only 1 station sends hum!");
    }

    delete[] arr;
  }
  else hum = -1;
}

void handleIndex() {
  Serial.println("New user on Index");
  server.send(200, "text/html", mymem.getPage("index"));
}

void handleStart() {
  String message = "{\"T\": ";
  message += String(12);
  message += "}";
  server.send(200, "application/json", message);
}

void handleMeas() { // /meas
  String message = "Number of args received: ";
  message += server.args();            //Get number of parameters
  message += "\n";                            //Add a new line

  stateRecord n;
  for (int i = 0; i < server.args(); i++) {
    message += "Arg No." + (String)i + " – > ";  //Include the current iteration value
    message += server.argName(i) + ": ";     //Get the name of the parameter
    message += server.arg(i) + "\n";              //Get the value of the parameter

    if (server.argName(i).equalsIgnoreCase("stationId")) n.stationId = server.arg(i).toInt();
    else if (server.argName(i).equalsIgnoreCase("type")) strcpy(n.type, server.arg(i).c_str());
    else if (server.argName(i).equalsIgnoreCase("value")) strcpy(n.value, server.arg(i).c_str());
    n.time = millis() / 1000;
  }
  DEBUG("type incoming: ", n.type);
  mymem.insertStateRecord(n);

  server.send(200, "text/plain", message);     //Response to the HTTP request
}

void handleGetMeas() { // /getMeasurements
  Serial.println("getMeasurements");
  int cnt = 0;
  stateRecord* arr = mymem.selectStateRecord("", cnt);
  DEBUG("count found records: ", cnt);
  String message = "<tbody>";
  if (!cnt) message = "No data";

  for (int i = 0; i < cnt; i++) {
    message += "<tr><td>";
    message += arr[i].stationId;
    message += "</td><td>";
    message += String(arr[i].time).toInt();
    message += "</td><td>";
    message += arr[i].type;
    message += "</td><td>";
    message += arr[i].value;
    message += "</td></tr>";
  }
  message += "</tbody>";

  if (cnt) delete[] arr;
  server.send(200, "application/xml", message);
}

void handle_OnConnect()
{
  Serial.print("GPIO7 Status: ");

  Serial.println("");
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
