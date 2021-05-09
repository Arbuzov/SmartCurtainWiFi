
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "Settings.h"
#include "CustomStepper.h"

#define IN1  12
#define IN2  14
#define IN3  27
#define IN4  26

CustomStepper mystepper(IN1, IN2, IN3, IN4);

boolean debug = false;
int position = 100;
long steps = 0;
long maxSteps = CURTAIN_STEPS / mystepper.step;

const char* ssid = WIFI_SSID;
const char* password = WIFI_SECRET;

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
const char* okResult = "{\"result\":\"ok\"}";
const char* respJson = "application/json";
const char* respText = "application/plain";

WebServer server(80);

void handleRoot() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", serverIndex);
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
  server.send(404, respText, message);
}

double calcPosition() {
  return (maxSteps - steps) * 100 / maxSteps;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  WiFi.disconnect(false, true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++tryCount > 20) {
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.enableCORS();
  server.on("/", handleRoot);

  server.on("/api/open", []() {
    mystepper.direction = 1;
    position = 110;
    server.send(200, respJson, okResult);
  });

  server.on("/api/close", []() {
    mystepper.direction = -1;
    position = -10;
    server.send(200, respJson, okResult);
  });

  server.on("/api/status", []() {
    String status = String(
                      "{\"result\":\"ok\", \"steps\": " + String(steps) +
                      ", \"maxSteps\": "                + String(maxSteps) +
                      ", \"debug\": "                   + String(debug) +
                      ", \"direction\":"                + String(mystepper.direction) +
                      ", \"position\": "                + String(int(calcPosition())) + "}"
                    );
    server.send(200, respJson, status);
  });

  server.on("/api/stop", []() {
    mystepper.direction = 0;
    position = calcPosition();
    mystepper.stop();
    server.send(200, respJson, okResult);
  });

  server.on("/api/opened", []() {
    steps = 0;
    server.send(200, respJson, okResult);
  });

  server.on("/api/closed", []() {
    steps = maxSteps;
    server.send(200, respJson, okResult);
  });

  server.on("/api/position/{}", []() {

    if (server.pathArg(0).toInt() < 0 || server.pathArg(0).toInt() > 100) {
      server.send(200, respJson, "{\"result\":\"error\"}");
    } else {
      position = server.pathArg(0).toInt();
      if (position > calcPosition()) {
        mystepper.direction = 1;
      } else if (position < calcPosition()) {
        mystepper.direction = -1;
      }
      server.send(200, respJson, okResult);
    }
  });

  server.on("/api/debug", []() {
    debug = !debug;
    server.send(200, respJson, String("{\"result\":\"ok\", \"value\":\"") + debug + String("\"}"));
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, respText, (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin()) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    } else {
      Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
    }
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");


}

void loop()
{
  server.handleClient();

  if (
    !debug && (
      ((position == calcPosition()) && mystepper.direction != 0) ||
      (steps <= 0 && mystepper.direction == 1 ) ||
      (steps >= maxSteps && mystepper.direction == -1)
    )
  ) {
    mystepper.direction = 0;
    position = calcPosition();
    mystepper.stop();
    Serial.println("stopped " + String(steps) + "  " + String(mystepper.direction));
  }

  if (mystepper.direction != 0) {
    steps = steps - mystepper.direction;
    mystepper.move();
  }

}
