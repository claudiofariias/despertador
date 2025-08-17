const mqtt = require('mqtt');
const TelegramBot = require('node-telegram-bot-api');
const dotenv = require('dotenv');
const express = require('express');
const app = express();

app.get('/', (req, res) => {
  res.send('Bot rodando 🚀');
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Servidor web iniciado na porta ${PORT}`);
});

// Configuração do ambiente
dotenv.config();

// Configurações MQTT
const MQTT_CONFIG = {
  url: process.env.MQTT_URL || 'mqtt://broker.hivemq.com',
  topics: {
    add: 'medicine_reminder/add',
    clear: 'medicine_reminder/clear',
    list: 'medicine_reminder/list',
    status: 'medicine_reminder/status'
  }
};

// Inicialização do Bot do Telegram
const bot = new TelegramBot(process.env.TELEGRAM_TOKEN, { polling: true });

// Conexão MQTT
const mqttClient = mqtt.connect(MQTT_CONFIG.url);

// Armazenamento local dos alarmes
let alarmes = [];

// Comandos do bot
bot.onText(/\/start/, (msg) => {
  const welcomeMsg = `💊 *Sistema de Lembrete de Medicamentos*\n\n` +
                    `/add - Adicionar novo alarme\n` +
                    `/list - Listar alarmes ativos\n` +
                    `/clear - Remover todos os alarmes`;
  bot.sendMessage(msg.chat.id, welcomeMsg, { parse_mode: 'Markdown' });
});

bot.onText(/\/add/, (msg) => {
  bot.sendMessage(msg.chat.id, "Envie o horário e medicamento no formato:\n\n`HH:MM Medicamento`\n\nExemplo: `08:00 Paracetamol 500mg`", {
    parse_mode: 'Markdown',
    reply_markup: { force_reply: true }
  }).then((sentMsg) => {
    bot.onReplyToMessage(sentMsg.chat.id, sentMsg.message_id, (reply) => {
      const [time, ...medicineParts] = reply.text.split(' ');
      const medicine = medicineParts.join(' ');
      
      if (!time.match(/^\d{2}:\d{2}$/) || !medicine) {
        return bot.sendMessage(msg.chat.id, "Formato inválido. Use: `HH:MM Medicamento`", { parse_mode: 'Markdown' });
      }

      const [hour, minute] = time.split(':');
      const alarme = { hour, minute, medicine };
      
      mqttClient.publish(MQTT_CONFIG.topics.add, JSON.stringify(alarme));
      alarmes.push(alarme);
      
      bot.sendMessage(msg.chat.id, `✅ Alarme adicionado:\n⏰ ${time} - ${medicine}`);
    });
  });
});

bot.onText(/\/list/, (msg) => {
  if (alarmes.length === 0) {
    return bot.sendMessage(msg.chat.id, "Nenhum alarme configurado.");
  }

  const lista = alarmes.map(a => `⏰ ${a.hour}:${a.minute} - ${a.medicine}`).join('\n');
  bot.sendMessage(msg.chat.id, `💊 *Alarmes Ativos*\n\n${lista}`, { parse_mode: 'Markdown' });
});

bot.onText(/\/clear/, (msg) => {
  mqttClient.publish(MQTT_CONFIG.topics.clear, '1');
  alarmes = [];
  bot.sendMessage(msg.chat.id, "Todos os alarmes foram removidos.");
});

// Tratamento MQTT
mqttClient.on('connect', () => {
  console.log('Conectado ao MQTT');
  mqttClient.subscribe(Object.values(MQTT_CONFIG.topics));
});

mqttClient.on('message', (topic, message) => {
  const msg = message.toString();
  
  if (topic === MQTT_CONFIG.topics.status) {
    // Tratar status do dispositivo
    console.log(`Status: ${msg}`);
  }
});

// Tratamento de erros
mqttClient.on('error', (err) => {
  console.error('Erro MQTT:', err);
});

bot.on('polling_error', (error) => {
  console.error('Erro Telegram:', error);
});

console.log('Bot iniciado. Aguardando comandos...');
