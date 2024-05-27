#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Laica-IoT";
const char* password = "Laica321";

#include <Ultrasonic.h>
const int trigPin = 16;
const int echoPin = 5;
const int distanciaMax = 30;
const int distanciaMin = 5;
const int tolerancia = 5;

int contadorPessoas = 0;

Ultrasonic ultrassonico(trigPin, echoPin);

void setup() {
  Serial.begin(9600);

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  if (WiFi.status() == WL_CONNECTED) {
    
    int distancia = 500;
    while (distancia > distanciaMax) {
      distancia = ultrassonico.distanceRead();
    }

    
    if (Serial.available() > 0) {
      
      int dado = Serial.read();
     
      HTTPClient http;
      WiFiClient client;
      
      http.begin(client,"http://10.25.1.162:5000/Ambiente");

  
      http.addHeader("Content-Type", "application/json");

      String requisicao;

      
      if (dado == '1') {
        if (distancia < distanciaMax + tolerancia && distancia > distanciaMin) {
          contadorPessoas++;
          Serial.println("Pessoas: " + String(contadorPessoas));  
          requisicao = "{\"count\": " + String(contadorPessoas) + "}";
          delay(1000);
        }
      } else {
        String mensagem = "Sensor desligado";
        Serial.println(mensagem);
       requisicao = "{\"mensagem\": \"" + mensagem + "\"}";
      }

     
      int httpCode = http.POST(requisicao);

      
      if (httpCode > 0) {
        Serial.print("HTTP response code: ");
        Serial.println(httpCode);
        String response = http.getString();
        Serial.println("Response: " + response);
      } else {
        Serial.println("Error on HTTP request");
        Serial.println(httpCode);
      }

     
      http.end();

      delay(1000);
    }
  } 
}