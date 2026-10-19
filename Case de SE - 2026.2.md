Perfeito. Vamos usar este chat como seu espaço de mentoria para o case da **Estação Meteorológica (EM) da Venturi Aerodesign**.

Como você está no início de Ciência da Computação e “não sabe nada da área”, a gente vai por partes: primeiro entender o que o case pede, depois estudar só o necessário, e por fim montar projeto teórico + protótipo.

## 1. O que o case pede, em resumo

Você precisa projetar e construir uma estação que:

- **Mede**: temperatura do ar, pressão atmosférica e umidade relativa.
- **Calcula**: ponto de orvalho, densidade do ar e altitude-densidade.
- **Atualiza e transmite** tudo a **1 Hz** (1 vez por segundo).
- **Transmite por Wi-Fi 2,4 GHz** via API HTTP.
- **Transmite por 433 MHz** via LoRa (módulo RA-01/RA-02).
- **Armazena** as amostras em memória não volátil, com timestamp.
- **Exibe** em um LCD 20x4.
- **Documenta** código-fonte e esquemático elétrico.

Datas importantes:
- **Entrega do projeto teórico (Etapa 1)**: 28/10/2026 (recomendado).
- **Entrega do protótipo (Etapa 2)**: 09/11/2026.
- Hoje é 15/10/2026, então você tem cerca de **13 dias** para a Etapa 1.

## 2. Hardware que a oficina já tem e que provavelmente será usado

- **ESP32-WROOM-32E**: cérebro do projeto. Tem Wi-Fi 2,4 GHz e processa tudo.
- **DHT11**: umidade e temperatura (mas é pouco preciso).
- **GY-87**: tem o BMP180, que mede pressão e temperatura. Use o BMP180 para pressão e, se possível, para temperatura.
- **RTC DS3231**: relógio de tempo real para gerar o TEMPO.
- **LoRa RA-01/RA-02**: comunicação em 433 MHz.
- **LCD 20x4 com I2C**: visor.
- **Memória**: dá para usar a memória flash interna do ESP32 com LittleFS/SPIFFS.
- **Alimentação**: bateria LiFePO4 6,6 V + regulador LM2596 para 5 V/3,3 V.

## 3. Conceitos que você vai precisar aprender 

Não precisa virar especialista. Vá aprendendo conforme usa:

- **GPIO**: pinos que ligam/desligam e conversam com componentes.
- **I2C**: protocolo de 2 fios usado por BMP180, RTC e LCD.
- **UART**: comunicação serial, usada pelo GPS.
- **SPI**: protocolo usado pelo LoRa.
- **Wi-Fi + HTTP**: para criar a API no ESP32.
- **LoRa 433 MHz**: rádio de longo alcance e baixa taxa.
- **Memória não volátil**: guardar dados mesmo sem energia.
- **Fórmulas meteorológicas**: ponto de orvalho, densidade do ar e altitude-densidade.

