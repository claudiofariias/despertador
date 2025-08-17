# 💊 Sistema de Lembrete de Medicamentos (ESP32 + Telegram + MQTT)

## 📌 Descrição do Projeto
Este projeto é um **Sistema de Lembrete de Medicamentos** que integra um **bot do Telegram**, um **servidor Node.js** e um **dispositivo ESP32**.  
A ideia é permitir que o usuário configure alarmes de medicamentos diretamente pelo Telegram, que são enviados ao ESP32 via MQTT.  
O ESP32 aciona um **alarme sonoro (buzzer)** e **visual (LED)** no horário programado, exibindo também as informações em um **display OLED**.

### ⚙️ Funcionalidades
- Configuração de alarmes via Telegram com o comando `/add`.
- Listagem de alarmes ativos com `/list`.
- Remoção de alarmes com `/clear`.
- O ESP32 recebe os alarmes via MQTT e dispara buzzer/LED no horário definido.
- Botão físico no ESP32 para cancelar o alarme.
- Display OLED mostrando hora atual e próximos alarmes.

---

## 🚀 Como Executar o Projeto

### 🔹 Requisitos
- **Node.js** (>= 18)
- **npm** ou **yarn**
- **Arduino IDE** ou **PlatformIO**
- **Broker MQTT** (pode ser o público `broker.hivemq.com`)
- **ESP32** com display OLED SSD1306, buzzer, LED e botão físico

### 🔹 Passo a Passo

#### 1. Clonar este repositório
```bash
git clone https://github.com/seu-usuario/medicine-reminder.git
cd medicine-reminder
```

#### 2. Configurar o Servidor Node.js
Entre na pasta `server/` e instale as dependências:
```bash
cd server
npm install
```

Crie um arquivo `.env` com:
```
TELEGRAM_TOKEN=seu_token_telegram
MQTT_URL=mqtt://broker.hivemq.com
PORT=3000
```

Inicie o servidor:
```bash
node server.js
```

#### 3. Configurar a ESP32
- Abra o código `esp32_medicine_reminder.ino` no Arduino IDE.
- Configure seu Wi-Fi e credenciais MQTT:
```cpp
const char* ssid = "SEU-WIFI";
const char* pass = "SUA-SENHA";
const char* mqtt_server = "broker.hivemq.com";
```
- Selecione a placa **ESP32 Dev Module** e faça o upload.

#### 4. Usar o Bot do Telegram
- No Telegram, inicie uma conversa com o bot criado no **BotFather**.
- Comandos disponíveis:
  - `/add` → Adicionar novo alarme
  - `/list` → Listar alarmes
  - `/clear` → Remover todos alarmes

---

## 🛠️ Tecnologias Utilizadas
- ESP32 (Arduino Core)
- Node.js + Express
- MQTT (HiveMQ ou Mosquitto)
- Telegram Bot API
- Display OLED SSD1306
- Buzzer + LED + Botão físico

---

## 📷 Imagem do Sistema em Funcionamento
![Exemplo](/images/prototipo/3.jpeg)

---

## 👨‍💻 Autor
Projeto desenvolvido para estudo e aplicação prática de **sistemas embarcados** e **IoT**.  
