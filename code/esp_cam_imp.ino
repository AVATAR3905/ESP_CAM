#include <WiFi.h>
#include <WebServer.h>
#include <AViShaESPCam.h>

const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

AViShaESPCam cam;
WebServer    server(80);

String lastQRData = "";
int    scanCount  = 0;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>QR Scanner</title>
<script src="https://cdn.jsdelivr.net/npm/jsqr@1.4.0/dist/jsQR.js"></script>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    font-family: 'Segoe UI', sans-serif;
    background: #111;
    color: #fff;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 24px 16px;
    gap: 16px;
    min-height: 100vh;
  }

  h1 {
    font-size: 1.2rem;
    color: #00e5ff;
    letter-spacing: 2px;
  }

  /* Camera view with QR overlay box */
  .viewer {
    position: relative;
    width: 100%;
    max-width: 400px;
    border-radius: 12px;
    overflow: hidden;
    background: #000;
  }

  #stream {
    width: 100%;
    display: block;
  }

  /* QR targeting box drawn on top of stream */
  #overlay {
    position: absolute;
    top: 0; left: 0;
    width: 100%; height: 100%;
    pointer-events: none;
  }

  /* Result box */
  .result {
    width: 100%;
    max-width: 400px;
    background: #1a1a1a;
    border-radius: 12px;
    padding: 20px;
    border: 1px solid #222;
  }

  .result-label {
    font-size: 0.7rem;
    color: #555;
    text-transform: uppercase;
    letter-spacing: 1px;
    margin-bottom: 8px;
  }

  #result-text {
    font-size: 1.1rem;
    font-weight: 600;
    color: #00e5ff;
    word-break: break-all;
    min-height: 28px;
  }

  #result-text.empty {
    color: #333;
    font-style: italic;
    font-weight: 400;
    font-size: 0.9rem;
  }

  /* Flash animation when QR found */
  @keyframes flashGreen {
    0%   { border-color: #00ff88; box-shadow: 0 0 16px #00ff8866; }
    100% { border-color: #222;    box-shadow: none; }
  }
  .result.flash {
    animation: flashGreen 0.6s ease-out;
  }

  /* History */
  .history {
    width: 100%;
    max-width: 400px;
    background: #1a1a1a;
    border-radius: 12px;
    padding: 20px;
    border: 1px solid #222;
    max-height: 200px;
    overflow-y: auto;
  }

  .history-label {
    font-size: 0.7rem;
    color: #555;
    text-transform: uppercase;
    letter-spacing: 1px;
    margin-bottom: 10px;
  }

  .hist-item {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    padding: 7px 0;
    border-bottom: 1px solid #222;
    gap: 10px;
    font-size: 0.85rem;
  }

  .hist-item:last-child { border-bottom: none; }
  .hist-val  { color: #ccc; word-break: break-all; }
  .hist-time { color: #444; white-space: nowrap; font-size: 0.75rem; }
  .no-history { color: #333; font-style: italic; font-size: 0.85rem; }
</style>
</head>
<body>

<h1>QR SCANNER</h1>

<div class="viewer">
  <img id="stream" src="/stream" crossorigin="anonymous">
  <canvas id="overlay"></canvas>
</div>

<div class="result" id="result-box">
  <div class="result-label">Scanned Result</div>
  <div id="result-text" class="empty">Point camera at a QR code</div>
</div>

<div class="history">
  <div class="history-label">History</div>
  <div id="history-list"><div class="no-history">No scans yet</div></div>
</div>

<script>
  const stream      = document.getElementById('stream');
  const overlay     = document.getElementById('overlay');
  const octx        = overlay.getContext('2d');
  const resultText  = document.getElementById('result-text');
  const resultBox   = document.getElementById('result-box');
  const historyList = document.getElementById('history-list');

  // Hidden canvas for jsQR pixel data
  const canvas = document.createElement('canvas');
  const ctx    = canvas.getContext('2d');

  const history = [];
  let lastCode  = '';

  function drawCorners(points, color) {
    if (!points) return;
    octx.strokeStyle = color;
    octx.lineWidth   = 3;
    octx.beginPath();
    octx.moveTo(points[0].x, points[0].y);
    points.forEach(p => octx.lineTo(p.x, p.y));
    octx.lineTo(points[0].x, points[0].y);
    octx.stroke();
  }

  function scan() {
    if (!stream.naturalWidth) { requestAnimationFrame(scan); return; }

    // Sync overlay size to stream display size
    overlay.width  = stream.offsetWidth;
    overlay.height = stream.offsetHeight;
    octx.clearRect(0, 0, overlay.width, overlay.height);

    // Draw stream into hidden canvas at native resolution
    canvas.width  = stream.naturalWidth;
    canvas.height = stream.naturalHeight;
    ctx.drawImage(stream, 0, 0);

    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    const code = jsQR(imageData.data, imageData.width, imageData.height, {
      inversionAttempts: "attemptBoth"  // tries both normal and inverted
    });

    if (code) {
      // Scale corner points from native to display size
      const scaleX = overlay.width  / canvas.width;
      const scaleY = overlay.height / canvas.height;
      const corners = code.location;
      const pts = [
        corners.topLeftCorner,
        corners.topRightCorner,
        corners.bottomRightCorner,
        corners.bottomLeftCorner
      ].map(p => ({ x: p.x * scaleX, y: p.y * scaleY }));

      drawCorners(pts, '#00ff88');

      if (code.data !== lastCode) {
        lastCode = code.data;

        // Update result
        resultText.textContent = code.data;
        resultText.className   = '';

        // Flash effect
        resultBox.classList.remove('flash');
        void resultBox.offsetWidth;
        resultBox.classList.add('flash');

        // Add to history
        const time = new Date().toLocaleTimeString();
        history.unshift({ val: code.data, time });
        if (history.length > 30) history.pop();
        historyList.innerHTML = history.map(h =>
          '<div class="hist-item">' +
            '<span class="hist-val">'  + esc(h.val)  + '</span>' +
            '<span class="hist-time">' + h.time + '</span>' +
          '</div>'
        ).join('');

        // Send to ESP32
        fetch('/result', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'data=' + encodeURIComponent(code.data)
        });
      }
    }

    requestAnimationFrame(scan);
  }

  function esc(s) {
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }

  requestAnimationFrame(scan);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleStream() {
  WiFiClient client = server.client();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Cache-Control: no-cache");
  client.println();

  while (client.connected()) {
    FrameBuffer* fb = cam.capture();
    if (!fb) continue;
    client.println("--frame");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len);
    client.println();
    cam.returnFrame(fb);
  }
}

void handleResult() {
  if (server.hasArg("data")) {
    String decoded = server.arg("data");
    if (decoded != lastQRData) {
      lastQRData = decoded;
      scanCount++;
      Serial.println("QR: " + decoded);
    }
  }
  server.send(200, "text/plain", "ok");
}

void setup() {
  Serial.begin(115200);

  cam.enableLogging(false);
  if (!cam.init(AI_THINKER(), QVGA)) {
    Serial.println("Camera init failed!");
    ESP.restart();
  }

  // Tune for QR scanning
  sensor_t* s = esp_camera_sensor_get();
  s->set_quality(s, 8);
  s->set_contrast(s, 2);
  s->set_sharpness(s, 2);
  s->set_saturation(s, -2);  // grayscale-like, better for QR
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 1);
  s->set_gain_ctrl(s, 1);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 20) {
    delay(500); Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    Serial.println("\nOpen: http://" + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi failed: " + String(WiFi.status()));
  }

  server.on("/",       handleRoot);
  server.on("/stream", handleStream);
  server.on("/result", HTTP_POST, handleResult);
  server.begin();
}

void loop() {
  server.handleClient();
}
