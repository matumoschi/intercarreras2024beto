//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//https://arduino.esp8266.com/stable/package_esp8266com_index.json

#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#include "imagenes.h"
//#include <avr/io.h>


#define i2c_Address 0x3c 


extern Adafruit_SH1106G display;

const char* ssid = "PLATINI";
const char* password = "capitan02";

int t;
int h;

int imageNumber = 1;
int nivelVida = 100;

unsigned long lastUpdate = 0;  // Para controlar la frecuencia de actualización de la pantalla
const long animationInterval = 1000; // Intervalo para actualizar la pantalla (cada 1 segundo)

#define DHTPIN 14
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 

const char broker[] = "test.mosquitto.org";
int port = 1883;
const char topic[] = "test23";
const char topic2[] = "test24";

int LDR_pin = A0;       
int LDR_val = 0;        
const int LED_pin = 13; 
const int ventiladorPin = 15; 
const int ledPin = 14;  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);


#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1   

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0; 
const long interval = 6000;

unsigned long prevOLEDMillis = 0; 
const long intervalOLED = 1000;


const char CienporY = 6;
const char CienporH = 52;
const char SetporY = 17;
const char SetporH = 41;
const char CinporY = 29;
const char CinporH = 29;
const char VeinporY = 43;
const char VeinporH = 15;
const char CerporY = 56;
const char CerporH = 2;


unsigned long lastSendMillis = 0; // Variable para almacenar el tiempo de la última publicación


void displayImage(int imageNumber) {
    display.clearDisplay();

    switch (imageNumber) {
        case 1: // Sano
            imagencomun();
            break;
        case 2: // Hambre
            imagenSueno();
            break;
        case 3: // Sed
            imagenEnfermo();           
            break;
        case 4: // Cansado
            imagenCansado();
            break;
        case 5: // Enfermo
            imagenFeliz();
            break;
        case 6: // Dormido
            imagenTriste();
            break;
        case 7: // Muerto
            imagenCalor();
            break;
        case 8: // Feliz
            imagenMuerte();
            break;
        //case 9: // Comer
            //imagenComida();
            //break;
        //case 10: // Tomar agua
            //imagenAgua();
            //break;
        //case 11: // Dormir
            //imagenSueno();
            //break;
        //case 12: // Medicina
            //imagenMedicina();
            //break;
        //case 13: // Revivir
            //imagenRevivir();
            //break;
        default: // Imagen por defecto
            imagencomun();
            break;
    }

    display.display();
}


// Dentro del callback MQTT, sólo guardamos el estado y el nivel de vida
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en el tópico: ");
    Serial.println(topic);
    
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0'; 

    Serial.print("Mensaje: ");
    Serial.println(message);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("Error al deserializar el JSON: ");
        Serial.println(error.f_str());
        return;
    }

    // Guardar los valores recibidos en las variables globales
    imageNumber = doc["estado"]; // Actualizar el número de la imagen
    nivelVida = doc["nivelVida"]; // Actualizar el nivel de vida

    // Manejar el estado del ventilador
    int ventiladorState = doc["prenderVentilador"];
    int ventiladorState2 = doc["apagarVentilador"];
    if (ventiladorState == 1) {
        digitalWrite(ventiladorPin, HIGH);
        Serial.println("Ventilador encendido.");
    } else if (ventiladorState2 == 1) {
        digitalWrite(ventiladorPin, LOW);
        Serial.println("Ventilador apagado.");
    }
}



void setup() {


    pinMode(ventiladorPin, OUTPUT);
    digitalWrite(ventiladorPin, LOW);
    pinMode(LED_pin, OUTPUT);
    pinMode(ledPin, OUTPUT);

    Serial.begin(115200);
    Serial.println(F("DHT 11 - Prueba de conexión con el servidor"));
    dht.begin();

    // Cambiar la configuración de WiFi para ESP32
    WiFi.begin(ssid, password);
    Serial.print("Conectando...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Conexión OK!");
    Serial.print("IP Local: ");
    Serial.println(WiFi.localIP());

    // NTP y MQTT igual que antes
    timeClient.begin();
    mqttClient.setServer(broker, port);
    mqttClient.setCallback(callback);
    if (mqttClient.connect("ClientID")) {
        Serial.println("Conectado al broker MQTT!");
        mqttClient.subscribe(topic2);
    } else {
        Serial.print("Fallo de conexión MQTT! Código de error: ");
        Serial.println(mqttClient.state());
    }

    // Inicializar el OLED
    display.begin(0x3c, true);
    display.invertDisplay(true);
    display.clearDisplay();
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevOLEDMillis >= intervalOLED) {
    prevOLEDMillis = currentMillis;  // Actualiza el tiempo anterior
      // Ejecuta la acción
      timeClient.update();
    mqttClient.loop(); // Escuchar mensajes MQTT
    displayImage(imageNumber);
    //mostrarNivelVida(nivelVida);
    //barra();
  }
  
  LDR_val = analogRead(LDR_pin); 
  Serial.print("LDR = ");
  Serial.println(LDR_val);
  manejarCondiciones(LDR_val);
  LecturaTH(); 

  if (currentMillis - lastSendMillis >= 6000) {
        EnvioDatos(); // Llamar a la función para enviar datos
        lastSendMillis = currentMillis; // Actualizar el tiempo de la última publicación
    }

}

void LecturaTH() {
    h = dht.readHumidity();
    t = dht.readTemperature();
}

void EnvioDatos() {
    unsigned long currentTimestamp = timeClient.getEpochTime();
    unsigned long adjustedTimestamp = currentTimestamp - (3 * 3575);
    setTime(adjustedTimestamp);

    int currentHour = hour();
    int currentMinute = minute();
    int currentSecond = second();
  
    char timeString[20];
    snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);

    StaticJsonDocument<200> doc;
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["time"] = timeString;
    doc["ldr"] = LDR_val;

    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);

    Serial.print("Enviando mensaje JSON al tópico: ");
    Serial.println(topic);
    Serial.println(jsonBuffer);
  
    if (mqttClient.publish(topic, jsonBuffer)) {
        Serial.println("Mensaje publicado correctamente.");
    } else {
        Serial.println("Error al publicar el mensaje.");
        Serial.print("Código de estado MQTT: ");
        Serial.println(mqttClient.state());
    }
    
}

void manejarCondiciones(int valor) {
    if (valor >= 650) {            
        digitalWrite(LED_pin, HIGH);  
        Serial.println("LDR detecta luz: LED encendido.");
    } else {        
        digitalWrite(LED_pin, LOW);   
        Serial.println("LDR detecta oscuridad: LED apagado.");
    }
}

void mostrarNivelVida(int nivel) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_BLACK);
    display.print("Nivel de Vida: ");
    display.println(nivel);
    display.display();
}



int vida = 100;


void barra() {
  display.drawRect(4, 4, 7, 56, SH110X_BLACK);
  if (nivelVida == 100) {
    display.fillRect(6, CienporY, 3, CienporH, SH110X_BLACK);
  }
  if (nivelVida == 75){
    display.fillRect(6, SetporY, 3, SetporH, SH110X_BLACK);
  }
  if (nivelVida == 50){
    display.fillRect(6, CinporY, 3, CinporH, SH110X_BLACK);
  }
  if (nivelVida == 25){
    display.fillRect(6, VeinporY, 3, VeinporH, SH110X_BLACK);
  }
   if (nivelVida == 0){
    display.fillRect(6, CerporY, 3, CerporH, SH110X_BLACK);
  }
  display.display();
}









