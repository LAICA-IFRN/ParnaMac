#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ultrasonic.h>
#include <SimpleTimer.h>

const char* ssid = "Laica-IoT";
const char* password = "Laica321";

const int trigPin = 16;
const int echoPin = 5;
const int distanciaMax = 75;
const int distanciaMin = 15;
//const int tolerancia = 5;

int contadorPessoas = 0;

Ultrasonic ultrassonico(trigPin, echoPin);
SimpleTimer timer;

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  
  // Conecta-se ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  // Configura o SimpleTimer para chamar a função requisicao() a cada 1 minuto (60.000 milissegundos)
  timer.setInterval(60000, requisicao);
  coldStart();
}

void contarPessoas() {
  int distancia = ultrassonico.distanceRead();
  while (distancia > distanciaMax) {
    distancia = ultrassonico.distanceRead();
    delay(100); 
    timer.run();  // Adiciona esta linha para garantir que o SimpleTimer continue a funcionar
  }
  if (distancia < distanciaMax && distancia > distanciaMin) {
    contadorPessoas++;
    Serial.println("Pessoas: " + String(contadorPessoas));
    delay(1000); // Pequeno delay para evitar contagens múltiplas da mesma pessoa
  }
}

void requisicao() {
  Serial.println("Enviando dados...");
  
  // Verifica se está conectado ao WiFi
  if (WiFi.status() == WL_CONNECTED) {
    // Cria um objeto WiFiClient
    WiFiClient client;
    HTTPClient http;
    // Inicializa a conexão HTTP
    http.begin(client, "http://laica.ifrn.edu.br/access-ng/log/");
    http.addHeader("Content-Type", "application/json");

    String body = "{\"deviceMac\": \"";
    body += WiFi.macAddress();
    body += "\",\"topic\": \"Contador\", \"type\": \"INFO\",\"message\": \"Pessoas = ";
    body += String(contadorPessoas);
    body += "\"}";

    int httpCode = http.POST(body);
    digitalWrite(2, HIGH);

    // Verifica o código de resposta HTTP
    if (httpCode > 0) {
      Serial.print("HTTP response code: ");
      Serial.println(httpCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on HTTP request");
    }

    // Finaliza a conexão HTTP
    http.end();
    digitalWrite(2, LOW);

    // Resetar o contador de pessoas após enviar os dados
    contadorPessoas = 0;
  }
}

void loop() {
  contarPessoas();

  // Executa as tarefas do SimpleTimer
  timer.run();
}

void coldStart() {
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://laica.ifrn.edu.br/access-ng/log/");  // HTTP
    http.addHeader("Content-Type", "application/json");
    String body = "{\"deviceMac\": \"";
    body += WiFi.macAddress();
    body += "\",\"topic\": \"ANGeLUS\", \"type\": \"INFO\",\"message\": \"Cold Start\"}";
    int httpCode = http.POST(body);
    if (httpCode > 0) {
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      } else {
      digitalWrite(2, HIGH);
      delay(1000);
      ESP.restart();
      }
  }
}
