// Bibliotecas de Comunicacao e Matematica
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <math.h>
#include <ESPmDNS.h>

// Sensores
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <LoRa.h>

// Definicao dos Objetos 
Adafruit_BMP085 bmp;
DHT dht(15, DHT11); 
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4); 
WebServer server(80);

// Variaveis Globais 
unsigned long tempo_leitura_dht = 0; // Guarda o tempo da ultima leitura valida do DHT
unsigned long tempo_envio = 0; // Controla o timer principal de 1Hz para leituras e envios
int tempo_dia = 0; // Tempo em segundos desde a meia-noite, Horario de Brasilia
float umidade = 50.0, temp = 0, pressao = 0, orv = 0, dens = 0, altd = 0; // Variaveis das medidas, Umidade comeca com 50 para evitar problemas com log(0)
int arq_num = 1; // Controla qual arquivo CSV esta sendo gravado no momento

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // Inicia o I2C nos pinos 21 (SDA) e 22 (SCL)

  // Inicia DHT11
  dht.begin();

  // Inicia RTC
  if (!rtc.begin()) {
    Serial.println("Erro: RTC nao encontrado.");
    while (1);
  }

  if (rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  }

  // Inicia BMP180
  if(!bmp.begin()){
    Serial.println("Erro: BMP180 nao encontrado.");
    while (1);
  }

  // Inicia LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando ...");

  // Inicia o sistema de arquivos para gravar os dados na flash
  LittleFS.begin(true);

  // Inicia Wi-Fi 
  WiFi.begin("", ""); // Colocar aqui o nome e senha do wifi da equipe
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("IP da EM: ");
  Serial.println(WiFi.localIP());

  // Configura a rota da API HTTP
  server.on("/dados", []() {
    // Monta o JSON com os dados no formato do projeto
    String json = "{";
    json += "\"TEMPO\":" + String(tempo_dia) + ",";
    json += "\"TEMP\":" + String(temp, 1) + ",";
    json += "\"PRES\":" + String(pressao, 1) + ",";
    json += "\"UMID\":" + String(umidade, 1) + ",";
    json += "\"ORV\":" + String(orv, 1) + ",";
    json += "\"DENS\":" + String(dens, 3) + ",";
    json += "\"ALTD\":" + String(altd, 1);
    json += "}";
    
    server.send(200, "application/json", json);
  });

  // Inicia o servidor web
  server.begin();

  // Cria um DNS local para nao precisar digitar o IP
  MDNS.begin("estacaoCase"); // Acessar http://estacaoCase.local/dados para ver o json
  
  // Registra o servico HTTP no mDNS
  MDNS.addService("http", "tcp", 80);

  // Inicia o LoRa
  LoRa.setPins(5, 4, 2); // Pinos do LoRa (CS, RST, DIO0)
  LoRa.begin(433E6); // Inicia o radio 

}

void loop() {
  // Mantem o servidor HTTP ouvindo requisicoes
  server.handleClient();

  DateTime agora = rtc.now();
  unsigned long tempo_atual = millis();
  float umidade_cache; // Variavel temporaria para leitura
  
  // Timer do DHT (2 segundos) 
  if((tempo_atual - tempo_leitura_dht) >= 2000){
    umidade_cache = dht.readHumidity();
    if (!isnan(umidade_cache)){
      umidade = umidade_cache;
    }
    tempo_leitura_dht = tempo_atual;
  }

  // Timer Principal (1 Hz) 
  if ((tempo_atual == 0) || ((tempo_atual - tempo_envio) >= 1000)){
    tempo_envio = tempo_atual;
    
    // Le os sensores
    temp = bmp.readTemperature();
    pressao = bmp.readPressure() / 100.0; // Converte para hPa
    tempo_dia = (agora.hour() * 3600) + (agora.minute() * 60) + agora.second(); 

    // Calcula ORV, DENS e ALTD
    float alpha = (17.27 * temp) / (237.7 + temp) + log(umidade / 100.0); // O ln em math e log
    orv = (237.7 * alpha) / (17.27 - alpha); // Em graus
    
    float pv = 6.112 * exp((17.67 * temp) / (temp + 243.5)) * (umidade / 100.0);
    dens = (pressao * 100.0) / (287.05 * (temp + 273.15)) * (1.0 - (0.378 * pv) / pressao);
    
    altd = 44330.77 * (1.0 - pow(dens / 1.225, 0.235));

    // Atualiza o LCD sem piscar a tela toda
    for(int x = 0; x < 4; x++) {
      lcd.setCursor(0, x); // Vai para a linha x
      lcd.print("                    "); // Preenche com espacos para limpar
    }

    lcd.setCursor(0, 0);
    lcd.print("T: "); lcd.print(temp, 1); 
    lcd.print("C  U: "); lcd.print(umidade, 1); lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("P: "); lcd.print(pressao, 1); lcd.print(" hPa");

    lcd.setCursor(0, 2);
    lcd.print("ORV: "); lcd.print(orv, 1); 
    lcd.print(" DENS: "); lcd.print(dens, 2);

    lcd.setCursor(0, 3);
    lcd.print("ALTD:"); lcd.print(altd, 0); 
    lcd.print("m T:"); lcd.print(tempo_dia); lcd.print("s");

    // Envia os dados pelo radio (nao esquecer da antena) 
    String pacote = String(tempo_dia) + "," + String(temp, 1) + "," + String(pressao, 1) + "," + String(umidade, 1) + "," + String(orv, 1) + "," + String(dens, 3) + "," + String(altd, 1);
    LoRa.beginPacket(); // Prepara o envio
    LoRa.print(pacote); // Coloca os dados no buffer
    LoRa.endPacket();   // Faz a transmissao 

    // Grava na flash de forma circular (dois arquivos de 500KB)
    String nome_arquivo = "/arq_" + String(arq_num) + ".csv";
    File arquivo = LittleFS.open(nome_arquivo, FILE_APPEND);

    if(arquivo){

      if(arquivo.size() > 500000) // Se o arquivo passou do limite de 500KB
      {
        arquivo.close(); // Fecha o arquivo cheio

        arq_num = (arq_num == 1) ? 2 : 1; // Troca para o outro arquivo (1 vira 2, 2 vira 1)

        nome_arquivo = "/arq_" + String(arq_num) + ".csv"; // Atualiza o nome

        LittleFS.remove(nome_arquivo); // Apaga o arquivo velho para liberar espaco

        arquivo = LittleFS.open(nome_arquivo, FILE_APPEND); // Abre o novo arquivo
      }

      // Grava a linha no CSV
      if (arquivo) {
        arquivo.print(tempo_dia); arquivo.print(",");
        arquivo.print(temp, 1); arquivo.print(",");
        arquivo.print(pressao, 1); arquivo.print(",");
        arquivo.print(umidade, 1); arquivo.print(",");
        arquivo.print(orv, 1); arquivo.print(",");
        arquivo.print(dens, 3); arquivo.print(",");
        arquivo.println(altd, 1);
        arquivo.close(); // Fecha para salvar os dados na Flash
      }
    }
  }
}