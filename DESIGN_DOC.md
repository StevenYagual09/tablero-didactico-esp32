# Design Doc: Tablero Didáctico para Bebés

* **Proyecto:** Tablero didáctico para bebés
* **Autor:** Roberth Steven Yagual Suárez
* **Curso:** Laboratorio de Sistemas Embebidos – Paralelo 106 (ESPOL)
* **Fecha:** Noviembre 2026

---

## 1. Introducción
El proyecto consiste en un tablero didáctico interactivo basado en **ESP32** orientado a la estimulación temprana en bebés y niños pequeños. Integra **7 botones de sonido**, **1 botón de modo** y **8 modos de juego** (animales de selva, mar y caseros, notas musicales, canciones de granja, fonemas con "m", fonemas con "p" y modo libre).

### Objetivos de diseño:
* Estimular el desarrollo sensorial y del lenguaje mediante botones físicos sin pantallas.
* Operar con 2 baterías Li-ion 18650 extraíbles (carga externa) y switch ON/OFF.
* Garantizar seguridad infantil (bordes redondeados, tapa con tornillo, volumen limitado a $\le 80\text{ dB(A)}$ y resistencia a caídas de $1\text{ m}$).
* Incorporar Wi-Fi (AP/STA con al menos 2 redes en EEPROM) apagado por defecto para configurar el equipo.
* Sincronizar estadísticas anónimas de uso en la nube.

---

## 2. Alcance y Limitaciones

### ¿Qué se construye?
* Prototipo funcional; firmware con 8 modos, antirrebote y audio MP3 vía DFPlayer Mini + microSD.
* Alimentación con 2 celdas 18650 en paralelo ($6000\text{ mAh}$ nominales), protección 1S, PTC de $3\text{ A}$ y elevador *boost* a $5\text{ V}$.
* Conectividad Wi-Fi (AP con portal web local + STA para subir métricas anónimas de sonidos favoritos a la nube).
* Carcasa en PETG (objetivo IP44, caída de $1\text{ m}$).

### ¿Qué no se va a resolver?
* Carga integrada por USB (decisión de diseño para evitar aberturas en la carcasa y simplificar protección).
* Certificación formal de laboratorio para juguetes.
* Validación clínica de eficacia educativa.
* Inmersión en agua.

---

## 3. Diagrama de Contexto
El niño interactúa únicamente con los botones físicos y el altavoz. El adulto cambia modos, activa manualmente el Wi-Fi (pulsación larga de $3\text{ s}$) y recarga las baterías fuera del tablero.

![Diagrama de Contexto](./docs/diagrama_contexto.png)

---

## 4. Diagrama de Bloques del Diseño

![Diagrama de Bloques](./docs/diagrama_bloques.png)

* **Subsistema de Energía:** Dos celdas 18650 en paralelo ($3.0\text{ – }4.2\text{ V}$) protegidas por placa 1S y PTC de $3\text{ A}$ alimentan un *boost* de $5\text{ V}$ para el DFPlayer y el pin `VIN` del ESP32. Un divisor resistivo ($100\text{ k}\Omega + 100\text{ k}\Omega$) en `GPIO 34` permite alertar a $3.4\text{ V}$ y cortar cargas a $3.2\text{ V}$.
* **Procesamiento, Audio y Nube:** El ESP32 gestiona las entradas digitales, almacena 2 redes Wi-Fi, volumen y último modo en EEPROM emulada, controla el DFPlayer Mini por UART ($9600\text{ bps}$) y acumula contadores de uso (pistas más tocadas y tiempo de sesión) para enviarlos por lotes en JSON vía HTTPS cuando el adulto enciende el Wi-Fi.

---

## 5. Diagrama de Máquina de Estados

![Máquina de Estados](./docs/maquina_estados.png)

---

## 6. Diseño de Interfaces
Las interfaces son digitales y de baja velocidad. La única con requisitos de tiempo es la UART hacia el DFPlayer, y la única analógica es el monitoreo de batería.

| Interfaz | Conexión | Señal / Protocolo | Pines / Detalle Técnico |
| :--- | :--- | :--- | :--- |
| **Botones** | Usuario $\rightarrow$ ESP32 | Digital, *pull-up* interno | `GPIO 13, 14, 25, 26, 18, 19, 21` (sonido); `GPIO 23` (modo). Antirrebote $30\text{ ms}$. |
| **Audio** | ESP32 $\leftrightarrow$ DFPlayer | UART $9600\text{ bps}$, $3.3\text{ V}$ | `GPIO 16` (RX), `GPIO 17` (TX con $R = 1\text{ k}\Omega$). |
| **Altavoz** | DFPlayer $\rightarrow$ Usuario | Salida de amplificador $3\text{ W}$ | `SPK_1`, `SPK_2` hacia altavoz $8\text{ }\Omega$. |
| **Monitoreo de batería** | Celdas $\rightarrow$ ESP32 | Analógica (ADC1) | `GPIO 34` vía divisor $100\text{ k}\Omega + 100\text{ k}\Omega$. Aviso $3.4\text{ V}$, corte $3.2\text{ V}$. |
| **Indicadores** | ESP32 $\rightarrow$ Usuario | Digital, LED con $330\text{ }\Omega$ | `GPIO 2, 4, 5` para estado de modo, Wi-Fi/Nube y alerta de batería baja. |
| **Alimentación** | Celdas $\rightarrow$ Boost $\rightarrow$ Cargas | $3.0\text{–}4.2\text{ V} \rightarrow 5\text{ V}$; $3.3\text{ V}$ en DevKit | `VIN`, `GND` (con Switch ON/OFF y PTC de $3\text{ A}$). |
| **Wi-Fi / Nube** | ESP32 $\leftrightarrow$ Red / Navegador | $802.11\text{ b/g/n}$, $2.4\text{ GHz}$, HTTP/HTTPS | Antena interna (Portal AP local y envío JSON). |
| **Configuración** | ESP32 $\leftrightarrow$ EEPROM | Emulada en flash, $196\text{ bytes}$ con CRC8 | Interna (2 redes Wi-Fi, volumen y último modo). |
| **Tarjeta de audio** | DFPlayer $\rightarrow$ microSD | SD, FAT32 | Interna (carpetas `01`–`08` y `mp3`). |

---

## 7. Alternativas de Diseño
Se eligió la opción que simplifica el circuito y reduce el riesgo para el niño, aun cuando otra alternativa fuera técnicamente mejor.

| Decisión | Alternativas | Elegida | ¿Por qué? |
| :--- | :--- | :--- | :--- |
| **Audio** | DFPlayer Mini (SD + amplificador); ESP32 con I2S + MAX98357A + SD; JQ6500 (flash interna) | **DFPlayer Mini** | Usa solo 2 pines (UART), trae amplificador y el código es corto. Su calidad es media y los clones con otro chip pueden fallar. |
| **Celdas** | 1S2P (2 celdas en paralelo); 2S (serie, $7.4\text{ V}$); 1 celda | **1S2P** | Dobla la capacidad sin balanceo ni cargador especial. 2S exigiría un regulador reductor y balanceo. |
| **Energía a 5 V** | Boost de $5\text{ V}$; LDO de bajo *dropout* a $3.3\text{ V}$ directo de la celda | **Boost de 5 V** | Garantiza $5\text{ V}$ estables para el DFPlayer incluso cuando la celda baja a $3.0\text{ V}$. |
| **Carga** | Puerto USB interno; carga externa de celdas | **Carga externa** | Menos componentes, una abertura menos en la carcasa y sin circuito de carga que supervisar. |
| **Protección de celdas** | Celdas protegidas; celdas estándar + placa 1S | **Placa 1S fija en el tablero** | La protección no sale con la celda y se puede usar cualquier celda de marca. |
| **Wi-Fi / Nube** | Apagado por defecto (activación manual); siempre activo | **Apagado por defecto** | Ahorra batería, minimiza exposición RF junto al bebé y sube datos solo bajo demanda. |
| **Botones** | Arcade $30\text{–}45\text{ mm}$; pulsadores táctiles $12\text{ mm}$ | **Arcade de 30 mm** | Evita piezas pequeñas desmontables que representen riesgo de asfixia infantil. |

---

## 8. Plan de Test y Validación

* **Eléctrico y Potencia:** Verificar diferencia $< 0.1\text{ V}$ antes de unir celdas en paralelo; salida del *boost* en $5.0\text{ V} \pm 0.15\text{ V}$ bajo carga; $20$ ciclos de switch ON/OFF sin consumo residual apagado; corte automático a $3.2\text{ V}$; y autonomía $> 15\text{ h}$ (consumos esperados: $75\text{ mA}$ reposo, $210\text{ mA}$ audio, $310\text{ mA}$ audio + Wi-Fi).
* **Funcional, Audio y Nube:** $50$ pulsaciones por botón sin rebotes; recorrido de 8 modos con persistencia en EEPROM; reproducción correcta de las 56 pistas con latencia $< 200\text{ ms}$; validación de corrupción de byte en EEPROM (recuperación vía CRC8); sincronización exitosa del JSON de pistas favoritas hacia la nube en STA y acceso al portal web en AP.
* **Mecánico y Seguridad:** Nivel sonoro $< 80\text{ dB(A)}$ a $30\text{ cm}$; temperatura $< 50\text{ }^\circ\text{C}$ tras $30\text{ min}$ de uso continuo; resistencia a $5$ caídas desde $1\text{ m}$; prueba de salpicaduras (IP44); y verificación de imposibilidad de abrir el portabaterías sin destornillador.

---

## 9. Consideraciones Éticas
El riesgo principal es que las celdas extraíbles queden al alcance de un niño; los demás dilemas son de menor gravedad y se mitigan con diseño.

| Dilema | Afectados | Decisión y mitigación |
| :--- | :--- | :--- |
| **Baterías extraíbles y seguridad infantil** | Niños y cuidadores; familias y escuelas | Extraíbles, con tornillo en el compartimento, placa de protección 1S, PTC de $3\text{ A}$, polaridad marcada e instrucciones de carga. |
| **Volumen y salud auditiva** | Bebés y cuidadores | Limitado a $\le 80\text{ dB(A)}$ a $30\text{ cm}$ mediante configuración de firmware y altavoz. |
| **Wi-Fi, privacidad y exposición** | Familia dueña de la red | Wi-Fi apagado por defecto; telemetría 100% anónima (solo ID de placa y conteo de pistas, sin datos personales) enviada vía HTTPS/TLS. |
| **Contenido y afirmaciones** | Niños, padres y autores | Audios propios o con licencia libre; se describe como estimulación, no como método clínico comprobado. |
| **Residuos electrónicos** | Familia y medio ambiente | Reparable: celdas reemplazables, zócalos para ESP32 y DFPlayer, y reciclaje de baterías. |

* **Responsables:** El equipo de diseño responde por la seguridad eléctrica, mecánica y acústica y por no exagerar los beneficios; los cuidadores, por supervisar el uso, manipular las celdas según las instrucciones y reciclarlas.
* **Impacto en la sociedad:** El tablero favorece el juego compartido entre adulto y niño, evita pantallas, reduce el uso de pilas desechables y permite cambiar los audios a otros idiomas o contextos. Su riesgo social es crear expectativas irreales sobre su efecto educativo, por eso se presenta como apoyo al juego. Cualquier comercialización exigiría ensayos de laboratorio y cumplir la normativa de juguetes vigente.
