#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <TimeLib.h>
#include "time.h"
#include "paginaWeb.h"



bool tempoAtualizado = false;
const char *ssid = "Monitor de Temperatura";

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer server(80);

void handleFileStream() {
  String path = "dados.txt"; 
  if (!LittleFS.exists(path)) {
    server.send(404, "text/plain", "Arquivo não encontrado");
    return;
  }
  File file = LittleFS.open(path, "r");
  if (!file) {
    server.send(500, "text/plain", "Erro Interno do Servidor");
    return;
  }
  size_t sent = server.streamFile(file, "text/plain");
  file.close();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  server.send(404, "text/plain", message);
}

void handleSincronizar(){
  if (server.hasArg("ano") && server.hasArg("mes") && server.hasArg("dia") && server.hasArg("hora") && server.hasArg("minuto") && server.hasArg("segundo")) {
    String dia     = server.arg("dia");
    String mes     = server.arg("mes");
    String ano     = server.arg("ano");
    String hora    = server.arg("hora");
    String minuto  = server.arg("minuto");
    String segundo = server.arg("segundo");

    Serial.print("Data sincronizada: ");
    Serial.print(dia + "/" + mes + "/" + ano + " ");
    Serial.println(hora + ":" + minuto + ":" + segundo);

    setTime(hora.toInt(), minuto.toInt(), segundo.toInt(), dia.toInt(), mes.toInt(), ano.toInt());
    server.send(200, "text/plain", "Sincronizado");
    tempoAtualizado = true;
  }
  else {
    server.send(400, "text/plain", "Não foi possível sincronizar");
  }
}

bool iniciarPontoDeAcesso() {
  wifi_fpm_do_wakeup();
  WiFi.forceSleepWake();
  Serial.print("Configurando AP (Ponto de Acesso)…\n");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/dados.txt", HTTP_GET, handleFileStream);
  server.on("/get", HTTP_GET, handleSincronizar);
  server.onNotFound([]() {
    server.send(200, "text/html", index_html);
  });
  server.begin();
  return tempoAtualizado;
}


void encerrarPontoDeAcesso() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
  WiFi.disconnect(true);
  Serial.println("Ponto de acesso encerrado");
  delay(2000);
}

