# Tablero Didáctico Interactivo para Bebés (ESP32)

Sistema embebido orientado a la estimulación sensorial, auditiva y del lenguaje en bebés y niños pequeños mediante botones físicos de alta resistencia, reproducción de audio MP3, conectividad Wi-Fi (Access Point/Estación) y telemetría de preferencias en la nube.

* **Materia:** Sistemas Embebidos
* **Autores:** Roberth Steven Yagual Suarez & Cristhian Erwin Lucas Chávez
* **Documentación completa:** Ver [DESIGN_DOC.md](./DESIGN_DOC.md)

---

## Características Principales
* **8 Modos de Juego:** Animales de la selva, mar y caseros, notas musicales, canciones de la granja, fonemas con "m", fonemas con "p" y modo libre.
* **Alimentación Autónoma Segura (1S2P):** Dos celdas Li-ion 18650 en paralelo ($3.7\text{ V}$ / $6000\text{ mAh}$) con placa de protección BMS 1S, fusible rearmable PTC de $3\text{ A}$, interruptor ON/OFF y elevador *boost* DC-DC a $5.0\text{ V}$.
* **Protección de Batería:** Monitoreo continuo por ADC con alerta visual a $3.40\text{ V}$ y corte seguro de audio y Wi-Fi a $3.20\text{ V}$.
* **Conectividad Wi-Fi y Nube:** Modo *Access Point* (`Proyectobebe`) con almacenamiento en EEPROM de hasta 2 redes Wi-Fi y envío de estadísticas anónimas de uso (sonidos favoritos).

---

## Mapa de Conexiones (Pinout ESP32)

| Componente | Pin ESP32 | Detalle Técnico |
| :--- | :--- | :--- |
| **Botones de Sonido (1–7)** | `GPIO 13, 14, 25, 26, 18, 19, 21` | `INPUT_PULLUP` interno, antirrebote $30\text{ ms}$ |
| **Botón de Cambio de Modo** | `GPIO 23` | `INPUT_PULLUP` interno (ciclo modos 1–8) |
| **DFPlayer Mini (UART2)** | `GPIO 16` (RX), `GPIO 17` (TX) | Comunicación serial a $9600\text{ bps}$ ($R=1\text{ k}\Omega$ en TX) |
| **Sensor Voltaje Batería** | `GPIO 34` (ADC1) | Divisor resistivo $100\text{ k}\Omega + 100\text{ k}\Omega$ desde pack 1S2P |
| **LEDs Indicadores** | `GPIO 2` (Modo), `GPIO 4` (Wi-Fi), `GPIO 5` (Bat.) | Resistencias limitadoras de $330\text{ }\Omega$ |

---

## Estructura de Audio en la Tarjeta microSD
Formatear la tarjeta microSD en **FAT32** y organizar los archivos MP3 de la siguiente manera:
* `/mp3/0001.mp3` a `0008.mp3`: Anuncios de voz al cambiar entre los 8 modos.
* `/01/001.mp3` a `/08/007.mp3`: 7 pistas de audio por cada una de las 8 carpetas de modo (56 pistas totales).

---

## Dependencias de Software (Arduino IDE / PlatformIO)
1. **Placa:** `ESP32 Dev Module`
2. **Librerías requeridas:**
   * `DFRobotDFPlayerMini`
   * `apwifieeprommode`
   * `EEPROM` (incluida en el core de ESP32)
