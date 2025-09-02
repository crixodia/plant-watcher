#ifndef WEBPAGE_CONTENT_H
#define WEBPAGE_CONTENT_H

const char PAGE_HTML_HEAD[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta http-equiv='refresh' content='10' />
  <title>ESP8266 Sensor Data</title>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <style>
    body {
      font-family: Arial;
      background: #f4f4f4;
      color: #333;
      margin: 20px;
    }
    h1 {
      color: #0066cc;
    }
    table {
      border-collapse: collapse;
      width: 100%;
      margin-bottom: 20px;
    }
    th, td {
      border: 1px solid #ccc;
      padding: 8px;
      text-align: center;
    }
    th {
      background: #0066cc;
      color: white;
    }
    .chart-container {
      width: 100%;
      max-width: 600px;
      margin: auto;
    }
  </style>
</head>
<body>
  <h1>Datos desde Arduino</h1>
)rawliteral";

const char PAGE_HTML_FOOT_START_SCRIPT[] = R"rawliteral(
  <div class='chart-container'><canvas id='chart'></canvas></div>
  <script>
    const data = )rawliteral";
    
const char PAGE_HTML_FOOT_END_SCRIPT[] = R"rawliteral(;
    const labels = data.map((_, i) => i + 1);
    const temp = data.map(d => d.temperature);
    const hum = data.map(d => d.humidity);
    const soil = data.map(d => d.soil);
    const ctx = document.getElementById('chart').getContext('2d');
    new Chart(ctx, {
      type: 'line',
      data: { 
        labels: labels, 
        datasets: [
          { label: 'Temperatura °C', data: temp, borderColor: 'red', fill: false },
          { label: 'Humedad %', data: hum, borderColor: 'blue', fill: false },
          { label: 'Suelo', data: soil, borderColor: 'green', fill: false }
        ] 
      },
      options: { 
        responsive: true, 
        plugins: { 
          legend: { 
            position: 'bottom' 
          } 
        } 
      }
    });
  </script>
  <p>Refrescando cada 10 segundos</p>
</body>
</html>
)rawliteral";

#endif