<div align="center">
  <p align="center">
    <a href="#">
      <img src="https://i.imgur.com/7jf2LM0.jpeg" height="150px" />
    </a>
  </p>
</div>

<div style="border: 2px solid #d1d5db; padding: 20px; border-radius: 8px; background-color: #f9fafb;">
  <h2 align="center">CrystalPure</h2>
  <p align="center">Monitor de turbidez (NTU) y TDS (ppm) para filtros de agua caseros con Arduino Uno o ESP32</p>
</div>

---

## 🔧 Requisitos

- Arduino Uno **o** ESP32
- Sensor de turbidez SEN0189
- Sensor de TDS 
- Cable USB y navegador Chrome/Edge

---

## 🚀 Configuración rápida (Arduino IDE)

1. Instalar o abrir Arduino IDE 
2. Descargar este repositorio y abrir el sketch apropiado en **firmware/**.  
3. Seleccionar la placa y puerto.  
4. Compilar y subir el código.

---

## 🟢 Arduino Uno

- Sketch: `firmware/arduino/water_quality_monitor.ino`
- Velocidad serie: 9600 baud
- Tras subir, abre el Monitor Serie para ver NTU y ppm en tiempo real.

---

## 🟣 ESP32

1. En el Gestor de tarjetas instala “ESP32 by Espressif Systems”.
2. Sketch: `firmware/esp32/water_quality_monitor.ino`
3. Al terminar:
   - El dispositivo crea la red **CrystalPure** (con contraseña `12345678`).
   - Conéctate y visita `http://192.168.4.1` para el panel o `/data` para JSON.
   - También envía datos por Serial a 115200 baud; abre el Monitor Serie para ver NTU y TDS.
   - Si navegas sin conexión a Internet, las gráficas se mostrarán sin Chart.js (solo valores numéricos).

---

## 🌐 Panel web (Arduino Uno)

- Abre `web/index.html` en Chrome/Edge.  
- Pulsa **Conectar Arduino** para ver las lecturas (requiere Web con soporte Serial).

---

## 📌 Conexiones

| Sensor   | Señal | Arduino Uno | ESP32 |
|----------|-------|-------------|-------|
| Turbidez | OUT   | A0          | 35    |
| TDS      | OUT   | A1          | 34    |
| VCC      | +     | 5 V         | 3.3 V |
| GND      | −     | GND         | GND   |
