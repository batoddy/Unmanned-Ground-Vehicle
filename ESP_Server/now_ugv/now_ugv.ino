#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi bilgilerini buraya gir
const char* ssid = "Aferin";
const char* password = "batu78999";

// PIN tanımları
#define OUTPUT_PIN 2       // LED veya röle kontrol pini
#define BUTTON_PIN 21      // Fiziksel butonun bağlı olduğu pin

volatile bool buttonState = true;

WebServer server(80);

// ISR (Interrupt Service Routine)
void IRAM_ATTR handleButtonInterrupt() {
  buttonState = digitalRead(BUTTON_PIN)==HIGH ? false : true;
  digitalWrite(OUTPUT_PIN,buttonState ? HIGH : LOW);
  buttonState ? Serial.println("Emergency - Button IRQ"):Serial.println("Emergency Exit - Button IRQ");
  Serial.print("Output state: ");
  Serial.println(buttonState);
}

// HTML arayüz
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head><title>ESP32 Buton Kontrol</title></head>
    <body>
      <h2>ESP32 Emergency Server</h2>
      <form action="/on" method="GET">
        <button style="font-size:24px;">EMERGENCY STATE (HIGH)</button>
      </form>
      <form action="/off" method="GET">
        <button style="font-size:24px;">TURN OFF EMERGENCY (LOW)</button>
      </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(OUTPUT_PIN, HIGH);
  buttonState = true;
  server.sendHeader("Location", "/");
  server.send(303);
  Serial.println("Emergency - Server IRQ");
  Serial.print("Output state: ");
  Serial.println(buttonState);
}

void handleOff() {
  digitalWrite(OUTPUT_PIN, LOW);
  buttonState = false;
  server.sendHeader("Location", "/");
  server.send(303);
  Serial.println("Emergency Exit - Server IRQ");
  Serial.print("Output state: ");
  Serial.println(buttonState);
}

void setup() {
  Serial.begin(115200);

  // Çıkış ve buton pin ayarları
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Buton bir ucu GND’ye, diğer ucu GPIO21'e bağlanmalı

  // Başlangıçta çıkışı kapalı yap
  digitalWrite(OUTPUT_PIN, LOW);

  // Buton için interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, CHANGE);

  // WiFi başlat
  WiFi.begin(ssid, password);
  Serial.print("WiFi bağlanıyor");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi bağlantısı tamamlandı!");
  Serial.print("ESP32 IP adresi: ");
  Serial.println(WiFi.localIP());

  // HTTP sunucusu
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
  Serial.println("HTTP sunucu başlatıldı");
}

void loop() {
  server.handleClient();
}
