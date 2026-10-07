#include <Adafruit_Sensor.h>
//#include <DHT.h>
#include <DHT_U.h>
#include "auxiliaresLittleFS.h"
#include "auxiliaresWifi.h"
//#include <LittleFS.h>


const int botao = 5;
const int ledIndicadorPin = 13; 
bool botaoPressionado = false;
unsigned long tempoBotaoPressionado = 0;
const int tempoFormatacao = 10000; // tempo que o botão deve ser pressionado para formatar

bool apWifiLigado = false;
unsigned long tempoInicializacaoWifi = 0;
const unsigned long tempoMaximoWifi = 5 * 60 * 1000; // 10 minutos

#define DHTPIN D5
#define DHTTYPE DHT11 
DHT_Unified dht(DHTPIN, DHTTYPE); 

FSInfo fs_info; // informações básicas sobre o sistema de arquivos

void setup() {
  delay(5000); //debug

  pinMode(botao, INPUT_PULLUP); // botão de wi-fi e formatação
  pinMode(ledIndicadorPin, OUTPUT); // led indicador

  Serial.begin(115200);
  dht.begin();
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  bool arquivoExiste = LittleFS.exists("/dados.txt");
  listDir("/");
  if(arquivoExiste){
    Serial.println("O arquivo dados.txt já existe e será mantido");
  }
  else{
    writeFile("/dados.txt", " ");
  }
  Serial.print("\n");
  listDir("/");
  Serial.print("\n");

  uint16_t ano = year();
  Serial.printf("Ano atual: %d", ano);
  if (ano == 1970) {
  Serial.printf(" (Desatualizado)\n");
  tempoAtualizado = false;
  }
  iniciarPontoDeAcesso();
  apWifiLigado = true;
  tempoInicializacaoWifi = millis();
}

void loop() {
  if (apWifiLigado && (millis() - tempoInicializacaoWifi >= tempoMaximoWifi)){
    encerrarPontoDeAcesso();
    apWifiLigado = false;
  }

  int estadoBotao = digitalRead(botao);

  float temperatura;
  float umidade;

  delay(1000);

  // executa formatação ao pressionar o botão por mais de  10 segundos
  if(estadoBotao == LOW && !botaoPressionado){
    botaoPressionado = true;
    tempoBotaoPressionado = millis();
  }
  else if (estadoBotao == LOW && botaoPressionado) {
    unsigned long tempoDecorrido = millis() - tempoBotaoPressionado;
    if(tempoBotaoPressionado >= tempoFormatacao){
      Serial.println(F("Formatação iniciada\n"));
      for (int i = 0; i<50; i++){
        digitalWrite(ledIndicadorPin, LOW);
        delay(100);
        digitalWrite(ledIndicadorPin, HIGH);
        delay(100);
      }
      formatar();
    }
  }
  else if(estadoBotao == HIGH && botaoPressionado){
    botaoPressionado = false;
  }

  if (tempoAtualizado) {
    digitalWrite(ledIndicadorPin, LOW);

    uint16_t ano = year();  
    uint8_t mes = month();
    uint8_t dia = day();
    uint8_t hora = hour();
    uint8_t minuto = minute();
    uint8_t segundo = second();

    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Erro ao ler a temperatura"));
    }
    else {
      temperatura = event.temperature;
      Serial.print("Temperatura: " + (String)temperatura + "°C  |  ");
    }
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Erro ao ler a umidade!"));
    }
    else {
      umidade = event.relative_humidity;
      Serial.print("Umidade: " + (String)umidade + "% | ");
    }

    String tempo = (String)dia + "/" + (String)mes + "/" + (String)ano + "-" + (String)hora + ":" + (String)minuto + ":" + (String)segundo;
    Serial.print("Tempo: " + (String)tempo);

    if(segundo == 0 || segundo == 15 || segundo == 30 || segundo == 45){
      LittleFS.info(fs_info); // atualiza informações sobre o armazenamento
      String dados = (String)temperatura + " " + (String)umidade + " " + tempo + "\n";
      Serial.print(" | ");
      Serial.print(fs_info.usedBytes);
      Serial.print("/");
      Serial.print(fs_info.totalBytes);
      appendFile("/dados.txt", dados.c_str());
    }
    Serial.print("\n");

    if(segundo == 0){
      lerArquivo("/dados.txt"); // improviso para salvar os dados permanentemente
    }
  }
  else{
    digitalWrite(ledIndicadorPin, HIGH);
    Serial.println("Tempo não configurado.");
  }

  // inicia ponto de acesso wi-fi e pisca o led ao pressionar o botão
  if (estadoBotao == LOW){
    iniciarPontoDeAcesso();
    apWifiLigado = true;
    tempoInicializacaoWifi = millis();
    Serial.println("Botão pressionado.");
    for (int i = 0; i<10; i++){
      digitalWrite(ledIndicadorPin, LOW);
      delay(250);
      digitalWrite(ledIndicadorPin, HIGH);
      delay(250);
    }
  }

  // if((fs_info.usedBytes * 0.9) >= fs_info.totalBytes){
  //   deleteFile("/dados.txt");
  //   writeFile("/dados.txt", " ");
  //   String dados = "Arquivo anterior apagado por falta de armazenamento";
  //   appendFile("/dados.txt", dados.c_str());
  // }

  dnsServer.processNextRequest();
  server.handleClient(); 
}