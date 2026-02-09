#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Fish Farm | Creative Portal</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary: #00d2ff;
            --secondary: #3a7bd5;
            --bg: #0f172a;
            --card-bg: rgba(30, 41, 59, 0.7);
            --glass: rgba(255, 255, 255, 0.05);
            --text: #f8fafc;
            --danger: #ef4444;
            --success: #22c55e;
            --warning: #f59e0b;
        }

        * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
        body {
            font-family: 'Outfit', sans-serif;
            background: radial-gradient(circle at top right, #1e293b, #0f172a);
            color: var(--text);
            margin: 0;
            padding: 0;
            overflow-x: hidden;
            min-height: 100vh;
        }

        /* --- Layout --- */
        .page { padding: 20px; padding-bottom: 100px; display: none; animation: fadeIn 0.4s ease; }
        .page.active { display: block; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }

        /* --- Master Health Ring --- */
        .health-container { display: flex; justify-content: center; margin-bottom: 30px; position: relative; }
        .health-ring {
            width: 150px; height: 150px; border-radius: 50%;
            border: 8px solid var(--glass);
            display: flex; flex-direction: column; align-items: center; justify-content: center;
            position: relative; transition: 0.5s;
            box-shadow: 0 0 20px rgba(0, 210, 255, 0.2);
        }
        .health-ring::after {
            content: ''; position: absolute; inset: -12px; border-radius: 50%;
            border: 2px dashed rgba(255,255,255,0.1); animation: rotate 20s linear infinite;
        }
        @keyframes rotate { to { transform: rotate(360deg); } }
        .health-ring.ok { border-color: var(--success); box-shadow: 0 0 30px rgba(34, 197, 94, 0.3); }
        .health-ring.warn { border-color: var(--warning); }
        .health-ring.error { border-color: var(--danger); animation: pulse-red 2s infinite; }
        @keyframes pulse-red { 0%, 100% { box-shadow: 0 0 10px var(--danger); } 50% { box-shadow: 0 0 40px var(--danger); } }
        .health-status { font-size: 0.8rem; text-transform: uppercase; letter-spacing: 2px; opacity: 0.7; }
        .health-val { font-size: 2rem; font-weight: 600; }

        /* --- Widgets Grid --- */
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 15px; }
        .card {
            background: var(--card-bg); backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 20px; padding: 15px; position: relative; overflow: hidden;
        }

        /* 1. Dynamic Tank */
        .tank-widget { display: flex; flex-direction: column; align-items: center; }
        .tank-body {
            width: 80px; height: 120px; border: 3px solid rgba(255,255,255,0.2);
            border-radius: 5px 5px 25px 25px; position: relative; overflow: hidden;
            background: rgba(255,255,255,0.05); margin-top: 10px;
        }
        .water {
            position: absolute; bottom: 0; width: 100%; background: linear-gradient(to top, #3a7bd5, #00d2ff);
            transition: height 1s cubic-bezier(0.4, 0, 0.2, 1);
            display: flex; align-items: center; justify-content: center;
        }
        .water::after {
            content: ''; position: absolute; top: 0; width: 200%; height: 10px;
            background: rgba(255,255,255,0.2); border-radius: 50%;
            left: -50%; transform: rotate(0); animation: wave 4s linear infinite;
        }
        @keyframes wave { to { transform: rotate(360deg); } }
        .tank-percent { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-weight: 600; z-index: 2; color: #fff; text-shadow: 0 2px 4px rgba(0,0,0,0.5); }
        .tank-body.low { border-color: var(--danger); box-shadow: inset 0 0 20px rgba(239, 68, 68, 0.4); }

        /* 2. pH Seesaw */
        .seesaw-container { height: 100px; display: flex; flex-direction: column; justify-content: center; align-items: center; }
        .seesaw-bar { width: 120px; height: 4px; background: rgba(255,255,255,0.1); position: relative; transform: rotate(0); transition: 0.5s; }
        .seesaw-pivot { width: 10px; height: 10px; background: #fff; border-radius: 50%; position: absolute; left: 50%; top: -3px; transform: translateX(-50%); }
        .seesaw-needle { width: 2px; height: 30px; background: var(--primary); position: absolute; left: 50%; bottom: 0; transform-origin: bottom; }
        .ph-icons { display: flex; justify-content: space-between; width: 100%; margin-top: 15px; }
        .ph-icon { font-size: 1.2rem; filter: grayscale(1); transition: 0.3s; }
        .ph-icon.active-up { color: #a855f7; filter: drop-shadow(0 0 8px #a855f7) grayscale(0); }
        .ph-icon.active-down { color: #f97316; filter: drop-shadow(0 0 8px #f97316) grayscale(0); }

        /* 3. TDS Cloud */
        .tds-box { width: 100%; height: 80px; position: relative; background: rgba(0,0,0,0.2); border-radius: 10px; overflow: hidden; }
        .particle { position: absolute; width: 3px; height: 3px; background: #fff; border-radius: 50%; opacity: 0.5; }
        .tds-drops { display: flex; gap: 10px; justify-content: center; margin-top: 10px; }
        .drop-icon { opacity: 0.3; transition: 0.3s; }
        .drop-icon.active { opacity: 1; transform: translateY(2px); animation: drip 1s infinite; }
        @keyframes drip { 0% { opacity: 1; } 50% { opacity: 0.5; transform: translateY(5px); } }

        /* 4. Turbidity Fog */
        .turb-gauge { width: 80px; height: 80px; border-radius: 50%; position: relative; margin: auto; transition: 1s; }
        .drain-icon { position: absolute; bottom: -5px; right: -5px; background: var(--card-bg); padding: 5px; border-radius: 50%; border: 1px solid rgba(255,255,255,0.1); }
        .drain-icon.spinning { animation: spin 2s linear infinite; color: var(--primary); }
        @keyframes spin { from { transform: rotate(0); } to { transform: rotate(360deg); } }

        /* 5. Feeding Clock */
        .feed-widget { display: flex; flex-direction: column; align-items: center; }
        .feed-ring {
            width: 100px; height: 100px; border-radius: 50%; border: 4px solid var(--glass);
            position: relative; display: flex; align-items: center; justify-content: center;
        }
        .feed-btn {
            width: 60px; height: 60px; border-radius: 50%; background: var(--warning);
            border: none; cursor: pointer; display: flex; align-items: center; justify-content: center;
            font-size: 1.2rem; transition: 0.2s; box-shadow: 0 4px 10px rgba(0,0,0,0.3);
        }
        .feed-btn:active { transform: scale(0.9); }
        .last-fed { font-size: 0.7rem; opacity: 0.6; margin-top: 8px; }

        /* 6. Power Rack */
        .rack { margin-top: 20px; }
        .rack-grid { display: grid; grid-template-columns: repeat(5, 1fr); gap: 10px; }
        .rack-item { display: flex; flex-direction: column; align-items: center; }
        .vibe-bar { width: 10px; height: 50px; background: var(--glass); border-radius: 5px; position: relative; overflow: hidden; }
        .vibe-fill { position: absolute; bottom: 0; width: 100%; height: 0; background: var(--primary); transition: 0.3s; }
        .rack-item.on .vibe-fill { animation: bounce 0.6s ease-in-out infinite alternate; height: 80% !important; }
        @keyframes bounce { from { filter: brightness(1); } to { filter: brightness(1.5); height: 100%; } }
        .rack-label { font-size: 0.6rem; margin-top: 5px; font-weight: 600; }

        /* 7. Sparklines */
        canvas.sparkline { width: 100%; height: 30px; margin-top: 10px; opacity: 0.5; transition: 0.3s; }
        .card:hover canvas.sparkline { opacity: 1; }

        /* --- Navigation --- */
        .dock {
            position: fixed; bottom: 20px; left: 50%; transform: translateX(-50%);
            width: 90%; max-width: 400px; background: rgba(15, 23, 42, 0.9);
            backdrop-filter: blur(20px); border-radius: 25px;
            display: flex; justify-content: space-around; padding: 10px;
            border: 1px solid rgba(255,255,255,0.1); box-shadow: 0 10px 30px rgba(0,0,0,0.5);
            z-index: 100;
        }
        .dock-item {
            display: flex; flex-direction: column; align-items: center; cursor: pointer;
            color: rgba(255,255,255,0.5); transition: 0.3s;
        }
        .dock-item.active { color: var(--primary); }
        .dock-item i { font-size: 1.4rem; margin-bottom: 4px; }
        .dock-item span { font-size: 0.6rem; text-transform: uppercase; font-weight: 600; }

        /* --- UI Components --- */
        .switch-row { display: flex; justify-content: space-between; align-items: center; background: var(--glass); padding: 15px; border-radius: 15px; margin-bottom: 10px; }
        input[type="checkbox"] { width: 40px; height: 20px; appearance: none; background: #334155; border-radius: 20px; position: relative; cursor: pointer; transition: 0.3s; }
        input[type="checkbox"]:checked { background: var(--primary); }
        input[type="checkbox"]::before { content: ''; position: absolute; width: 16px; height: 16px; background: #fff; border-radius: 50%; top: 2px; left: 2px; transition: 0.3s; }
        input[type="checkbox"]:checked::before { left: 22px; }

        .btn-save { width: 100%; padding: 15px; border-radius: 15px; border: none; background: var(--secondary); color: #fff; font-weight: 600; margin-top: 20px; cursor: pointer; }
        input[type="text"], input[type="password"], input[type="number"] {
            width: 100%; background: var(--glass); border: 1px solid rgba(255,255,255,0.1);
            color: #fff; padding: 12px; border-radius: 10px; margin-bottom: 10px; font-family: inherit;
        }

        /* --- Alert Popup --- */
        #alert-box {
            position: fixed; top: 20px; left: 50%; transform: translateX(-50%) translateY(-100px);
            width: 90%; max-width: 350px; background: var(--danger); color: #fff;
            padding: 20px; border-radius: 20px; display: flex; align-items: center; gap: 15px;
            box-shadow: 0 10px 30px rgba(239, 68, 68, 0.5); z-index: 200; transition: 0.4s cubic-bezier(0.18, 0.89, 0.32, 1.28);
        }
        #alert-box.show { transform: translateX(-50%) translateY(0); }
        .alert-icon { font-size: 2rem; }

        /* --- Scrollbar --- */
        ::-webkit-scrollbar { width: 4px; }
        ::-webkit-scrollbar-thumb { background: var(--glass); border-radius: 10px; }

    </style>
    <!-- Icons -->
    <script src="https://kit.fontawesome.com/a076d05399.js" crossorigin="anonymous"></script>
</head>
<body>

    <div id="alert-box">
        <div class="alert-icon">⚠️</div>
        <div>
            <strong>CRITICAL ALERT</strong><br>
            <span id="alert-msg">Water Level dangerously low!</span>
        </div>
    </div>

    <!-- HOME PAGE -->
    <div id="page-home" class="page active">
        <div class="health-container">
            <div id="main-ring" class="health-ring ok">
                <span class="health-status">System</span>
                <span class="health-val" id="health-txt">READY</span>
            </div>
        </div>

        <div class="grid">
            <!-- 1. Tank -->
            <div class="card tank-widget">
                <div style="font-size: 0.7rem; font-weight: 600;">WATER TANK</div>
                <div class="tank-body" id="tank-body">
                    <div class="water" id="water-fill" style="height: 0%">
                        <span class="tank-percent" id="txt-tank">0%</span>
                    </div>
                </div>
                <canvas id="spark-level" class="sparkline"></canvas>
            </div>

            <!-- 2. pH Seesaw -->
            <div class="card">
                <div style="font-size: 0.7rem; font-weight: 600;">pH BALANCE</div>
                <div class="seesaw-container">
                    <div class="seesaw-bar" id="ph-bar">
                        <div class="seesaw-pivot"></div>
                        <div class="seesaw-needle"></div>
                    </div>
                </div>
                <div style="text-align: center; font-size: 1.2rem; font-weight: 600;" id="txt-ph">7.0</div>
                <div class="ph-icons">
                    <span class="ph-icon" id="ph-up-icon">🔼</span>
                    <span class="ph-icon" id="ph-down-icon">🔽</span>
                </div>
                <canvas id="spark-ph" class="sparkline"></canvas>
            </div>

            <!-- 3. TDS Cloud -->
            <div class="card">
                <div style="font-size: 0.7rem; font-weight: 600;">NUTRIENTS (TDS)</div>
                <div class="tds-box" id="tds-cloud"></div>
                <div style="text-align: center; font-size: 1.2rem; font-weight: 600; margin-top: 5px;" id="txt-tds">0 ppm</div>
                <div class="tds-drops">
                    <span class="drop-icon" id="tds-up-icon">💧+</span>
                    <span class="drop-icon" id="tds-down-icon">💧-</span>
                </div>
                <canvas id="spark-tds" class="sparkline"></canvas>
            </div>

            <!-- 4. Turbidity Fog -->
            <div class="card" style="text-align: center;">
                <div style="font-size: 0.7rem; font-weight: 600;">CLARITY</div>
                <div class="turb-gauge" id="turb-gauge">
                    <div style="line-height: 80px; font-weight: 600;" id="txt-turb">3.3V</div>
                    <div class="drain-icon" id="sol-icon">⚙️</div>
                </div>
                <canvas id="spark-turb" class="sparkline"></canvas>
            </div>

            <!-- 5. Feeding Clock -->
            <div class="card feed-widget" style="grid-column: 1 / -1;">
                <div style="font-size: 0.7rem; font-weight: 600; margin-bottom: 10px;">AUTO-FEEDER</div>
                <div class="feed-ring">
                    <button class="feed-btn" onclick="feedFish()">🍖</button>
                    <!-- Simulated food particles on click -->
                    <div id="food-container"></div>
                </div>
                <div class="last-fed" id="txt-last-fed">Last meal: -- ago</div>
            </div>
        </div>

        <div class="card rack">
            <div style="font-size: 0.7rem; font-weight: 600; margin-bottom: 15px;">POWER RACK (PUMPS)</div>
            <div class="rack-grid">
                <div class="rack-item" id="rack-p1">
                    <div class="vibe-bar"><div class="vibe-fill"></div></div>
                    <span class="rack-label">pH+</span>
                </div>
                <div class="rack-item" id="rack-p2">
                    <div class="vibe-bar"><div class="vibe-fill"></div></div>
                    <span class="rack-label">pH-</span>
                </div>
                <div class="rack-item" id="rack-p3">
                    <div class="vibe-bar"><div class="vibe-fill"></div></div>
                    <span class="rack-label">TDS+</span>
                </div>
                <div class="rack-item" id="rack-p4">
                    <div class="vibe-bar"><div class="vibe-fill"></div></div>
                    <span class="rack-label">TDS-</span>
                </div>
                <div class="rack-item" id="rack-p5">
                    <div class="vibe-bar"><div class="vibe-fill"></div></div>
                    <span class="rack-label">FRESH</span>
                </div>
            </div>
        </div>
    </div>

    <!-- MOTORS PAGE -->
    <div id="page-motors" class="page">
        <h2 style="margin-top: 0;">Manual Overrides</h2>
        <div class="switch-row">
            <span>Automation Mode</span>
            <input type="checkbox" id="auto-check" onchange="toggleAuto(this.checked)">
        </div>
        <p style="font-size: 0.8rem; opacity: 0.6; margin-bottom: 20px;">* Manual toggle only works when Automation is OFF</p>
        
        <div class="grid" style="grid-template-columns: repeat(2, 1fr);">
            <button class="btn-save" id="btn-m1" onclick="togglePump(1)" style="background: var(--glass);">Pump: pH+</button>
            <button class="btn-save" id="btn-m2" onclick="togglePump(2)" style="background: var(--glass);">Pump: pH-</button>
            <button class="btn-save" id="btn-m3" onclick="togglePump(3)" style="background: var(--glass);">Pump: TDS+</button>
            <button class="btn-save" id="btn-m4" onclick="togglePump(4)" style="background: var(--glass);">Pump: TDS-</button>
            <button class="btn-save" id="btn-m5" onclick="togglePump(5)" style="background: var(--glass);">Pump: Fresh</button>
            <button class="btn-save" id="btn-m6" onclick="togglePump(6)" style="background: var(--glass);">Solenoid</button>
        </div>
    </div>

    <!-- SETTINGS PAGE -->
    <div id="page-settings" class="page">
        <h2 style="margin-top: 0;">Configuration</h2>
        
        <div class="card" style="margin-bottom: 20px;">
            <h3>Feed Schedule</h3>
            <div style="display: flex; gap: 10px;">
                <input type="number" id="f-h" placeholder="Hour (0-23)" min="0" max="23">
                <input type="number" id="f-m" placeholder="Min (0-59)" min="0" max="59">
            </div>
            <label>Servo Duration (sec)</label>
            <input type="number" id="f-d" value="1">
            <button class="btn-save" onclick="saveSettings()">Save Schedule</button>
        </div>

        <div class="card">
            <h3>WiFi Setup</h3>
            <input type="text" id="w-s" placeholder="Router SSID">
            <input type="password" id="w-p" placeholder="Password">
            <button class="btn-save" onclick="saveWiFi()" style="background: var(--warning); color: #000;">Reboot & Connect</button>
        </div>
    </div>

    <!-- NAVIGATION DOCK -->
    <div class="dock">
        <div class="dock-item active" onclick="showPage('home')">
            <span>🏠 Home</span>
        </div>
        <div class="dock-item" onclick="showPage('motors')">
            <span>⚙️ Logic</span>
        </div>
        <div class="dock-item" onclick="showPage('settings')">
            <span>🛠️ Tools</span>
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var ws;
        var sensorHistory = { lev: [], ph: [], tds: [], turb: [] };
        var lastFedTime = Date.now();

        function initWS() {
            ws = new WebSocket(gateway);
            ws.onopen = () => console.log('Connected');
            ws.onclose = () => setTimeout(initWS, 2000);
            ws.onmessage = (e) => {
                var d = JSON.parse(e.data);
                if (d.type === 'sensors') updateSensors(d);
                if (d.type === 'states') updateStates(d);
                if (d.type === 'settings') updateSettings(d);
            };
        }

        function updateSensors(d) {
            // 1. Tank (Ultrasonic) - Assume range 5cm (100%) to 30cm (0%)
            var levelP = Math.max(0, Math.min(100, (30 - d.level) / 25 * 100));
            document.getElementById('water-fill').style.height = levelP + '%';
            document.getElementById('txt-tank').innerText = levelP.toFixed(0) + '%';
            if (levelP < 20) {
                document.getElementById('tank-body').classList.add('low');
                showAlert("CRITICAL: Water level is dangerously low!", true);
            } else {
                document.getElementById('tank-body').classList.remove('low');
                showAlert("", false);
            }

            // 2. pH Seesaw
            var ph = d.ph;
            document.getElementById('txt-ph').innerText = ph.toFixed(1);
            var phD = (ph - 7) * 10; // Simple scale
            document.getElementById('ph-bar').style.transform = `rotate(${phD}deg)`;

            // 3. TDS Cloud
            document.getElementById('txt-tds').innerText = d.tds.toFixed(0) + ' ppm';
            updateTDSCloud(d.tds);

            // 4. Turbidity
            document.getElementById('txt-turb').innerText = d.turb.toFixed(2) + 'V';
            var opacity = Math.max(0.1, Math.min(1, (3.3 - d.turb) / 2));
            document.getElementById('turb-gauge').style.background = `rgba(101, 67, 33, ${opacity})`;

            // Sparklines
            drawSparkline('spark-level', 'lev', levelP);
            drawSparkline('spark-ph', 'ph', d.ph * 10);
            drawSparkline('spark-tds', 'tds', d.tds / 10);
            drawSparkline('spark-turb', 'turb', d.turb * 20);

            // Master Ring
            updateHealth(levelP, d.ph, d.tds);
        }

        function updateStates(d) {
            document.getElementById('auto-check').checked = d.auto;
            
            // Rack Animations
            updateRack('p1', d.p1);
            updateRack('p2', d.p2);
            updateRack('p3', d.p3);
            updateRack('p4', d.p4);
            updateRack('p5', d.p5);
            
            // Icons
            document.getElementById('ph-up-icon').className = d.p1 ? 'ph-icon active-up' : 'ph-icon';
            document.getElementById('ph-down-icon').className = d.p2 ? 'ph-icon active-down' : 'ph-icon';
            document.getElementById('tds-up-icon').className = d.p3 ? 'drop-icon active' : 'drop-icon';
            document.getElementById('tds-down-icon').className = d.p4 ? 'drop-icon active' : 'drop-icon';
            document.getElementById('sol-icon').className = d.sol ? 'drain-icon spinning' : 'drain-icon';
        }

        function updateRack(id, state) {
            var item = document.getElementById('rack-' + id);
            if (state) item.classList.add('on');
            else item.classList.remove('on');
        }

        function updateSettings(d) {
            document.getElementById('f-h').value = d.h;
            document.getElementById('f-m').value = d.m;
            document.getElementById('f-d').value = d.d;
        }

        function updateTDSCloud(val) {
            var cloud = document.getElementById('tds-cloud');
            cloud.innerHTML = '';
            var count = Math.min(100, val / 10);
            for(let i=0; i<count; i++) {
                let p = document.createElement('div');
                p.className = 'particle';
                p.style.top = Math.random() * 100 + '%';
                p.style.left = Math.random() * 100 + '%';
                cloud.appendChild(p);
            }
        }

        function updateHealth(lev, ph, tds) {
            var ring = document.getElementById('main-ring');
            var txt = document.getElementById('health-txt');
            if (lev < 20 || ph < 5 || ph > 9 || tds > 800) {
                ring.className = 'health-ring error';
                txt.innerText = 'ALERT';
            } else if (lev < 40 || ph < 6 || ph > 8) {
                ring.className = 'health-ring warn';
                txt.innerText = 'CHECK';
            } else {
                ring.className = 'health-ring ok';
                txt.innerText = 'HEALTHY';
            }
        }

        function drawSparkline(canvasId, key, newVal) {
            sensorHistory[key].push(newVal);
            if (sensorHistory[key].length > 50) sensorHistory[key].shift();
            
            var c = document.getElementById(canvasId);
            var ctx = c.getContext('2d');
            ctx.clearRect(0,0,c.width,c.height);
            ctx.strokeStyle = '#00d2ff';
            ctx.lineWidth = 2;
            ctx.beginPath();
            var step = c.width / 50;
            for(let i=0; i<sensorHistory[key].length; i++) {
                let x = i * step;
                let y = c.height - (sensorHistory[key][i] / 100 * c.height);
                if(i===0) ctx.moveTo(x,y);
                else ctx.lineTo(x,y);
            }
            ctx.stroke();
        }

        function showPage(id) {
            document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
            document.querySelectorAll('.dock-item').forEach(i => i.classList.remove('active'));
            document.getElementById('page-' + id).classList.add('active');
            event.currentTarget.classList.add('active');
        }

        function showAlert(msg, show) {
            var box = document.getElementById('alert-box');
            if (show) {
                document.getElementById('alert-msg').innerText = msg;
                box.classList.add('show');
            } else {
                box.classList.remove('show');
            }
        }

        function toggleAuto(val) { ws.send(JSON.stringify({cmd: 'auto', val: val})); }
        function feedFish() { 
            ws.send(JSON.stringify({cmd: 'feed'}));
            lastFedTime = Date.now();
            updateLastFed();
            spawnParticles(); 
        }
        function togglePump(id) { ws.send(JSON.stringify({cmd: 'toggle', id: id})); }
        function saveSettings() {
            ws.send(JSON.stringify({
                cmd: 'save_settings',
                h: parseInt(document.getElementById('f-h').value),
                m: parseInt(document.getElementById('f-m').value),
                d: parseInt(document.getElementById('f-d').value)
            }));
            alert("Schedule Updated!");
        }
        function saveWiFi() {
            ws.send(JSON.stringify({
                cmd: 'save_wifi',
                s: document.getElementById('w-s').value,
                p: document.getElementById('w-p').value
            }));
        }

        function updateLastFed() {
            var diff = Math.floor((Date.now() - lastFedTime) / 1000);
            var m = Math.floor(diff / 60);
            var s = diff % 60;
            document.getElementById('txt-last-fed').innerText = `Last meal: ${m}m ${s}s ago`;
        }
        setInterval(updateLastFed, 1000);

        function spawnParticles() {
            var container = document.getElementById('food-container');
            for(let i=0; i<15; i++) {
                let p = document.createElement('div');
                p.style.position = 'absolute';
                p.style.width = '4px'; p.style.height = '4px';
                p.style.background = '#ffc107';
                p.style.left = '50%'; p.style.top = '50%';
                p.style.borderRadius = '50%';
                container.appendChild(p);
                let x = (Math.random() - 0.5) * 100;
                let y = Math.random() * 50 + 20;
                p.animate([
                    { transform: 'translate(0, 0)', opacity: 1 },
                    { transform: `translate(${x}px, ${y}px)`, opacity: 0 }
                ], { duration: 1000, fill: 'forwards' });
                setTimeout(() => p.remove(), 1000);
            }
        }

        window.onload = initWS;
    </script>
</body>
</html>
)rawliteral";

#endif
