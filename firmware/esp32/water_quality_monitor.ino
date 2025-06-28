#include <WiFi.h>
#include <WebServer.h>
#include <pgmspace.h>

// Página principal servida por el ESP32
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Panel de Calidad de Agua</title>
  <!-- Chart.js CDN -->
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    const chartReady = typeof Chart !== 'undefined';
    if(!chartReady){
      console.warn('Chart.js no pudo cargarse; las gráficas estarán deshabilitadas.');
    }
  </script>
  <style>
    :root{
      --bg:#18181b;
      --card:#1e1e22;
      --primary:#2563eb;
      --primary-dark:#1d4ed8;
      --text:#e5e5e5;
      --accent:#38bdf8;
    }
    *{box-sizing:border-box}
    body{
      font-family: system-ui, sans-serif;
      background:var(--bg);
      color:var(--text);
      margin:0;
      display:flex;
      flex-direction:column;
      align-items:center;
      padding:2rem;
      gap:1.5rem;
    }
    h1{margin:0 0 0.5rem}
    #status{font-size:0.9rem;color:var(--accent)}
    button{
      background:var(--primary);
      color:white;
      padding:0.6rem 1.2rem;
      border:none;
      border-radius:0.5rem;
      font-size:1rem;
      cursor:pointer;
      transition:background 200ms ease;
    }
    button:hover{background:var(--primary-dark)}

    #cards{
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(220px,1fr));
      gap:1rem;
      width:100%;
      max-width:900px;
    }
    .card{
      background:var(--card);
      padding:1rem 1.25rem;
      border-radius:0.75rem;
      box-shadow:0 4px 6px rgb(0 0 0 / .25);
      text-align:center;
    }
    .card h3{margin:0 0 0.25rem;font-size:1.1rem;color:var(--accent)}
    .value{font-size:1.8rem;font-weight:600;margin-bottom:0.25rem}
    small{color:#9ca3af}

    canvas{max-width:900px;margin-top:1.5rem}
    @media(max-width:940px){canvas{max-width:100%}}
  </style>
</head>
<body>
  <h1>💧 Panel de Calidad de Agua</h1>
  <div id="status">🔌 Conectando...</div>
  <button id="resetBtn" disabled>Reiniciar promedios</button>

  <!-- Tarjetas métricas -->
  <section id="cards">
    <div class="card">
      <h3>Turbidez actual</h3>
      <div id="ntuNow" class="value">--</div>
      <small>Promedio (1 min): <span id="ntuAvgWin">--</span></small><br>
      <small>Promedio sesión: <span id="ntuAvgAll">--</span></small>
    </div>
    <div class="card">
      <h3>TDS actual</h3>
      <div id="tdsNow" class="value">--</div>
      <small>Promedio (1 min): <span id="tdsAvgWin">--</span></small><br>
      <small>Promedio sesión: <span id="tdsAvgAll">--</span></small>
    </div>
  </section>

  <canvas id="ntuChart"></canvas>
  <canvas id="tdsChart"></canvas>

<script>
/* ================== Configuración ================== */
const MAX_POINTS = 60; // 60 s (asumiendo 1 Hz)
const labels = Array(MAX_POINTS).fill('');
const ntuData = [];
const tdsData = [];

// Para promedios globales
let sumNTU = 0, sumTDS = 0, countAll = 0;

/* ================== Gráficos ================== */
let ntuChart = null;
let tdsChart = null;
if(chartReady){
  ntuChart = new Chart(document.getElementById('ntuChart'), {
    type:'line',
    data:{labels,datasets:[{label:'Turbidez (NTU)',data:ntuData,borderWidth:2,fill:false}]},
    options:{responsive:true,animation:false,scales:{y:{title:{display:true,text:'NTU'}}}}
  });

  tdsChart = new Chart(document.getElementById('tdsChart'), {
    type:'line',
    data:{labels,datasets:[{label:'TDS (ppm)',data:tdsData,borderWidth:2,fill:false,tension:0.25}]},
    options:{responsive:true,animation:false,scales:{y:{title:{display:true,text:'ppm'}}}}
  });
}

/* ============== Lectura WiFi ============== */
const statusEl = document.getElementById('status');
const resetBtn   = document.getElementById('resetBtn');
let timer;

async function fetchData(){
  try{
    const res = await fetch('/data');
    if(!res.ok) throw new Error();
    const data = await res.json();
    const ntu = parseFloat(data.ntu);
    const tds = parseFloat(data.tds);
    if(isNaN(ntu) || isNaN(tds)) throw new Error();

    pushData(ntuChart, ntuData, ntu);
    pushData(tdsChart, tdsData, tds);

    sumNTU += ntu; sumTDS += tds; countAll++;
    updateUI(ntu, tds);
    statusEl.textContent = '✅ Conectado';
    resetBtn.disabled = false;
  }catch(err){
    statusEl.textContent = '⚠️ Sin conexión';
  }
}

function pushData(chart, arr, val){
  arr.push(val);
  if(arr.length > MAX_POINTS) arr.shift();
  if(chart) chart.update('none');
}

function promedio(arr){
  if(arr.length === 0) return NaN;
  return arr.reduce((a,b)=>a+b,0) / arr.length;
}

function updateUI(ntu, tds){
  document.getElementById('ntuNow').textContent = isNaN(ntu) ? '--' : ntu.toFixed(1);
  document.getElementById('tdsNow').textContent = isNaN(tds) ? '--' : tds.toFixed(0);

  const ntuWin = promedio(ntuData);
  const tdsWin = promedio(tdsData);
  const ntuAll = sumNTU / countAll;
  const tdsAll = sumTDS / countAll;

  document.getElementById('ntuAvgWin').textContent = isNaN(ntuWin) ? '--' : ntuWin.toFixed(1);
  document.getElementById('tdsAvgWin').textContent = isNaN(tdsWin) ? '--' : tdsWin.toFixed(0);

  document.getElementById('ntuAvgAll').textContent = isNaN(ntuAll) ? '--' : ntuAll.toFixed(1);
  document.getElementById('tdsAvgAll').textContent = isNaN(tdsAll) ? '--' : tdsAll.toFixed(0);
}

resetBtn.addEventListener('click', ()=>{
  sumNTU = sumTDS = countAll = 0;
  updateUI(NaN, NaN);
});

window.addEventListener('load', ()=>{
  timer = setInterval(fetchData, 1000);
  fetchData();
});
</script>
</body>
</html>
)rawliteral";

// Configuración del Punto de Acceso WiFi
const char* ssid = "CrystalPure";
const char* password = "12345678";

// Pines de conexión de los sensores para ESP32
const uint8_t TDS_PIN = 34;  // GPIO34 para el sensor de TDS
const uint8_t TURB_PIN = 35; // GPIO35 para el sensor de turbidez

// Calibración/Configuración de los sensores
const float VREF   = 3.3;   // Voltaje de referencia del ADC del ESP32 (3.3V)
const float TDS_K  = 0.5;   // Factor K para convertir EC en TDS (ppm = K * µS/cm)
const float TEMP_C = 25.0;  // Temperatura en °C asumida para una compensación simple

WebServer server(80); // Crea un servidor web en el puerto 80

// Para regular el envío por Serial cada segundo
unsigned long lastSerial = 0;

/**
 * @brief Lee los datos de los sensores de turbidez y TDS.
 * @param ntu Referencia donde se almacenará el valor de turbidez en NTU.
 * @param tds Referencia donde se almacenará el valor de TDS en ppm.
 */
void readSensors(float& ntu, float& tds) {
  // Lectura del Sensor de Turbidez
  int turbRaw = analogRead(TURB_PIN);
  float turbVoltage = turbRaw * VREF / 4095.0; // ADC del ESP32 es de 12 bits (0-4095)
  // Polinomio empírico de la hoja de datos del DFRobot SEN0189 (válido 0–4.5 V, ≈0–3000 NTU)
  ntu = -865.68 * sq(turbVoltage) + 770.02 * turbVoltage + 90.54;

  // Lectura del Sensor de TDS
  int tdsRaw = analogRead(TDS_PIN);
  float tdsVoltage = tdsRaw * VREF / 4095.0; // ADC del ESP32 es de 12 bits (0-4095)
  // Convierte el voltaje a conductividad eléctrica (mS/cm) a 25 °C
  float ecValue = (133.42 * pow(tdsVoltage, 3)
                 - 255.86 * sq(tdsVoltage)
                 + 857.39 * tdsVoltage) / 1000.0;
  // Compensación de temperatura (muy básica)
  float ec25 = ecValue / (1 + 0.0185 * (TEMP_C - 25.0));
  // Convierte EC a Sólidos Disueltos Totales (ppm)
  tds = ec25 * TDS_K * 1000;
}

/**
 * @brief Maneja las solicitudes a la URL raíz ("/").
 *        Sirve la página HTML principal del panel web.
 */
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

/**
 * @brief Maneja las solicitudes a la URL "/data".
 *        Lee los sensores y devuelve los datos en formato JSON.
 */
void handleData() {
  float ntu, tds;
  readSensors(ntu, tds);
  String json = "{";
  json += "\"ntu\":" + String(ntu, 1) + ","; // Turbidez con 1 decimal
  json += "\"tds\":" + String(tds, 0);       // TDS sin decimales
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200); // Inicializa la comunicación serial para depuración
  Serial.println("NTU\tTDS (ppm)");

  // Configura el ESP32 como un Punto de Acceso (SoftAP)
  Serial.print("Configurando SoftAP con SSID: ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Dirección IP del AP: ");
  Serial.println(IP);

  // Define las rutas del servidor web
  server.on("/", handleRoot);     // Ruta para la página principal
  server.on("/data", handleData); // Ruta para obtener los datos de los sensores

  server.begin(); // Inicia el servidor web
  Serial.println("Servidor web iniciado!");
}

void loop() {
  if (millis() - lastSerial >= 1000) {
    float ntu, tds;
    readSensors(ntu, tds);
    Serial.print(ntu, 1);
    Serial.print('\t');
    Serial.println(tds, 0);
    lastSerial = millis();
  }
  server.handleClient(); // Atiende las solicitudes de los clientes conectados
}
