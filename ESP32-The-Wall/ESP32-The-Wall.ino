// 'THE WALL' [ESP32 Public Message Board] (CAPTIVE)
//Creates an AP named 'The Wall' and serves a simple webpage where anyone can leave an annoymous message.
//Simply connect to the Access Point that is created and navigate to http://wall.local OR http://192.168.4.1
// On Android Devices or Windows Laptops a login pop up should appear
// Use http://wall.local/logs to see other nearby wifi networks.

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

const char *ssid = "📣 The Wall 📣";
const char *password = "";

AsyncWebServer server(80);
DNSServer dnsServer;

String messageBoard;
String deviceLogs;
unsigned long startTime;

String getUptime() {
  unsigned long currentTime = millis();
  unsigned long uptimeInSeconds = (currentTime - startTime) / 1000;
  unsigned long days = uptimeInSeconds / 86400;
  uptimeInSeconds %= 86400;
  unsigned long hours = uptimeInSeconds / 3600;
  uptimeInSeconds %= 3600;
  unsigned long minutes = uptimeInSeconds / 60;
  unsigned long seconds = uptimeInSeconds % 60;
  return String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void saveMessagesToFile() {
  File file = SPIFFS.open("/messages.json", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  DynamicJsonDocument jsonDoc(8192);
  jsonDoc["messages"] = messageBoard;
  serializeJson(jsonDoc, file);
  file.close();
}

void loadMessagesFromFile() {
  File file = SPIFFS.open("/messages.json", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  DynamicJsonDocument jsonDoc(8192);
  deserializeJson(jsonDoc, file);
  messageBoard = jsonDoc["messages"].as<String>();
  file.close();
}

void saveLogsToFile() {
  File file = SPIFFS.open("/logs.json", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["logs"] = deviceLogs;
  serializeJson(jsonDoc, file);
  file.close();
}

void loadLogsFromFile() {
  File file = SPIFFS.open("/logs.json", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, file);
  deviceLogs = jsonDoc["logs"].as<String>();
  file.close();
}

void updateDeviceLogs() {
  deviceLogs = "<h2 align='center'>Discovered Networks:</h2>";
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    deviceLogs += "<p>No nearby networks detected..</p>";
  } else {
    for (int i = 0; i < numNetworks; ++i) {
      deviceLogs += "<p>" +
                    String("SSID: ") + WiFi.SSID(i) + "<br>" +
                    String("BSSID: ") + WiFi.BSSIDstr(i) + "</p>";
    }
  }
  saveLogsToFile();
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("Access Point IP address: " + apIP.toString());
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount file system");
    return;
  }
  loadMessagesFromFile();
  loadLogsFromFile();
  startTime = millis();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  String html = "<html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css?family=Open+Sans\"></head>";
  html += "<body style=\"background:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAD04JH5AAACVVBMVEVHcEz///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////84/////////////////////////////////////////////////////////////////////////////v////79///+//8JM9G3AAAAwnRSTlMARAMM/AsF/gIB9w39FPsHHhvREvUI+gS+2wkm8IxYCi3vBryvzSLsDjAg5qM52nK6Lh2s+BBMXqWTQiQP1sVOSUpBVePK3amCPlmdwYTY52ZWfinINDOft1/hL7O2K/aGMTex9IX5YcM81O1ceVtAanQc5Xh9fL9di5o4da070NWclj0yF6Zn4iMTiYjAJYHJf0Y6dsJ3h1JogNmQuKuZFo9a8qRQsM7onukZzEhrJwlRg40qZSyu3FNx68bq5GOgqnHbEFAAAAN5SURBVBgZ7cFVdxtXAEbRT1DNjJkZ4oAhdhw7DjMzMzNjmduUmZmZmRlP0v6ujmSQYrnRQ9bc+zJ7KxQKhUKhUCgUumIlnqzaRY9smgRMlD3tJJXIlmJSCmRJeZx+41xZMZlBD8iGKtL2yrhYC/3OtwFOiUyrZcCdWg485MqsCgY1S9EeqJVRtzLEVVJTzf0yaS1DKmUDabNkwW2kzZAFV5M2VxaQabTMI9M1Mi7BJWTcWC5xgyvD1jHMCpmVxzAFMmo2FximTSbtJ9t6GXQHIxglcxjRtTKllRE5R26cd2BWyaSYgjaTy7tXASsjh/EKFrnE6xQocupTcLzeJVzeP3fdXaqglDaSQ8GahAKzu4VcipoVnNv5HxcZ1K0gdZJTqQI0hcv4qPA4sFRBijAy56eeE/JtcVim4HiKk63mi2OTEurn9jJRQfKilc1XpdUllPLH31LxuabPP5WWr5Y5vVOnTp3gyT3Jkd9IeVd6QaZ8Q79YH77fp/9y+vR6V0aMj8cbmo5P+ar+FDBZ8FexJ6MmkvR1Eb6DRxt17uyfP/8gs9zvvuVLYLt04Fd8hz0ZdQam7ZrG2nxPBfjiMuzsvtouRQBnh3vmx0P7FftM5kVIiu/5Xnt3F30s8yKkfSILIqQ4ON2NsiFCSoNsiZDiyJYISY/VS3JXPtp3U9f198moFfgSkkY/zoDF42XSqs5V8o0hw9ioTJjwRryiTgMeccjQoOAlXsVXqKQF+55S+xgytCho80n6UFM2qXo1SU8+XD5/x4T61votG08upcxToEaRtEEnJie0kyHOnEPLto87LGlRQoHCty2h950P1Emmf/G1Kmhl4LwslUP0PbK9qIC5VdQWSiqAUkaQV61gPcviakmboWMNWZocnlewnu6WLx8oJ9sT3jgWKVDPxeS7B4iSJU/SgjH5Cp4DeGRplCGjgLgYLs+VITcDFSLTRWCDTKnBJ4bpkjEN+DwuNUPmOPii08k0TQZdh69wIRnyZNJYfFu9ozgM2iST5uEr0jY6GLBHRnWQJL1WzABPRo2Gg3BMEv3myLAlK10ok+SQMluGvZKvN6FQep0UWVAdZ7r0Nkm3yIZ3oFIxkmbKip20SVX4XFlRuY6tegufLGmvOiXVQFzWVGxUcRHImrrNCb3EBVdWPcNC2fXgXFmWr1AoFAqFQqFQ6Ar8B7oMBQn0DQfKAAAAAElFTkSuQmCC), linear-gradient(to bottom left, #fa711b, #8104c9)\">";
  html += "<div align='center'>";
  html += "<h1 style=\"border-radius: 5px; background-color: #404040; color: white; font-size: 36px;\"> 📣 THE WALL 📣</h1>";
  html += "<h2 style=\"border-radius: 5px; background-color: #404040; color: white; font-size: 24px;\"> Public Message Board</h2>";
  html += "<form id='messageForm' action='/post' method='post' style=\"border-radius: 5px; background-color: #404040; color: white;\">";
  html += "<input type='text' name='message' style=\"border-radius: 5px; font-size: 24px;\" placeholder='Enter your message' autofocus>";
  html += "<input type='submit' value='Post' style=\"border-radius: 5px; background-color: #00cc00; color: white; font-weight: bold; font-size: 26px;\">";
  html += "<p style=\"border-radius: 5px; background-color: #404040; color: white; font-weight: bold; font-size: 16px;\"> Uptime: " + getUptime() + "</p>";
  html += "</form></div>";
  html += "<h2 align='center' style=\"border-radius: 5px; background-color: #404040; color: white; font-size: 30px;\"> Recent Messages:</h2>";
  html += "<div id='messageBoard' style=\"border-radius: 5px; background-color: #404040; color: white; font-size: 24px;\">" + messageBoard + "</div>";
  html += "<script>function refreshMessages() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.onreadystatechange = function() {";
  html += "if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "document.getElementById('messageBoard').innerHTML = xhr.responseText;";
  html += "}";
  html += "};";
  html += "xhr.open('GET', '/messages', true);";
  html += "xhr.send();";
  html += "} setInterval(refreshMessages, 5000); </script>";
  html += "</body></html>";
  request->send(200, "text/html", html);
  });

  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("message", true)) {
      AsyncWebParameter* p = request->getParam("message", true);
      messageBoard = "<p>" + p->value() + "</p>" + messageBoard;
      saveMessagesToFile();
      request->redirect("/");
    } else {
      request->send(400, "text/plain", "Invalid request");
    }
  });

  server.on("/messages", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", messageBoard);
  });

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    updateDeviceLogs();
    String logsPage = "<html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>";
    logsPage += "<body style=\"background:url(data:image/png;base64,), linear-gradient(to bottom left, #fa711b, #8104c9)\">";
    logsPage += "<h1 align='center' style=\"border-radius: 10px; background-color: #404040; color: white; font-size: 36px;\">Nearby WiFi Logs</h1>";
    logsPage += "<div id='logs' style=\"background-color: #404040; color: white; font-size: 20px;\">" + deviceLogs + "</div>";
    logsPage += "</body></html>";
    request->send(200, "text/html", logsPage);
  });

  server.onNotFound(handleNotFound);

  if (!MDNS.begin("wall")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  MDNS.addService("http", "tcp", 80);
  server.begin();
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);
}

void loop() {
  dnsServer.processNextRequest();
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) {
    updateDeviceLogs();
    lastUpdate = millis();
  }
}
