#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// AP Name & Password
const char* apName = "ESP8266-Game";
const char* apPassword = "12345678";

ESP8266WebServer server(80);

String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP8266 Game</title>
<style>
body {
  margin: 0;
  background: #111;
  color: white;
  font-family: Arial;
  text-align: center;
}

h1 {
  margin-top: 10px;
}

canvas {
  background: #222;
  border: 2px solid white;
  margin-top: 20px;
}

button {
  margin-top: 15px;
  padding: 10px 20px;
  font-size: 18px;
}
</style>
</head>
<body>

<h1>ESP8266 Mini Game</h1>
<p>Tap Jump Button</p>

<canvas id="game" width="320" height="200"></canvas>
<br>
<button onclick="jump()">JUMP</button>

<script>
const canvas = document.getElementById('game');
const ctx = canvas.getContext('2d');

let player = {
  x: 40,
  y: 150,
  w: 20,
  h: 20,
  vy: 0,
  jumping: false
};

let obstacle = {
  x: 320,
  y: 160,
  w: 20,
  h: 40
};

let score = 0;
let gameOver = false;

function jump() {
  if (!player.jumping) {
    player.vy = -10;
    player.jumping = true;
  }
}

function update() {
  if (gameOver) return;

  player.y += player.vy;
  player.vy += 0.5;

  if (player.y >= 150) {
    player.y = 150;
    player.vy = 0;
    player.jumping = false;
  }

  obstacle.x -= 5;

  if (obstacle.x < -20) {
    obstacle.x = 320;
    score++;
  }

  // Collision
  if (
    player.x < obstacle.x + obstacle.w &&
    player.x + player.w > obstacle.x &&
    player.y < obstacle.y + obstacle.h &&
    player.y + player.h > obstacle.y
  ) {
    gameOver = true;
  }
}

function draw() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // Ground
  ctx.fillStyle = 'white';
  ctx.fillRect(0, 180, 320, 5);

  // Player
  ctx.fillStyle = 'lime';
  ctx.fillRect(player.x, player.y, player.w, player.h);

  // Obstacle
  ctx.fillStyle = 'red';
  ctx.fillRect(obstacle.x, obstacle.y, obstacle.w, obstacle.h);

  // Score
  ctx.fillStyle = 'white';
  ctx.font = '18px Arial';
  ctx.fillText('Score: ' + score, 10, 20);

  if (gameOver) {
    ctx.fillStyle = 'yellow';
    ctx.font = '30px Arial';
    ctx.fillText('GAME OVER', 70, 100);
  }
}

function loop() {
  update();
  draw();
  requestAnimationFrame(loop);
}

loop();
</script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(apName, apPassword);

  IPAddress IP = WiFi.softAPIP();

  Serial.println("AP Started");
  Serial.print("IP Address: ");
  Serial.println(IP);

  server.on("/", handleRoot);

  server.begin();

  Serial.println("Web Server Started");
}

void loop() {
  server.handleClient();
}
