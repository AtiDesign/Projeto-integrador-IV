// ============================================================================
// ESP32 - MONITOR DE TEMPERATURA PARA CULTIVO
// DHT11 + Indicadores Visuais para 5 Vegetais
// ============================================================================

#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// ============================================================================
// CONFIGURAÇÕES WI-FI
// ============================================================================
const char* ssid = "alienGame";
const char* password = "2513vxbrasil";

// ============================================================================
// CONFIGURAÇÕES DO SENSOR DHT11
// ============================================================================
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ============================================================================
// SERVIDOR WEB
// ============================================================================
WebServer server(8080);

// ============================================================================
// ESTRUTURA DE DADOS
// ============================================================================
struct DadosTemperatura {
  float temperatura = 0.0;
  float umidade = 0.0;
  unsigned long ultimaLeitura = 0;
  bool valido = false;
};

DadosTemperatura sensor;

// Intervalo de atualização
const unsigned long INTERVALO_LEITURA = 2000; // 2 segundos
unsigned long ultimaLeitura = 0;

// ============================================================================
// ESTRUTURA DE VEGETAIS E SUAS FAIXAS DE TEMPERATURA
// ============================================================================
struct Vegetal {
  String nome;
  float tempMin;
  float tempMax;
  float tempIdealMin;
  float tempIdealMax;
};

Vegetal vegetais[5] = {
  {"Arroz",   20.0, 35.0, 25.0, 30.0},  // Adequa entre 20-35°C
  {"Milho",   15.0, 35.0, 20.0, 30.0},  // Bom em quase todos climas
  {"Soja",    20.0, 30.0, 22.0, 28.0},  // Adapta bem entre 20-30°C
  {"Tomate",  15.0, 25.0, 18.0, 24.0},  // Germina 15-25°C, suporta 10-34°C
  {"Alface",  20.0, 35.0, 22.0, 28.0}   // 20-35°C, prefere temperados
};

// ============================================================================
// FUNÇÃO PARA DETERMINAR STATUS DO VEGETAL
// ============================================================================
String getStatus(float temp, Vegetal v) {
  if (temp < v.tempMin || temp > v.tempMax) {
    return "danger"; // Vermelho - Fora da faixa
  } else if (temp >= v.tempIdealMin && temp <= v.tempIdealMax) {
    return "success"; // Verde - Ideal para germinação
  } else {
    return "warning"; // Amarelo - Aceitável mas não ideal
  }
}

String getStatusTexto(float temp, Vegetal v) {
  if (temp < v.tempMin || temp > v.tempMax) {
    return "Fora da faixa";
  } else if (temp >= v.tempIdealMin && temp <= v.tempIdealMax) {
    return "Ideal para germinação";
  } else {
    return "Aceitável";
  }
}

// ============================================================================
// ADICIONAR HEADERS CORS
// ============================================================================
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ============================================================================
// FUNÇÃO PARA LER DHT11
// ============================================================================
void lerSensorDHT11() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ Erro ao ler DHT11!");
    sensor.valido = false;
    return;
  }
  
  sensor.temperatura = temp;
  sensor.umidade = hum;
  sensor.ultimaLeitura = millis();
  sensor.valido = true;
  
  Serial.print("🌡️ Temp: ");
  Serial.print(temp, 1);
  Serial.print("°C | Umid: ");
  Serial.print(hum, 1);
  Serial.println("%");
}

// ============================================================================
// ENDPOINT: /dados
// ============================================================================
void handleDados() {
  addCORSHeaders();
  
  String json = "{";
  json += "\"temperatura\":" + String(sensor.temperatura, 1) + ",";
  json += "\"umidade\":" + String(sensor.umidade, 1) + ",";
  json += "\"valido\":" + String(sensor.valido ? "true" : "false") + ",";
  json += "\"vegetais\":[";
  
  for (int i = 0; i < 5; i++) {
    json += "{";
    json += "\"nome\":\"" + vegetais[i].nome + "\",";
    json += "\"status\":\"" + getStatus(sensor.temperatura, vegetais[i]) + "\",";
    json += "\"statusTexto\":\"" + getStatusTexto(sensor.temperatura, vegetais[i]) + "\",";
    json += "\"tempMin\":" + String(vegetais[i].tempMin, 1) + ",";
    json += "\"tempMax\":" + String(vegetais[i].tempMax, 1);
    json += "}";
    if (i < 4) json += ",";
  }
  
  json += "]}";
  
  server.send(200, "application/json", json);
}

// ============================================================================
// PÁGINA WEB PRINCIPAL
// ============================================================================
void handleRoot() {
  addCORSHeaders();
  
  String html = "<!DOCTYPE html>";
  html += "<html><head><meta charset=\"UTF-8\">";
  html += "<title>Monitor de Cultivo ESP32</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', Tahoma, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 20px; }";
  html += ".container { max-width: 1200px; margin: 0 auto; }";
  html += ".header { background: white; border-radius: 20px; padding: 30px; margin-bottom: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); }";
  html += ".header h1 { color: #333; font-size: 32px; margin-bottom: 10px; }";
  html += ".temp-display { display: flex; align-items: center; gap: 20px; margin-top: 20px; }";
  html += ".temp-value { font-size: 72px; font-weight: bold; color: #667eea; }";
  html += ".temp-unit { font-size: 24px; color: #999; }";
  html += ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }";
  html += ".card { background: white; border-radius: 15px; padding: 25px; box-shadow: 0 5px 15px rgba(0,0,0,0.15); transition: transform 0.3s; }";
  html += ".card:hover { transform: translateY(-5px); }";
  html += ".card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }";
  html += ".vegetal-name { font-size: 24px; font-weight: bold; color: #333; }";
  html += ".status-badge { padding: 8px 16px; border-radius: 20px; font-size: 14px; font-weight: bold; }";
  html += ".status-success { background: #4caf50; color: white; }";
  html += ".status-warning { background: #ff9800; color: white; }";
  html += ".status-danger { background: #f44336; color: white; }";
  html += ".temp-range { margin: 15px 0; padding: 15px; background: #f5f5f5; border-radius: 10px; }";
  html += ".temp-range p { margin: 8px 0; color: #555; font-size: 14px; }";
  html += ".indicator { width: 100%; height: 40px; border-radius: 10px; margin-top: 15px; display: flex; align-items: center; justify-content: center; font-weight: bold; color: white; font-size: 16px; }";
  html += ".indicator-success { background: linear-gradient(135deg, #4caf50, #8bc34a); }";
  html += ".indicator-warning { background: linear-gradient(135deg, #ff9800, #ffc107); }";
  html += ".indicator-danger { background: linear-gradient(135deg, #f44336, #e91e63); }";
  html += ".footer { text-align: center; color: white; margin-top: 30px; font-size: 14px; }";
  html += "@media(max-width: 768px) { .temp-value { font-size: 56px; } }";
  html += "</style>";
  html += "<script>";
  html += "setInterval(() => location.reload(), 3000);";
  html += "</script>";
  html += "</head><body>";
  
  html += "<div class='container'>";
  
  // Header com temperatura atual
  html += "<div class='header'>";
  html += "<h1>PROJETO INTEGRADOR 2025 - UNIVESP</h1>";
  html += "<h1>🌱 Monitor de Temperatura para Cultivo </h1>";
  html += "<div class='temp-display'>";
  html += "<div class='temp-value'>" + String(sensor.temperatura, 1) + "</div>";
  html += "<div class='temp-unit'>°C</div>";
  html += "</div>";
  html += "<p style='color:#666;margin-top:10px'>Umidade: " + String(sensor.umidade, 1) + "%</p>";
  html += "</div>";
  
  // Grid de vegetais
  html += "<div class='grid'>";
  
  for (int i = 0; i < 5; i++) {
    String status = getStatus(sensor.temperatura, vegetais[i]);
    String statusTexto = getStatusTexto(sensor.temperatura, vegetais[i]);
    
    html += "<div class='card'>";
    html += "<div class='card-header'>";
    html += "<div class='vegetal-name'>" + vegetais[i].nome + "</div>";
    html += "<div class='status-badge status-" + status + "'>";
    
    if (status == "success") html += "✓ IDEAL";
    else if (status == "warning") html += "⚠ OK";
    else html += "✗ FORA";
    
    html += "</div></div>";
    
    html += "<div class='temp-range'>";
    html += "<p><strong>Faixa total:</strong> " + String(vegetais[i].tempMin, 0) + "°C - " + String(vegetais[i].tempMax, 0) + "°C</p>";
    html += "<p><strong>Ideal germinação:</strong> " + String(vegetais[i].tempIdealMin, 0) + "°C - " + String(vegetais[i].tempIdealMax, 0) + "°C</p>";
    html += "</div>";
    
    html += "<div class='indicator indicator-" + status + "'>";
    html += statusTexto;
    html += "</div>";
    
    html += "</div>";
  }
  
  html += "</div>";
  
  html += "<div class='footer'>";
  html += "<p>ESP32 Monitor | IP: " + WiFi.localIP().toString() + " | Atualização automática a cada 3s</p>";
  html += "</div>";
  
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// ============================================================================
// 404
// ============================================================================
void handleNotFound() {
  addCORSHeaders();
  server.send(404, "application/json", "{\"erro\":\"Endpoint não encontrado\"}");
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║  ESP32 - MONITOR DE TEMPERATURA CULTIVO   ║");
  Serial.println("║  DHT11 + Indicadores para 5 Vegetais      ║");
  Serial.println("╚════════════════════════════════════════════╝\n");
  
  // Inicializar DHT11
  dht.begin();
  Serial.println("✅ DHT11 inicializado");
  
  // Conectar WiFi
  Serial.println("\n🌐 Conectando ao WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi conectado!");
    Serial.print("   📍 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("❌ Falha na conexão WiFi!");
    while(1) delay(1000);
  }
  
  // Configurar servidor
  server.on("/", HTTP_GET, handleRoot);
  server.on("/dados", HTTP_GET, handleDados);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("✅ Servidor ativo na porta 8080");
  Serial.print("🌐 Acesse: http://");
  Serial.println(WiFi.localIP());
  
  // Primeira leitura
  delay(2000);
  lerSensorDHT11();
  
  Serial.println("\n✅ SISTEMA OPERACIONAL!\n");
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================
void loop() {
  server.handleClient();
  
  unsigned long now = millis();
  
  // Leitura a cada 2 segundos
  if (now - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = now;
    lerSensorDHT11();
  }
  
  delay(10);
}