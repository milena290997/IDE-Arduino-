#include <WiFi.h>   // Ou ESP32
#include <PubSubClient.h>

const char* ssid = "SEU_SSID";          // Sua rede WiFi
const char* password = "SUA_SENHA";     // Sua senha WiFi

const char* mqtt_server = "66190068545d4e5fbaaebebef1be801d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;

const char* mqtt_user = "SEU_USUARIO";        // Credenciais MQTT
const char* mqtt_password = "SUA_SENHA";

WiFiClientSecure espClient;
PubSubClient client(espClient);

const int sensorPin = A0;
const int bombaPin = 9;
const int ledPin = LED_BUILTIN;

int umidade;
const int threshold = 500;

void setup() {
  Serial.begin(115200);
  pinMode(bombaPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(bombaPin, LOW);  // bomba desligada inicialmente
  WiFi.begin(ssid, password);
  
  // Conexão WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
  
  // Configuração MQTT
  espClient.setInsecure(); // Para conexão TLS, use o método adequado para certificados
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  connectMQTT();
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Conectando ao MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("MQTT conectado");
      client.subscribe("sensor/comando"); // Tópico para receber comandos
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i=0; i<length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Mensagem recebida: ");
  Serial.println(msg);

  if (msg == "sensor : verificar_umidade") {
    // Lê o valor do sensor
    umidade = analogRead(sensorPin);
    Serial.print("Umidade: ");
    Serial.println(umidade);

    if (umidade < threshold) {
      // Ativa bomba e LED
      digitalWrite(ledPin, HIGH);
      digitalWrite(bombaPin, HIGH);
      // Envia mensagem de início de irrigação
      client.publish("sensor/resposta", "umidade abaixo, começando a irrigar");
      delay(10000); // Bomba ligada por 10 segundos
      digitalWrite(bombaPin, LOW);
      digitalWrite(ledPin, LOW);
    }
  }
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
}

