# Projeto Teórico

Esse documento contém cada decisão escolhida para a realização da estação meteorológica do case do Processo Seletivo de Sistemas Embarcados.

## Visão Geral do Sistema
O que a Estação Meteorológica (EM) faz:

1. **Mede** → temperatura do ar, pressão atmosférica e umidade relativa.
2. **Calcula** → ponto de orvalho, densidade do ar e altitude-densidade.
3. **Exibe** → em um LCD 20x4, em tempo real.
4. **Armazena** → em memória flash interna, com timestamp.
5. **Transmite** → por Wi-Fi 2,4 GHz (API HTTP) e por LoRa 433 MHz, simultaneamente, a 1 Hz.

### Tabela de Parâmetros
Parâmetros que a EM deve exibir, armazenar e transmitir. 

| Nome  | Parâmetro           | Unidade | Origem    | Descrição                                                                                |
| :---- | :------------------ | :------ | :-------- | :--------------------------------------------------------------------------------------- |
| TEMPO | Tempo               | s       | Relógio   | Base de tempo da amostra, em segundos a partir de 00:00:00 do dia (horário de Brasília). |
| TEMP  | Temperatura do ar   | °C      | Medido    | Temperatura do ar ambiente, à sombra.                                                    |
| PRES  | Pressão atmosférica | hPa     | Medido    | Pressão absoluta no local da estação, sem redução ao nível do mar.                       |
| UMID  | Umidade relativa    | %       | Medido    | Umidade relativa do ar.                                                                  |
| ORV   | Ponto de orvalho    | °C      | Calculado | Temperatura em que o ar atingiria a saturação.                                           |
| DENS  | Densidade do ar     | kg/m³   | Calculado | Massa específica do ar úmido.                                                            |
| ALTD  | Altitude-densidade  | m       | Calculado | Altitude da atmosfera padrão em que a densidade do ar é igual à calculada.               |
### Tabela de Hardware Disponível

| Categoria     | Item                          | Descrição                                                                                                           |
| :------------ | :---------------------------- | :------------------------------------------------------------------------------------------------------------------ |
| Processamento | ESP32-WROOM-32E               | Microcontrolador com Wi-Fi e Bluetooth (2,4 GHz), antena integrada.                                                 |
| Processamento | Arduino                       | Microcontrolador de 5 V, hoje usado na estação de solo.                                                             |
| Sensores      | GY-87                         | Módulo I2C com barômetro BMP180 (pressão e temperatura), acelerômetro e giroscópio MPU6050 e magnetômetro HMC5883L. |
| Sensores      | DHT11                         | Temperatura e umidade relativa do ar, saída digital.                                                                |
| Sensores      | GPS NEO-6MV2                  | Posição, altitude e horário UTC, via UART.                                                                          |
| Sensores      | WCS-1800                      | Corrente por efeito Hall, saída analógica.                                                                          |
| Sensores      | HMC5883L                      | Magnetômetro de 3 eixos, I2C.                                                                                       |
| Sensores      | 3144E                         | Chave de efeito Hall, saída digital.                                                                                |
| Comunicação   | LoRa RA-01 / RA-02            | Rádio LoRa de 433 MHz com interface SPI.                                                                            |
| Apoio         | RTC DS3231                    | Relógio de tempo real com bateria própria, I2C.                                                                     |
| Apoio         | ADS1115                       | Conversor analógico-digital de 16 bits, I2C.                                                                        |
| Apoio         | Conversor de nível lógico     | Bidirecional de 4 canais (BSS138), entre 3,3 V e 5 V.                                                               |
| Apoio         | Placa padrão ilhada           | Para montagem soldada do circuito.                                                                                  |
| Alimentação   | LM2596                        | Regulador DC-DC abaixador (buck).                                                                                   |
| Alimentação   | MT3608                        | Regulador DC-DC elevador (boost).                                                                                   |
| Alimentação   | Bateria LiFePO4 6,6 V 700 mAh | Zippy Compact.                                                                                                      |
| Visor         | LCD 20x4                      | Display de caracteres com 20 colunas e 4 linhas, com adaptador I2C.                                                 |

### Especificação do hardware (Só os usados)

 ESP32-WROOM-32E
 
- Processador : Xtensa dual-core 32-bit LX6 running up to 240 MHz (80 MHz to 240 MHz adjustable).
- Memória : 520 KB SRAM, 448 KB ROM, and typically 4 MB, 8 MB, or 16 MB of SPI flash.
- Conectividade: Wi-Fi 802.11 b/g/n (up to 150 Mbps) and Bluetooth v4.2 BR/EDR + BLE (Bluetooth Low Energy).
- Periféricos: Up to 26 GPIOs supporting ADC, DAC, I2C, I2S, SPI, UART, PWM, and CAN (TWAI).
- Antena: Usa uma antena PCB integrada.
- Voltagem de operação: 3.0V to 3.6V (logic level is 3.3V; it is **not** 5V tolerant)

Fonte: https://documentation.espressif.com/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.html

GY-87
Módulo de sensores que utilizam protocolo I2C. (Combina 3 chips de sensores diferentes) (Tem regulador de tensão)

Sensores Integrados no GY-87:

- **MPU6050:** Combina um **Acelerômetro de 3 eixos** (para medir inclinação e quedas) e um **Giroscópio de 3 eixos** (para medir a velocidade de rotação orbital).
- **HMC5883L:** Um **Magnetômetro de 3 eixos** (bússola digital) usado para determinar a orientação em relação ao norte magnético da Terra.
- **BMP180:** Um **Barômetro digital** que mede a pressão atmosférica e a temperatura ambiente. Ele permite calcular a altitude relativa com boa precisão.

 Endereços I2C Padrão:
    - MPU6050: `0x68` ou `0x69`
    - HMC5883L: `0x3D`(read) `0x3C`(write)
    - BMP180: `0x77`


| **VCC_IN** (ou VCC) | **3V3** (ou 5V) | Alimentação principal. A placa possui regulador interno de tensão LDO. |
| ------------------- | --------------- | ---------------------------------------------------------------------- |
| **GND**             | **GND**         | Referência de aterramento comum.                                       |
| **SDA**             | **GPIO 21**     | Linha de dados I2C. _(Aqui passa o sinal de todos os sensores)._       |
| **SCL**             | **GPIO 22**     | Linha de clock I2C. _(Aqui passa o sinal de todos os sensores)._       |

MPU6050

| Atributo               | Acelerômetro                                       | Giroscópio                                        |
| :--------------------- | :------------------------------------------------- | :------------------------------------------------ |
| **O que mede**         | Aceleração linear e gravidade (eixos X, Y, Z).     | Velocidade de rotação (eixos X, Y, Z).            |
| **Faixa de Medição**   | ±2g, ±4g, ±8g ou ±16g                              | ±250, ±500, ±1000 ou ±2000 °/s                    |
| **Erro / Exatidão**    | ±3% (calibração inicial) e ±50mg a ±80mg (Zero-G). | ±3% (sensibilidade) e ±20% (erro de repouso/ZRO). |
| **Tensão de Operação** | VDD: 2,375 V a 3,46 V (VLOGIC: 1,71 V a VDD)       | VDD: 2,375 V a 3,46 V (VLOGIC: 1,71 V a VDD)      |
| **Unidade**            | **g** (gravidade)                                  | **°/s** (graus por segundo)                       |

HMC5883L

| Atributo | Especificação |
| :--- | :--- |
| **O que mede** | Campo Magnético da Terra (eixos X, Y, Z). |
| **Faixa de Medição** | ±8 Gauss |
| **Erro / Exatidão** | Resolução de 2 mG (mili-Gauss) e erro de direção de 1° a 2°. |
| **Tensão de Operação** | VDD: 2,16 V a 3,6 V (VDDIO: 1,71 V a VDD) |
| **Unidade** | **Gauss** (G) ou **mG** |
BMP180

| Atributo               | Pressão Atmosférica                                     | Temperatura                                |
| :--------------------- | :------------------------------------------------------ | :----------------------------------------- |
| **O que mede**         | Pressão absoluta do ar no local.                        | Temperatura do ar ambiente.                |
| **Faixa de Medição**   | 300 a 1100 hPa (+9000m ... -500m relating to sea level) | -40 °C a +85 °C                            |
| **Erro / Exatidão**    | ±0,12 hPa (relativa) / ±2,0 hPa (absoluta)              | ±0,5 °C (na faixa de 0 a 65 °C)            |
| **Tensão de Operação** | VDD: 1,8 V a 3,6 V (VDDIO: 1,62 V a 3,6 V)              | VDD: 1,8 V a 3,6 V (VDDIO: 1,62 V a 3,6 V) |
| **Unidade**            | **hPa** (hectopascal)                                   | **°C** (graus Celsius)                     |

Fontes
MPU6050: https://www.invensense.tdk.com/en-us/search-result?query=PS-MPU-6000A-00%20%E2%80%93%20MPU-6000%20and%20MPU-6050%20Datasheet
HMC5883L: https://cdn-shop.adafruit.com/datasheets/HMC5883L_3-Axis_Digital_Compass_IC.pdf
BMP180: https://airu.coe.utah.edu/wp-content/uploads/sites/62/2017/01/BMP180-Digital-TempPressureAlt.pdf

DHT11

| Atributo              | Especificação (Umidade)                 | Especificação (Temperatura)      |
| :-------------------- | :-------------------------------------- | :------------------------------- |
| **O que mede**        | Umidade Relativa do ar (UR).            | Temperatura do ar ambiente.      |
| **Faixa de Medição**  | 20% a 90% UR                            | 0 °C a 50 °C                     |
| **Erro / Exatidão**   | ±5% UR (em 0-50 °C)<br>±4% UR (a 25 °C) | ±2 °C                            |
| **Resolução**         | 1% UR (saída de 8 bits)                 | 1 °C (saída de 8 bits)           |
| **Repetibilidade**    | ±1% UR                                  | ±1 °C                            |
| **Tempo de Resposta** | 6 s a 30 s (para 63% da mudança)        | 6 s a 30 s (para 63% da mudança) |

| Atributo                           | Especificação                                                                                 |              |                   |             |
| :--------------------------------- | :-------------------------------------------------------------------------------------------- | ------------ | ----------------- | ----------- |
| **Tensão de Operação (VDD)**       | 3,0 V a 5,5 V DC (Típico: 5 V)                                                                |              |                   |             |
| **Consumo de Corrente**            | Medindo: 0,5 mA a 2,5 mA<br>Média: 0,2 mA a 1 mA<br>Standby (Repouso): 100 µA a 150 µA        |              |                   |             |
| **Interface**                      | Single-Bus (1 fio de dados bidirecional)                                                      |              |                   |             |
| **Taxa de Atualização (Sampling)** | Não pode ser menor que 1 segundo.                                                             |              |                   |             |
| **Pacote de Dados**                | 40 bits (5 bytes): 8b Umidade Int + 8b Umidade Dec + 8b Temp Int + 8b Temp Dec + 8b Checksum. |              |                   |             |
| **Pinagem**                        | Pino 1: VDD                                                                                   | Pino 2: DATA | Pino 3: NC (Nulo) | Pino 4: GND |

1. Atraso no Boot: Ao ligar a energia, o sensor precisa de 1 segundo no modo instável antes de receber qualquer comando.
2. Resistor de Pull-up: O pino de dados (DATA) precisa de um resistor de pull-up de 5 kΩ ligado ao VDD para funcionar. (Depende)
3. Tempo de Leitura : a taxa de leitura não deve ser menor que 1 segundo. Se o código tentar ler mais rápido que isso, ele vai retornar erro (NaN).


Fonte
DTH11: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf

RTC DS3231
Relógio de tempo real com bateria própria.

- **Tensão de Alimentação Principal ($V_{CC}$):** **2,3V a 5,5V** (totalmente compatível com lógicas de 3,3V e 5V).
- **Tensão da Bateria de Backup ($V_{BAT}$):** **2,3V a 5,5V** (projetado para células de lítio de 3V).
- **Corrente de Consumo em Standby (Modo Bateria):** Menor que **1,0 μA** com o oscilador ativo.
- **Interface de Comunicação:** Protocolo **I2C** em modo rápido (_Fast Mode_) de até **400 kHz**.
- **Endereço I2C Padrão:** Fixo em **`0x68`**.

Pinos obrigatórios

- **VCC:** Pino de alimentação principal.
- **GND:** Pino de aterramento (referência negativa).
- **SDA (Serial Data):** Linha de envio e recebimento de dados do protocolo I2C.
- **SCL (Serial Clock):** Linha de sincronismo (clock) do protocolo I2C.

Fonte 
RTC DS3231: https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf

LoRa RA-01

- **Chip Base:** Semtech SX1278 
- **Frequência de Operação:** **410 MHz a 525 MHz** (Frequência central padrão em **433 MHz**) 
- **Interface de Comunicação:** **Protocolo SPI** (Half-Duplex)
- **Tensão de Alimentação:** **2,5V a 3,7V** (Tensão típica nominal de **3,3V**) 
- **Potência de Saída RF Máxima:** +20 dBm (ajustável via código) 
- **Sensibilidade do Receptor:** Até **-141 dBm** 
- **Consumo de Corrente:**
    - **Transmissão (TX):** ~93 mA
    - **Recepção (RX):** ~12,15 mA
    - **Modo de Espera (Standby):** ~1,6 mA 
- **Antena de fábrica:** Antena helicoidal de mola soldada diretamente na placa.

pinagem do LoRa RA-01

| Pino   | Nome     | Função / Onde conectar no ESP32                                                     |
| ------ | -------- | ----------------------------------------------------------------------------------- |
| **1**  | **GND**  | Aterramento (GND do ESP32)                                                          |
| **2**  | **DIO1** | Pino E/S Digital (Opcional - Usado para funções avançadas de timeout/CAD)           |
| **3**  | **DIO2** | Pino E/S Digital (Opcional - Usado em modulações FSK/OOK)                           |
| **4**  | **DIO3** | Pino E/S Digital (Opcional)                                                         |
| **5**  | **DIO4** | Pino E/S Digital (Opcional)                                                         |
| **6**  | **DIO5** | Pino E/S Digital (Opcional)                                                         |
| **7**  | **3.3V** | Alimentação positiva (**3V3 do ESP32**)                                             |
| **8**  | **GND**  | Aterramento (GND do ESP32)                                                          |
| **9**  | **REST** | Pino de Reset (Pode ser ligado em qualquer GPIO livre do ESP32, ex: **GPIO 14**)    |
| **10** | **DIO0** | **Interrupção Principal** (Obrigatório! Ligue em um GPIO do ESP32, ex: **GPIO 26**) |
| **11** | **NSS**  | SPI Chip Select (Ligue em um GPIO do ESP32, ex: **GPIO 5**)                         |
| **12** | **SCK**  | SPI Clock (**GPIO 18** do ESP32)                                                    |
| **13** | **MISO** | SPI Master Input Slave Output (**GPIO 19** do ESP32)                                |
| **14** | **MOSI** | SPI Master Output Slave Input (**GPIO 23** do ESP32)                                |
| **15** | **ANT**  | Saída para antena (já conectada à mola no módulo comum)                             |
| **16** | **GND**  | Aterramento (GND do ESP32)                                                          |
Bateria LiFePO4 6,6 V 700 mAh

Regulador DC-DC abaixador (buck).

LCD 20x4
