#include <WiFi.h>
#include <PubSubClient.h>

// Configurações Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA_WIFI";

// Configurações MQTT
const char* mqtt_server = "IP_DO_SERVIDOR_MQTT";
const int mqtt_port = 1883;
const char* mqtt_user = "USUARIO_MQTT";      // se necessário
const char* mqtt_password = "SENHA_MQTT";   // se necessário

// Tópicos MQTT
const char* topic_sensor = "sistema/umidade";
const char* topic_comando = "sistema/comando";

// Pinos
const int sensorPin = 34;      // ADC GPIO34
const int relePin = 26;        // GPIO conectado ao relé (controle da bomba)

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, LOW); // bomba desligada inicialmente

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Lendo a umidade do solo
  int umidade = analogRead(sensorPin);
  Serial.print("Umidade do solo: ");
  Serial.println(umidade);

  // Envia o dado do sensor para a nuvem
  char msg[50];
  sprintf(msg, "%d", umidade);
  client.publish(topic_sensor, msg);

  // Verifica se recebeu comando para ligar/desligar a bomba
  // Isso é feito via callback (definido abaixo)

  delay(60000); // espera 1 minuto antes da próxima leitura
}

void connectWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado");
      // Inscreve-se no tópico de comandos
      client.subscribe(topic_comando);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

// callback para receber comandos
void callback(char* topic, byte* payload, unsigned int length) {
  String msgTemp;

  for (int i=0; i<length; i++) {
    msgTemp += (char)payload[i];
  }
  Serial.print("Mensagem recebida: ");
  Serial.println(msgTemp);

  if (msgTemp == "LIGAR") {
    digitalWrite(relePin, HIGH); // Liga a bomba
  } else if (msgTemp == "DESLIGAR") {
    digitalWrite(relePin, LOW);  // Desliga a bomba
  }
}

// seta a função de callback
void setupCallbacks() {
  client.setCallback(callback);
}
