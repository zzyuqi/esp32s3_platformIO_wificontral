#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

// ==================== WiFi 配置 ====================
const char* WIFI_SSID = "zyq";
const char* WIFI_PASSWORD = "123123123";

// ==================== NTP 配置 ====================
const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = 8 * 3600;    // 东八区
const int   DAYLIGHT_OFFSET_SEC = 0;

// ==================== Web 服务器 ====================
WebServer server(80);

// ==================== LED 引脚 ====================
#define LED_PIN 10  // ESP32-S3 开发板上的 LED

// ==================== 网页 HTML ====================
String getHtmlPage() {
  // 获取当前时间
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

  // 获取运行时间
  unsigned long uptimeSec = millis() / 1000;
  int hours = uptimeSec / 3600;
  int minutes = (uptimeSec % 3600) / 60;
  int seconds = uptimeSec % 60;

  // 获取 WiFi 信号强度
  int rssi = WiFi.RSSI();

  String html = "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<title>ESP32-S3 网络示例</title>"
  "<style>"
  "body { font-family: Arial; margin: 20px; background: #f0f0f0; }"
  "h1 { color: #333; }"
  ".card { background: white; padding: 20px; margin: 10px 0; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }"
  ".info { font-size: 18px; margin: 10px 0; }"
  ".label { color: #666; font-weight: bold; }"
  ".value { color: #2196F3; }"
  ".btn { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }"
  ".btn-on { background: #4CAF50; color: white; }"
  ".btn-off { background: #f44336; color: white; }"
  ".btn:hover { opacity: 0.8; }"
  "</style>"
  "</head>"
  "<body>"
  "<h1>ESP32-S3 网络服务器</h1>"

  "<div class='card'>"
  "<h2>系统信息</h2>"
  "<div class='info'><span class='label'>当前时间:</span> <span class='value'>" + String(timeStr) + "</span></div>"
  "<div class='info'><span class='label'>运行时间:</span> <span class='value'>" + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s</span></div>"
  "<div class='info'><span class='label'>芯片型号:</span> <span class='value'>" + String(ESP.getChipModel()) + "</span></div>"
  "<div class='info'><span class='label'>芯片ID:</span> <span class='value'>" + String(ESP.getChipCores()) + " 核心</span></div>"
  "<div class='info'><span class='label'>Flash大小:</span> <span class='value'>" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB</span></div>"
  "</div>"

  "<div class='card'>"
  "<h2>WiFi 信息</h2>"
  "<div class='info'><span class='label'>IP地址:</span> <span class='value'>" + WiFi.localIP().toString() + "</span></div>"
  "<div class='info'><span class='label'>网关:</span> <span class='value'>" + WiFi.gatewayIP().toString() + "</span></div>"
  "<div class='info'><span class='label'>信号强度:</span> <span class='value'>" + String(rssi) + " dBm (" + String(WiFi.RSSI()) + ")</span></div>"
  "<div class='info'><span class='label'>SSID:</span> <span class='value'>" + String(WiFi.SSID()) + "</span></div>"
  "</div>"

  "<div class='card'>"
  "<h2>LED 控制</h2>"
  "<p>当前状态: <span id='ledStatus' class='value'>--</span></p>"
  "<button class='btn btn-on' onclick=\"fetch('/led/on')\">打开 LED</button>"
  "<button class='btn btn-off' onclick=\"fetch('/led/off')\">关闭 LED</button>"
  "<script>"
  "function updateLed() {"
  "  fetch('/led/status').then(r=>r.text()).then(s=>document.getElementById('ledStatus').innerText=s);"
  "}"
  "setInterval(updateLed, 1000);"
  "updateLed();"
  "</script>"
  "</div>"

  "<div class='card'>"
  "<h2>功能测试</h2>"
  "<button class='btn btn-on' onclick=\"fetch('/reboot').then(()=>alert('重启中...')\")\">重启设备</button>"
  "<button class='btn btn-on' onclick=\"fetch('/info/json').then(r=>r.json()).then(d=>console.log(d))\">获取JSON信息</button>"
  "</div>"

  "</body>"
  "</html>";

  return html;
}

// ==================== LED 状态变量 ====================
bool ledState = false;

// ==================== Web 服务器路由 ====================
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", getHtmlPage());
}

void handleLedOn() {
  ledState = true;
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "LED ON");
  Serial.println("LED 已打开");
}

void handleLedOff() {
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "LED OFF");
  Serial.println("LED 已关闭");
}

void handleLedStatus() {
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void handleJsonInfo() {
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

  String json = "{";
  json += "\"time\":\"" + String(timeStr) + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"ssid\":\"" + String(WiFi.SSID()) + "\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"led\":" + String(ledState ? "true" : "false") + ",";
  json += "\"chip\":\"" + String(ESP.getChipModel()) + "\",";
  json += "\"flash\":" + String(ESP.getFlashChipSize());
  json += "}";

  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// ==================== WiFi 连接 ====================
void connectWiFi() {
  Serial.println();
  Serial.println("========== WiFi 连接开始 ==========");
  Serial.printf("正在连接 WiFi: %s\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 等待连接，最多 30 秒
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("========== WiFi 连接成功! ==========");
    Serial.printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("信号强度: %d dBm\n", WiFi.RSSI());
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("网关: %s\n", WiFi.gatewayIP().toString().c_str());
  } else {
    Serial.println();
    Serial.println("WiFi 连接失败!");
  }
}

// ==================== NTP 时间同步 ====================
void setupNTP() {
  Serial.println("\n========== NTP 时间同步 ==========");
  Serial.printf("正在同步 NTP 时间 (服务器: %s)...\n", NTP_SERVER);

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // 等待时间获取
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }

  if (getLocalTime(&timeinfo)) {
    Serial.println();
    Serial.println("========== NTP 时间同步成功! ==========");
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("当前时间: %s\n", timeStr);
  } else {
    Serial.println("NTP 时间同步失败!");
  }
}

// ==================== 打印本地时间 ====================
void printLocalTime() {
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
  Serial.printf("[%s] ", timeStr);
}

// ==================== setup ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("*********************************************");
  Serial.println("     ESP32-S3 网络示例 - 学习代码");
  Serial.println("*********************************************");

  // 初始化 LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 连接 WiFi
  connectWiFi();

  // 同步 NTP 时间
  setupNTP();

  // 配置 Web 服务器路由
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/led/status", handleLedStatus);
  server.on("/info/json", handleJsonInfo);
  server.on("/reboot", []() {
    server.send(200, "text/plain", "Rebooting...");
    delay(1000);
    ESP.restart();
  });
  server.onNotFound(handleNotFound);

  // 启动 Web 服务器
  server.begin();
  Serial.println("\n========== Web 服务器已启动 ==========");
  printLocalTime();
  Serial.printf("在浏览器访问: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("*********************************************");
}

// ==================== loop ====================
void loop() {
  // 处理 Web 服务器请求
  server.handleClient();

  // 每 5 秒打印一次状态
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  if (now - lastPrint > 5000) {
    lastPrint = now;

    printLocalTime();
    Serial.printf("IP: %s | RSSI: %d dBm | LED: %s | 运行: %lu s\n",
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI(),
                  ledState ? "ON" : "OFF",
                  now / 1000);
  }

  delay(10);
}
