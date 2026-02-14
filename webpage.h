#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Fish Farm</title>
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
            --warn: #f59e0b;
        }

        * {
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: radial-gradient(circle at top right, #1e293b, #0f172a);
            color: var(--text);
            margin: 0;
            padding: 0;
            min-height: 100vh;
            overflow-x: hidden;
        }

        /* --- Animations --- */
        @keyframes pulse { 0% { box-shadow: 0 0 0 0 rgba(0, 210, 255, 0.4); } 70% { box-shadow: 0 0 0 10px rgba(0, 210, 255, 0); } 100% { box-shadow: 0 0 0 0 rgba(0, 210, 255, 0); } }
        @keyframes float { 0% { transform: translateY(0px); } 50% { transform: translateY(-10px); } 100% { transform: translateY(0px); } }
        @keyframes slideUp { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }

        .page {
            display: none;
            padding: 20px 20px 100px;
            animation: slideUp 0.4s ease-out;
            max-width: 600px;
            margin: 0 auto;
        }
        
        .page.active { display: block; }

        /* --- Header --- */
        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
        }

        h1, h2, h3 { margin: 0; font-weight: 600; }
        
        .status-badge {
            padding: 5px 12px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: bold;
            display: flex;
            align-items: center;
            gap: 6px;
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(5px);
        }

        .status-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--success); box-shadow: 0 0 8px var(--success); }
        .status-dot.disc { background: var(--danger); box-shadow: 0 0 8px var(--danger); }

        /* --- Health Ring --- */
        .health-container {
            display: flex;
            justify-content: center;
            margin: 20px 0 40px;
        }

        .health-ring {
            width: 160px;
            height: 160px;
            border-radius: 50%;
            border: 8px solid var(--glass);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            position: relative;
            background: radial-gradient(circle, rgba(58, 123, 213, 0.1), transparent 70%);
            box-shadow: 0 0 30px rgba(0, 210, 255, 0.2);
            transition: all 0.5s ease;
        }

        .health-ring.ok { border-color: var(--success); box-shadow: 0 0 40px rgba(34, 197, 94, 0.3); }
        .health-ring.warn { border-color: var(--warn); }
        .health-ring.err { border-color: var(--danger); animation: pulse 2s infinite; }

        .health-val { font-size: 2.5rem; font-weight: 700; background: linear-gradient(to right, #fff, #cbd5e1); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        .health-label { font-size: 0.8rem; letter-spacing: 2px; opacity: 0.7; margin-top: 5px; text-transform: uppercase; }

        /* --- Cards Grid --- */
        .grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(12px);
            -webkit-backdrop-filter: blur(12px);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 20px;
            padding: 15px;
            position: relative;
            overflow: hidden;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
        }

        .card-title { font-size: 0.75rem; font-weight: 600; color: #94a3b8; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 10px; }
        .card-val { font-size: 1.8rem; font-weight: 700; margin: 5px 0; }
        .unit { font-size: 0.9rem; font-weight: 500; opacity: 0.6; margin-left: 2px; }

        /* --- Visualizations --- */
        /* Water Tank */
        .tank-widget { width: 60px; height: 100px; border: 3px solid rgba(255,255,255,0.2); border-radius: 8px 8px 20px 20px; position: relative; overflow: hidden; margin: 0 auto; background: rgba(0,0,0,0.3); }
        .water-level { position: absolute; bottom: 0; width: 100%; background: linear-gradient(to top, var(--secondary), var(--primary)); transition: height 0.8s cubic-bezier(0.4, 0, 0.2, 1); opacity: 0.8; }
        .water-level::before { content: ''; position: absolute; top: -5px; left: 0; width: 100%; height: 10px; background: rgba(255,255,255,0.2); border-radius: 50%; }

        /* pH Gauge */
        .ph-gauge { height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; position: relative; margin: 30px 5px 10px; }
        .ph-marker { width: 12px; height: 12px; background: #fff; border-radius: 50%; position: absolute; top: -3px; transform: translateX(-50%); box-shadow: 0 0 10px rgba(255,255,255,0.5); transition: left 0.5s; }
        .ph-scale { display: flex; justify-content: space-between; font-size: 0.6rem; opacity: 0.4; margin-top: 5px; }

        /* Turbidity Bar */
        .turb-bar-bg { height: 8px; background: #334155; border-radius: 4px; overflow: hidden; margin-top: 15px; }
        .turb-bar { height: 100%; width: 0%; background: linear-gradient(90deg, #a855f7, #ec4899); transition: width 0.5s; }

        /* --- Controls --- */
        .btn {
            width: 100%;
            padding: 14px;
            border: none;
            border-radius: 14px;
            background: var(--secondary);
            color: white;
            font-size: 0.95rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            position: relative;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.2);
        }

        .btn:active { transform: scale(0.97); }
        .btn.active { background: var(--success); box-shadow: 0 0 15px rgba(34, 197, 94, 0.4); }
        .btn.danger { background: var(--danger); }
        
        .switch-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: var(--glass);
            padding: 12px 15px;
            border-radius: 12px;
            margin-bottom: 10px;
            border: 1px solid rgba(255,255,255,0.05);
        }
        
        .switch-label { font-size: 0.9rem; font-weight: 500; display: flex; align-items: center; gap: 10px; }
        
        /* Custom Toggle */
        .toggle { width: 44px; height: 24px; background: #334155; border-radius: 24px; position: relative; transition: 0.3s; cursor: pointer; }
        .toggle::after { content: ''; position: absolute; top: 2px; left: 2px; width: 20px; height: 20px; background: #fff; border-radius: 50%; transition: 0.3s; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
        .toggle.checked { background: var(--success); }
        .toggle.checked::after { left: 22px; }

        /* --- Dock Navigation --- */
        .dock {
            position: fixed;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(15, 23, 42, 0.9);
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            border-radius: 24px;
            display: flex;
            padding: 8px 10px;
            gap: 15px;
            border: 1px solid rgba(255,255,255,0.1);
            box-shadow: 0 10px 40px rgba(0,0,0,0.5);
            z-index: 100;
        }

        .dock-item {
            padding: 10px 16px;
            border-radius: 16px;
            cursor: pointer;
            transition: all 0.3s;
            opacity: 0.6;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 4px;
        }

        .dock-item.active { background: rgba(255,255,255,0.1); opacity: 1; transform: translateY(-5px); }
        .dock-icon { font-size: 1.2rem; }
        .dock-label { font-size: 0.6rem; font-weight: 600; text-transform: uppercase; }

        /* Inputs */
        input, select {
            width: 100%;
            background: rgba(0,0,0,0.2);
            border: 1px solid rgba(255,255,255,0.1);
            color: white;
            padding: 12px;
            border-radius: 12px;
            font-size: 1rem;
            margin-bottom: 12px;
            outline: none;
        }
        
        input:focus { border-color: var(--primary); }

    </style>
</head>
<body>

    <!-- HOME PAGE -->
    <div id="home" class="page active">
        <header>
            <div>
                <h3>Smart Fish Farm</h3>
                <span id="date-time" style="font-size:0.8rem; opacity:0.6">--:--</span>
            </div>
            <div class="status-badge"><div class="status-dot" id="conn-dot"></div><span id="conn-txt">DISC</span></div>
        </header>

        <div class="health-container">
            <div id="ring" class="health-ring ok">
                <div class="health-val" id="sys-status">GOOD</div>
                <div class="health-label">SYSTEM HEALTH</div>
            </div>
        </div>

        <div class="grid">
            <!-- Water Tank -->
            <div class="card" style="text-align:center">
                <div class="card-title">WATER LEVEL</div>
                <div class="tank-widget">
                    <div class="water-level" id="w-lvl" style="height:50%"></div>
                </div>
                <div class="card-val" id="t-txt" style="font-size:1.2rem; margin-top:10px">50%</div>
            </div>
            
            <!-- pH Balance -->
            <div class="card">
                <div class="card-title">pH BALANCE</div>
                <div class="card-val" id="ph-val">7.0</div>
                <div class="ph-gauge">
                    <div class="ph-marker" id="ph-marker" style="left: 50%"></div>
                </div>
                <div class="ph-scale"><span>Ac</span><span>Alk</span></div>
                <div style="font-size:0.75rem; margin-top:8px; opacity:0.7" id="ph-stat">Neutral</div>
            </div>

            <!-- TDS -->
            <div class="card">
                <div class="card-title">WATER PURITY (TDS)</div>
                <div class="card-val" id="tds-val">0<span class="unit">ppm</span></div>
                <div style="font-size:0.75rem; opacity:0.7; margin-top:5px">Target: 100-500</div>
            </div>

            <!-- Turbidity -->
            <div class="card">
                <div class="card-title">TURBIDITY</div>
                <div class="card-val" id="turb-val">0<span class="unit">V</span></div>
                <div class="turb-bar-bg"><div class="turb-bar" id="turb-bar"></div></div>
                <div style="font-size:0.75rem; opacity:0.7; margin-top:5px" id="turb-stat">Clear</div>
            </div>
            
            <!-- Feeder -->
            <div class="card" style="grid-column: span 2; display:flex; align-items:center; justify-content:space-between">
                <div>
                    <div class="card-title">SMART FEEDER</div>
                    <div style="font-size:1.5rem">🍖 <span id="last-feed">--m ago</span></div>
                </div>
                <button class="btn" style="width:auto; padding:10px 20px" onclick="feed()">Feed Now</button>
            </div>
        </div>
    </div>

    <!-- CONTROL PAGE -->
    <div id="ctrl" class="page">
        <h2>Manual Control</h2>
        <p style="opacity:0.6; margin-bottom:20px">Override automation logic</p>

        <div class="switch-row" style="background:rgba(0, 210, 255, 0.1); border-color:var(--primary)">
            <div class="switch-label">✨ Auto Mode</div>
            <div id="sw-auto" class="toggle checked" onclick="togAuto()"></div>
        </div>

        <div class="grid">
            <button id="b1" class="btn" onclick="tog(1)">💧 Pump Fill</button>
            <button id="b2" class="btn" onclick="tog(2)">🔄 Drain / Rep</button>
            <button id="b3" class="btn" onclick="tog(3)">🧂 Add Minerals</button>
            <button id="b4" class="btn" onclick="tog(4)">🔼 pH Up</button>
            <button id="b5" class="btn" onclick="tog(5)">🔽 pH Down</button>
            <button id="b6" class="btn" onclick="tog(6)">🛑 Solenoid</button>
        </div>
    </div>

    <!-- SETTINGS PAGE -->
    <div id="set" class="page">
        <h2>Settings</h2>
        
        <div class="card" style="margin-top:20px">
            <div class="card-title">WIFI CONFIGURATION</div>
            <input id="ssid" placeholder="WiFi SSID">
            <input id="pass" type="password" placeholder="Password">
            <button class="btn" onclick="saveWifi()">Save & Reboot</button>
        </div>

        <div class="card" style="margin-top:15px">
            <div class="card-title">FEEDING SCHEDULE</div>
            <div id="sched-list"></div>
            <button class="btn" style="margin-bottom:10px; background:var(--glass)" onclick="addSched()">+ Add Time</button>
            
            <div class="card-title" style="margin-top:15px">SERVO DURATION (Sec)</div>
            <input id="dur" type="number" value="1" min="0.5" max="5" step="0.5">
            
            <button class="btn success" style="background:var(--success)" onclick="saveSet()">Save Settings</button>
        </div>
    </div>

    <!-- NAVIGATION DOCK -->
    <div class="dock">
        <div class="dock-item active" onclick="nav('home', this)">
            <div class="dock-icon">🏠</div>
            <div class="dock-label">Home</div>
        </div>
        <div class="dock-item" onclick="nav('ctrl', this)">
            <div class="dock-icon">⚙️</div>
            <div class="dock-label">Control</div>
        </div>
        <div class="dock-item" onclick="nav('set', this)">
            <div class="dock-icon">🛠️</div>
            <div class="dock-label">Setup</div>
        </div>
    </div>

    <script>
        var ws, t, scheds = [];
        const $ = (id) => document.getElementById(id);

        function con() {
            // Reconnect logic
            ws = new WebSocket('ws://' + location.hostname + '/ws');
            ws.onopen = () => { 
                $('conn-dot').className = 'status-dot';
                $('conn-txt').innerText = 'LIVE';
            };
            ws.onclose = () => { 
                $('conn-dot').className = 'status-dot disc';
                $('conn-txt').innerText = 'DISC';
                setTimeout(con, 2000); 
            };
            ws.onmessage = (e) => {
                var d = JSON.parse(e.data);
                if(d.type == 'sensors') updSensors(d);
                if(d.type == 'states') updStates(d);
                if(d.type == 'settings') updSet(d);
            };
        }

        function updSensors(d) {
            // Level (30cm empty, 5cm full -> 25cm range)
            var lp = Math.max(0, Math.min(100, ((30 - d.level)/25)*100));
            $('w-lvl').style.height = lp + '%';
            $('t-txt').innerText = lp.toFixed(0) + '%';
            
            // pH
            $('ph-val').innerText = d.ph.toFixed(1);
            var phP = ((d.ph - 0) / 14) * 100; // 0-14 scale
            $('ph-marker').style.left = phP + '%';
            $('ph-stat').innerText = d.ph < 6 ? 'Acidic' : (d.ph > 8 ? 'Alkaline' : 'Neutral');
            
            // TDS
            $('tds-val').innerHTML = Math.round(d.tds) + '<span class="unit">ppm</span>';
            
            // Turbidity
            $('turb-val').innerHTML = d.turb.toFixed(2) + '<span class="unit">V</span>';
            var tp = (d.turb / 5.0) * 100; 
            $('turb-bar').style.width = tp + '%';
            $('turb-stat').innerText = d.turb < 2.0 ? 'Dirty (Cleaning)' : 'Clear';
            
            // Ring Status
            var r = $('ring');
            var txt = $('sys-status');
            if(lp < 20 || d.ph < 5 || d.ph > 9 || d.turb < 2.0) {
                r.className = 'health-ring err';
                txt.innerText = 'ATTN';
            } else {
                r.className = 'health-ring ok';
                txt.innerText = 'GOOD';
            }
        }

        function updStates(d) {
            // Auto Toggle
            var sw = $('sw-auto');
            if(d.auto) sw.classList.add('checked'); else sw.classList.remove('checked');
            
            // Buttons
            for(var i=1; i<=6; i++) {
                var b = $('b'+i);
                if(d['p'+i]) b.classList.add('active'); else b.classList.remove('active');
            }
            
            // Time
            if(d.ct) $('date-time').innerText = d.ct;
        }

        function updSet(d) {
            // Schedule
            if(d.times) {
                scheds = d.times;
                renSched();
            }
            if(d.d) $('dur').value = d.d;
            if(d.nr > -1) {
                var h = Math.floor(d.nr / 3600);
                var m = Math.floor((d.nr % 3600) / 60);
                var s = d.nr % 60;
                $('last-feed').innerHTML = '<span style="font-size:1rem; opacity:0.8">Next in:</span><br>' + 
                                           z(h) + ':' + z(m) + ':' + z(s);
            } else if(d.lf > -1) {
                var m = Math.floor(d.lf / 60);
                $('last-feed').innerText = 'Last: ' + m + 'm ago';
            } else {
                 $('last-feed').innerText = '--:--:--';
            }
        }

        // Navigation
        function nav(p, el) {
            document.querySelectorAll('.page').forEach(x => x.classList.remove('active'));
            $(p).classList.add('active');
            document.querySelectorAll('.dock-item').forEach(x => x.classList.remove('active'));
            el.classList.add('active');
        }

        // Actions
        function tog(id) {
            // Optimistic Update
            $('b'+id).classList.toggle('active');
            ws.send(JSON.stringify({cmd:'toggle', id:id}));
        }

        function togAuto() {
            var sw = $('sw-auto');
            var v = sw.classList.toggle('checked');
            ws.send(JSON.stringify({cmd:'auto', val:v}));
        }

        function feed() {
            if(confirm('Dispense food now?')) {
                ws.send(JSON.stringify({cmd:'feed'}));
            }
        }

        // Settings Logic
        function renSched() {
            var h = '';
            scheds.forEach((t, i) => {
                h += `<div style="display:flex; gap:10px; margin-bottom:5px">
                    <input type="time" value="${z(t[0])}:${z(t[1])}" onchange="updSched(${i}, this.value)">
                    <button class="btn danger" style="width:50px" onclick="delSched(${i})">X</button>
                </div>`;
            });
            $('sched-list').innerHTML = h;
        }
        
        function z(n) { return n<10?'0'+n:n; }

        function addSched() {
            if(scheds.length >= 5) return alert('Max 5 schedules');
            scheds.push([8, 0]);
            renSched();
        }

        function updSched(i, v) {
            var [h, m] = v.split(':').map(Number);
            scheds[i] = [h, m];
        }

        function delSched(i) {
            scheds.splice(i, 1);
            renSched();
        }

        function saveSet() {
            var d = {
                cmd: 'save_settings',
                times: scheds,
                d: $('dur').value
            };
            ws.send(JSON.stringify(d));
            alert('Settings Saved');
        }

        function saveWifi() {
            ws.send(JSON.stringify({
                cmd: 'save_wifi',
                s: $('ssid').value,
                p: $('pass').value
            }));
            alert('Rebooting...');
        }

        window.onload = con;
    </script>
</body>
</html>
)rawliteral";

#endif
