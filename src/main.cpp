
#include "LedControl.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "RenderText.h"
#include "RenderAnim.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//ipc interface leds array
#define DIN 12
#define CS 13
#define CLK 15
#define DEVICES 4

const char* host = "legoleds";

ESP8266WebServer server(80);

LedControl lc =  LedControl(DIN,CLK,CS,DEVICES);
unsigned long delayTime=200;  // Delay between Frames

RenderText renderText(lc);
File fsUploadFile;

RenderAnim renderAnim(lc);

void appendAnim(){
  if(!server.hasArg("name")) {
    server.send(500, "text/plain", "BAD ARGS name is mandatory");
    return;
  }
  if(!server.hasArg("frame")) {
    server.send(500, "text/plain", "BAD ARGS frame is mandatory");
    return;
  }
  String frame = server.arg("frame");
  Serial.println("frame");
  Serial.println(frame);
  String path = server.arg("name");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(frame);
  if (!root.success()) {
    server.send(500, "text/plain", "Error parsing request");
    return;
  }
  byte frameBuffer[4][8];
  for(int x = 0; x < 4; x++){
       for(int y = 0; y < 8; y++){
         String b =  root["frames"][x][y];
         Serial.println(b);
          frameBuffer[x][y] = b.toInt();
        }
  }
  SPIFFS.begin();

  File file = SPIFFS.open(path, "a");
  int size = sizeof(frameBuffer);
  Serial.printf("Size:%i",size);
  uint8_t buffer[size];
  memcpy(buffer, &frameBuffer, size);
  file.write(buffer, size);
  file.close();
  SPIFFS.end();
  server.send(200, "text/plain", "OK");
}

/*void newAnim(){
  if(!server.hasArg("name")) {
    server.send(500, "text/plain", "BAD ARGS name is mandatory");
    return;
  }
  SPIFFS.begin();
  String path = server.arg("name");
  File file = SPIFFS.open(path, "w");
  file.close();
  SPIFFS.end();
  server.send(200, "text/plain", "OK");
}*/

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  SPIFFS.begin();
  if(path.endsWith("/")) path += "index.htm";
  String contentType = "text/html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  SPIFFS.end();
  return false;
}

void getInfo(){
  StaticJsonBuffer<200> jsonBuffer;
  Serial.println("memory: ");
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  String result;
  JsonObject& root = jsonBuffer.createObject();
  root["totalBytes"] = fs_info.totalBytes;
  root["usedBytes"] = fs_info.usedBytes;
  root["blockSize"] = fs_info.blockSize;
  root["pageSize"] = fs_info.pageSize;
  root["maxOpenFiles"] = fs_info.maxOpenFiles;
  root["maxPathLength"] = fs_info.maxPathLength;
  root.printTo(result);
  Serial.println(result);
  server.send(200, "text/json", result);
}

void handleText(){
  if(!server.hasArg("value")) {
    server.send(500, "text/plain", "BAD ARGS value is mandatory");
    return;
  }
  String text = server.arg("value");
  String loop = server.arg("loop");
  String time = server.arg("time");
  renderText.setText(text, loop == "true", time.toInt());
  server.send(200, "text/plain", "OK");
}
void handleFileUpload(){
  HTTPUpload& upload = server.upload();

  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")){
      filename = "/"+filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    SPIFFS.begin();
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile){
      Serial.printf("%s\n", "escribiendo");
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile){
      fsUploadFile.close();
    }
    SPIFFS.end();
    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
  }
}
void handleFileList() {
  String result;
  if(!server.hasArg("name")) {
    server.send(500, "text/plain", "BAD ARGS name is mandatory");
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  String path = server.arg("name");
  Serial.println("handleFileList: " + path);
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir(path);
  path = String();
  JsonArray& data = root.createNestedArray("files");
  while(dir.next()){
    JsonObject& element = data.createNestedObject();
    File entry = dir.openFile("r");
    element["type"] = "file";
    String name(entry.name());
    element["name"] = name;
    element["size"] = entry.size();
    entry.close();
    //data.add(element);
  }
  SPIFFS.end();
  root.printTo(result);
  Serial.println(result);
  server.send(200, "text/json", result);
}

void setup()
{
  Serial.begin(115200);
  Serial.print("\n");
  Serial.setDebugOutput(true);

  WiFiManager wifiManager;
  wifiManager.autoConnect(host);

  ArduinoOTA.begin();

  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);

  //Handleds definitions
  server.on("/fs/info", HTTP_GET, getInfo);
  server.on("/fs/dir", HTTP_GET, handleFileList);
  server.on("/text", HTTP_POST, handleText);
  server.on("/anim", HTTP_POST, [](){
      server.send(200, "text/plain", "");
    }, handleFileUpload);
  server.on("/anim", HTTP_PUT, appendAnim );
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();
  Serial.println("HTTP server started");
  //Init anim
  renderAnim.setAnim("/initial.anim",false,200);

}

void loop()
{
 ArduinoOTA.handle();
 server.handleClient();
 renderText.handleRender();
 renderAnim.handleRender();
}
