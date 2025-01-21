#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ultrasonic.h>
#include <SimpleTimer.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

const char* ssid = "Laica-IoT";
const char* password = "Laica321";

const int trigPin = 16;
const int echoPin = 5;
const int distanciaMax = 75;
const int distanciaMin = 15;
const int distancia_Max_Saida = 220;
const int tolerancia = 10;

int contadorPessoas = 0;

Ultrasonic ultrassonico(trigPin, echoPin);
SimpleTimer timer;

// Definindo o display OLED (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

  // Inicializa o display OLED
  Wire.begin(D6, D5); // SDA (D6), SCL (D5)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Endereço padrão 0x3C
    Serial.println(F("Erro ao inicializar o display OLED"));
    while (true);  // Se o display não inicializar, trava
  }
  display.clearDisplay();
  
  // Configura o SimpleTimer para chamar a função requisicao() a cada 1 minuto (60.000 milissegundos)
  timer.setInterval(60000, requisicao);
  coldStart();
}

bool pessoaNoSensor = false;

void contarPessoas() {
  int distancia = ultrassonico.distanceRead();

  display.clearDisplay();
  display.setTextSize(2);  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Distancia:");
  display.setCursor(0, 16); 
  display.print(distancia);
  display.print(" cm");

  while (distancia > distancia_Max_Saida) {
    distancia = ultrassonico.distanceRead();
    delay(100); 
    timer.run();
  }
  
  digitalWrite(2, HIGH);
  if (distancia < distanciaMax && distancia > distanciaMin && !pessoaNoSensor) {
    Serial.println("Distancia no range: " + String(distancia));
    pessoaNoSensor = true;
  }

  if (distancia > (distanciaMax + tolerancia) && pessoaNoSensor) {
    contadorPessoas++;
    Serial.println("Pessoas: " + String(contadorPessoas));
    Serial.println("Distancia saindo do range: " + String(distancia));

    
    display.setCursor(0, 32); 
    display.print("Pessoas:");
    display.setCursor(0, 48); 
    display.print(contadorPessoas);

    display.display();  

    pessoaNoSensor = false;
    digitalWrite(2, LOW);   
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
