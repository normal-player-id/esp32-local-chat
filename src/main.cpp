
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

WebServer server(80);

const char* AP_NAME = "local_server";
const char* AP_PASSWORD = "vesna!!!";

struct ChatMessage {
  uint32_t id;
  String name;
  String message;
};

ChatMessage messages[20];
uint32_t nextId = 1;
uint8_t messageCount = 0;

String escapeJson(const String& input) {
  String out;
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];

    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else if ((uint8_t)c >= 32) {
      out += c;
    }
  }
  return out;
}

void sendFile(const char* path, const char* type) {
  File file = LittleFS.open(path, "r");

  if (!file) {
    server.send(404, "text/plain", "File tidak ditemukan");
    return;
  }

  server.streamFile(file, type);
  file.close();
}

void handleMessages() {
  uint32_t after = server.arg("after").toInt();
  String json = "[";

  bool first = true;

  for (uint8_t i = 0; i < messageCount; i++) {
    if (messages[i].id <= after) continue;

    if (!first) json += ",";
    first = false;

    json += "{\"id\":" + String(messages[i].id);
    json += ",\"name\":\"" + escapeJson(messages[i].name) + "\"";
    json += ",\"message\":\"" + escapeJson(messages[i].message) + "\"}";
  }

  json += "]";
  server.send(200, "application/json", json);
}

void handleSend() {
  String name = server.arg("name");
  String message = server.arg("message");

  name.trim();
  message.trim();

  if (name.isEmpty() || message.isEmpty()) {
    server.send(400, "text/plain", "Nama dan pesan wajib diisi");
    return;
  }

  if (name.length() > 20 || message.length() > 160) {
    server.send(400, "text/plain", "Pesan terlalu panjang");
    return;
  }

  if (messageCount == 20) {
    for (int i = 1; i < 20; i++) {
      messages[i - 1] = messages[i];
    }
    messageCount--;
  }

  messages[messageCount++] = {nextId++, name, message};

  server.send(200, "application/json", "{\"ok\":true}");
}

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS gagal dimulai!");
    return;
  }

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(AP_NAME, AP_PASSWORD)) {
    Serial.println("Wi-Fi AP gagal dibuat!");
    return;
  }

  server.on("/", HTTP_GET, []() {
    sendFile("/index.html", "text/html; charset=utf-8");
  });

  server.on("/style.css", HTTP_GET, []() {
    sendFile("/style.css", "text/css; charset=utf-8");
  });

  server.on("/script.js", HTTP_GET, []() {
    sendFile("/script.js", "application/javascript; charset=utf-8");
  });

  server.on("/api/messages", HTTP_GET, handleMessages);
  server.on("/api/send", HTTP_POST, handleSend);

  server.begin();

  Serial.println();
  Serial.println("Local Chat siap!");
  Serial.print("Wi-Fi: ");
  Serial.println(AP_NAME);
  Serial.print("Alamat: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
