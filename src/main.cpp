#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <apwifieeprommode.h>
#include <EEPROM.h>

// --- ASIGNACIÓN DE PINES (ESP32 DevKit) ---
const int btnJuego[7] = {13, 14, 25, 26, 18, 19, 21}; 
const int btnModo = 23;

const int ledModo = 2;
const int ledWiFi = 4;
const int ledBateria = 5;
const int pinBateria = 34; // Divisor resistivo 100k + 100k desde pack 1S2P

// --- GESTIÓN DE BATERÍA (2x 18650 1S2P) ---
unsigned long ultimoTiempoBateria = 0;
const int intervaloLecturaBateria = 5000; // Lectura cada 5 segundos
const float VOLTAJE_AVISO = 3.40;         // Encender LED batería baja
const float VOLTAJE_CORTE = 3.20;         // Corte de audio y WiFi por protección
bool sistemaProtegido = false;

// --- VARIABLES DE ESTADO Y TIEMPO ---
int modoActual = 1;
unsigned long ultimoTiempoBoton = 0;
const int tiempoRebote = 30; // 30 ms antirrebote según Design Doc

int estadoAnteriorModo = HIGH;
int estadoAnteriorJuego[7] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

// Contadores para telemetría en la nube (8 modos x 7 pistas)
unsigned int conteoPistas[8][7] = {0};

DFRobotDFPlayerMini myDFPlayer;

// Conversión ADC (12 bits) a voltaje real de celda con divisor 1:2
float leerVoltajeBateria() {
  int rawADC = analogRead(pinBateria);
  float voltajePin = (rawADC / 4095.0) * 3.3;
  return voltajePin * 2.0; // Factor del divisor 100k + 100k
}

void setup() {
  Serial.begin(115200);
  
  // Inicialización de Access Point y redes guardadas en EEPROM
  intentoconexion("Proyectobebe", "67bebe67");
  
  // UART2 Hardware para DFPlayer Mini (RX=16, TX=17)
  Serial2.begin(9600, SERIAL_8N1, 16, 17); 
  
  pinMode(btnModo, INPUT_PULLUP);
  for (int i = 0; i < 7; i++) {
    pinMode(btnJuego[i], INPUT_PULLUP);
  }

  pinMode(ledModo, OUTPUT);
  pinMode(ledWiFi, OUTPUT);
  pinMode(ledBateria, OUTPUT);
  pinMode(pinBateria, INPUT);

  digitalWrite(ledWiFi, HIGH);

  Serial.println("Inicializando DFPlayer Mini...");
  delay(2000); 
  
  if (!myDFPlayer.begin(Serial2, true, false)) {
    Serial.println("Error: Revisa conexiones UART (GPIO 16/17) o microSD.");
    while (true); 
  }
  
  Serial.println("DFPlayer listo.");
  myDFPlayer.volume(18); // Volumen calibrado < 80 dB(A) a 30 cm
  
  digitalWrite(ledModo, LOW);
  digitalWrite(ledBateria, LOW);
}

void loop() {
  unsigned long tiempoActual = millis();

  // 1. MONITOREO DE BATERÍA (1S2P + BMS + PTC 3A + Boost 5V)
  if (tiempoActual - ultimoTiempoBateria > intervaloLecturaBateria) {
    float vBat = leerVoltajeBateria();
    Serial.print("Voltaje Pack 1S2P: ");
    Serial.print(vBat, 2);
    Serial.println(" V");

    if (vBat <= VOLTAJE_CORTE) {
      sistemaProtegido = true;
      digitalWrite(ledBateria, HIGH);
      digitalWrite(ledWiFi, LOW);
      myDFPlayer.stop();
      Serial.println("ALERTA CRITICA: Bateria <= 3.2V. Audio y WiFi deshabilitados.");
    } else if (vBat <= VOLTAJE_AVISO) {
      sistemaProtegido = false;
      digitalWrite(ledBateria, HIGH); // Aviso preventivo de recarga
    } else {
      sistemaProtegido = false;
      digitalWrite(ledBateria, LOW);
    }
    ultimoTiempoBateria = tiempoActual;
  }

  // Si entró en corte por batería baja, bloquea reproducción y red
  if (sistemaProtegido) return;

  loopAP();

  // 2. GESTIÓN DE BOTÓN DE CAMBIO DE MODO (GPIO 23)
  int estadoActualModo = digitalRead(btnModo);
  if (estadoActualModo == HIGH && estadoAnteriorModo == LOW) {
    if (tiempoActual - ultimoTiempoBoton > tiempoRebote) {
      modoActual++;
      if (modoActual > 8) modoActual = 1;
      
      Serial.print("Modo activo: ");
      Serial.println(modoActual);
      
      digitalWrite(ledModo, HIGH);
      delay(150);
      digitalWrite(ledModo, LOW);
      
      myDFPlayer.playMp3Folder(modoActual); 
      ultimoTiempoBoton = tiempoActual;
    }
  }
  estadoAnteriorModo = estadoActualModo;

  // 3. GESTIÓN DE LOS 7 BOTONES INTERACTIVOS
  for (int i = 0; i < 7; i++) {
    int estadoActualJuego = digitalRead(btnJuego[i]);
    if (estadoActualJuego == HIGH && estadoAnteriorJuego[i] == LOW) {
      if (tiempoActual - ultimoTiempoBoton > tiempoRebote) {
        int numeroPista = i + 1;
        conteoPistas[modoActual - 1][i]++; // Registra preferencia para telemetría
        
        Serial.print("Modo "); Serial.print(modoActual);
        Serial.print(" | Pista "); Serial.print(numeroPista);
        Serial.print(" | Pulsaciones: "); Serial.println(conteoPistas[modoActual - 1][i]);
        
        myDFPlayer.playFolder(modoActual, numeroPista);
        ultimoTiempoBoton = tiempoActual;
      }
    }
    estadoAnteriorJuego[i] = estadoActualJuego;
  }
}
