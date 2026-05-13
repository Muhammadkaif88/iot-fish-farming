#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>AquaSmart | IoT Fish Farm</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&family=Plus+Jakarta+Sans:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary: #00d2ff;
            --primary-rgb: 0, 210, 255;
            --secondary: #3a7bd5;
            --bg: #0b0f1a;
            --card-bg: rgba(23, 32, 53, 0.65);
            --glass: rgba(255, 255, 255, 0.03);
            --glass-border: rgba(255, 255, 255, 0.08);
            --text: #f8fafc;
            --text-muted: #94a3b8;
            --danger: #ff4b5c;
            --success: #00e676;
            --warn: #ffab00;
            --font-main: 'Plus Jakarta Sans', sans-serif;
            --font-heading: 'Outfit', sans-serif;
        }

        * {
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
            outline: none;
        }

        body {
            font-family: var(--font-main);
            background: var(--bg);
            background-image: 
                radial-gradient(circle at 0% 0%, rgba(0, 210, 255, 0.1) 0%, transparent 40%),
                radial-gradient(circle at 100% 100%, rgba(58, 123, 213, 0.1) 0%, transparent 40%);
            color: var(--text);
            margin: 0;
            padding: 0;
            min-height: 100vh;
            overflow-x: hidden;
            line-height: 1.5;
        }

        /* --- Animations --- */
        @keyframes pulse-glow { 
            0% { box-shadow: 0 0 0 0 rgba(var(--primary-rgb), 0.4); } 
            70% { box-shadow: 0 0 0 15px rgba(var(--primary-rgb), 0); } 
            100% { box-shadow: 0 0 0 0 rgba(var(--primary-rgb), 0); } 
        }
        @keyframes fade-in { from { opacity: 0; } to { opacity: 1; } }
        @keyframes slide-up { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }
        @keyframes rotate { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }

        .page {
            display: none;
            padding: 24px 20px 110px;
            animation: slide-up 0.5s cubic-bezier(0.16, 1, 0.3, 1);
            max-width: 500px;
            margin: 0 auto;
        }
        
        .page.active { display: block; }

        /* --- Header --- */
        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 32px;
            padding-top: 10px;
        }

        .logo-area h3 { 
            font-family: var(--font-heading);
            font-size: 1.4rem;
            font-weight: 700;
            letter-spacing: -0.5px;
            margin: 0;
            background: linear-gradient(135deg, #fff 0%, var(--primary) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        
        .status-badge {
            padding: 6px 14px;
            border-radius: 100px;
            font-size: 0.75rem;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 8px;
            background: var(--glass);
            border: 1px solid var(--glass-border);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
        }

        .status-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--success); box-shadow: 0 0 12px var(--success); transition: 0.3s; }
        .status-dot.disc { background: var(--danger); box-shadow: 0 0 12px var(--danger); }

        /* --- Health Monitor --- */
        .health-section {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin-bottom: 40px;
        }

        .health-ring-outer {
            width: 180px;
            height: 180px;
            border-radius: 50%;
            padding: 10px;
            background: conic-gradient(from 0deg, var(--primary) 0%, transparent 70%);
            position: relative;
            display: flex;
            align-items: center;
            justify-content: center;
            animation: rotate 10s linear infinite;
        }

        .health-ring-inner {
            width: 100%;
            height: 100%;
            background: var(--bg);
            border-radius: 50%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            position: relative;
            z-index: 2;
            transform: rotate(calc(-1 * var(--rot, 0deg))); /* counter-rotate text if needed */
        }

        .health-ring-static {
            position: absolute;
            width: 170px;
            height: 170px;
            border: 2px solid var(--glass-border);
            border-radius: 50%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            background: radial-gradient(circle, rgba(var(--primary-rgb), 0.08), transparent 75%);
            box-shadow: 0 0 40px rgba(var(--primary-rgb), 0.15);
        }

        .health-val { 
            font-family: var(--font-heading);
            font-size: 2.2rem; 
            font-weight: 800; 
            letter-spacing: -1px;
            color: #fff;
        }
        .health-label { 
            font-size: 0.7rem; 
            font-weight: 700;
            letter-spacing: 2px; 
            color: var(--text-muted);
            text-transform: uppercase; 
            margin-top: 2px;
        }

        /* --- Cards Grid --- */
        .grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 16px;
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            border: 1px solid var(--glass-border);
            border-radius: 24px;
            padding: 20px;
            position: relative;
            transition: transform 0.3s cubic-bezier(0.34, 1.56, 0.64, 1);
        }
        
        .card:active { transform: scale(0.98); }

        .card-title { 
            font-size: 0.7rem; 
            font-weight: 700; 
            color: var(--text-muted); 
            text-transform: uppercase; 
            letter-spacing: 1.5px; 
            margin-bottom: 12px; 
            display: flex;
            align-items: center;
            gap: 6px;
        }
        .card-val { 
            font-family: var(--font-heading);
            font-size: 1.8rem; 
            font-weight: 700; 
            margin: 0;
            color: #fff;
        }
        .unit { font-size: 0.85rem; font-weight: 500; color: var(--text-muted); margin-left: 4px; }

        /* --- Visual Helpers --- */
        .tank-track { width: 50px; height: 90px; background: rgba(0,0,0,0.4); border-radius: 12px; position: relative; overflow: hidden; border: 1px solid var(--glass-border); margin: 0 auto; }
        .tank-fill { position: absolute; bottom: 0; width: 100%; background: linear-gradient(to top, var(--secondary), var(--primary)); transition: height 1s cubic-bezier(0.4, 0, 0.2, 1); }
        .tank-fill::after { content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 4px; background: rgba(255,255,255,0.3); filter: blur(2px); }

        .progress-mini { height: 6px; background: rgba(0,0,0,0.3); border-radius: 100px; margin-top: 15px; overflow: hidden; }
        .progress-bar { height: 100%; width: 0%; transition: width 1s; border-radius: 100px; }

        /* --- Buttons & Inputs --- */
        .btn {
            width: 100%;
            padding: 16px;
            border: none;
            border-radius: 18px;
            background: linear-gradient(135deg, var(--secondary), var(--primary));
            color: white;
            font-family: var(--font-main);
            font-size: 0.9rem;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            box-shadow: 0 10px 20px -10px rgba(var(--primary-rgb), 0.5);
        }

        .btn:active { transform: translateY(2px); box-shadow: 0 5px 10px -5px rgba(var(--primary-rgb), 0.5); }
        .btn.success { background: linear-gradient(135deg, #00c853, #64ffda); box-shadow: 0 10px 20px -10px rgba(0, 200, 83, 0.4); }
        .btn.danger { background: linear-gradient(135deg, #ff1744, #f50057); box-shadow: 0 10px 20px -10px rgba(255, 23, 68, 0.4); }
        .btn.outline { background: var(--glass); border: 1px solid var(--glass-border); box-shadow: none; }
        .btn.active { background: var(--success); color: #000; }

        .switch-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: var(--card-bg);
            padding: 18px 20px;
            border-radius: 22px;
            margin-bottom: 12px;
            border: 1px solid var(--glass-border);
        }
        
        .toggle { width: 50px; height: 26px; background: #2d3748; border-radius: 50px; position: relative; transition: 0.3s; cursor: pointer; }
        .toggle::after { content: ''; position: absolute; top: 3px; left: 3px; width: 20px; height: 20px; background: #fff; border-radius: 50%; transition: 0.3s; }
        .toggle.checked { background: var(--primary); }
        .toggle.checked::after { left: 27px; }

        /* --- Dock --- */
        .dock {
            position: fixed;
            bottom: 24px;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(15, 23, 42, 0.85);
            backdrop-filter: blur(25px);
            -webkit-backdrop-filter: blur(25px);
            border-radius: 30px;
            display: flex;
            padding: 6px;
            gap: 8px;
            border: 1px solid var(--glass-border);
            box-shadow: 0 20px 50px rgba(0,0,0,0.6);
            z-index: 1000;
        }

        .dock-item {
            padding: 12px 20px;
            border-radius: 24px;
            cursor: pointer;
            transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            color: var(--text-muted);
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 4px;
        }

        .dock-item.active { background: var(--primary); color: #000; transform: translateY(-8px); font-weight: 700; }
        .dock-icon { font-size: 1.3rem; }
        .dock-label { font-size: 0.6rem; text-transform: uppercase; letter-spacing: 1px; }

        input, select {
            width: 100%;
            background: rgba(0,0,0,0.3);
            border: 1px solid var(--glass-border);
            color: white;
            padding: 14px;
            border-radius: 14px;
            font-family: var(--font-main);
            font-size: 0.95rem;
            margin-bottom: 12px;
            transition: 0.3s;
        }
        
        input:focus { border-color: var(--primary); background: rgba(0,0,0,0.5); }

    </style>
</head>
<body>

    <!-- HOME PAGE -->
    <div id="home" class="page active">
        <header>
            <div class="logo-area">
                <div id="date-disp" style="font-size:0.7rem; font-weight:700; color:var(--text-muted); text-transform:uppercase; margin-bottom:2px">--- -- ---</div>
                <h3 id="time-disp">--:--:--</h3>
            </div>
            <div class="status-badge">
                <div class="status-dot" id="conn-dot"></div>
                <span id="conn-txt">OFFLINE</span>
            </div>
        </header>

        <div class="health-section">
            <div class="health-ring-static" id="ring">
                <div class="health-val" id="sys-status">---</div>
                <div class="health-label">HEALTH</div>
            </div>
        </div>

        <div class="grid">
            <div class="card" style="text-align:center">
                <div class="card-title">🌊 Water Level</div>
                <div class="tank-track">
                    <div class="tank-fill" id="w-lvl" style="height:0%"></div>
                </div>
                <div class="card-val" id="t-txt" style="margin-top:12px; font-size:1.5rem">--%</div>
            </div>
            
            <div class="card">
                <div class="card-title">🧪 pH Balance</div>
                <div class="card-val" id="ph-val">--.-</div>
                <div class="progress-mini">
                    <div id="ph-bar" class="progress-bar" style="background:linear-gradient(90deg, #ff4b2b, #ff416c, #00d2ff, #3a7bd5)"></div>
                </div>
                <div id="ph-stat" style="font-size:0.7rem; margin-top:10px; color:var(--text-muted); font-weight:600">---</div>
            </div>

            <div class="card">
                <div class="card-title">💎 TDS Purity</div>
                <div class="card-val" id="tds-val">--<span class="unit">ppm</span></div>
                <div id="tds-stat" style="font-size:0.7rem; margin-top:12px; color:var(--text-muted)">Range: 100-500</div>
            </div>

            <div class="card">
                <div class="card-title">🌪️ Turbidity</div>
                <div class="card-val" id="turb-val">--<span class="unit">NTU</span></div>
                <div class="progress-mini">
                    <div id="turb-bar" class="progress-bar" style="background:linear-gradient(90deg, #a855f7, #ec4899)"></div>
                </div>
            </div>
            
            <div class="card" style="grid-column: span 2; padding: 24px;">
                <div style="display:flex; align-items:center; justify-content:space-between; margin-bottom:20px">
                    <div>
                        <div class="card-title" style="margin-bottom:4px">🍖 SMART FEEDER</div>
                        <div id="next-feed" style="font-size:0.8rem; font-weight:600; color:var(--primary)">Waiting for data...</div>
                    </div>
                    <button class="btn" style="width:auto; padding:10px 20px; font-size:0.8rem" onclick="feed()">Feed Now</button>
                </div>
                
                <div id="feed-working" style="display:none; animation: fade-in 0.3s;">
                    <div style="display:flex; justify-content:space-between; font-size:0.7rem; margin-bottom:8px; font-weight:700">
                        <span style="color:var(--success)">⚙️ DISPENSING...</span>
                        <span id="work-time" style="color:var(--text-muted)">0s</span>
                    </div>
                    <div class="progress-mini" style="height:8px; margin-top:0">
                        <div id="work-bar" class="progress-bar" style="background:var(--success); box-shadow:0 0 10px var(--success)"></div>
                    </div>
                </div>
                
                <div id="feed-idle" style="display:flex; gap:20px; border-top:1px solid var(--glass-border); padding-top:15px; margin-top:5px">
                    <div>
                        <div style="font-size:0.6rem; color:var(--text-muted); font-weight:700; text-transform:uppercase">Last Fed</div>
                        <div id="last-feed" style="font-weight:700; font-size:0.9rem">---</div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <!-- CONTROL PAGE -->
    <div id="ctrl" class="page">
        <h2 style="font-family:var(--font-heading); font-size:1.8rem; margin-bottom:8px">Operations</h2>
        <p style="color:var(--text-muted); font-size:0.9rem; margin-bottom:24px">Manual system override</p>

        <div class="switch-row" style="background:rgba(var(--primary-rgb), 0.1); border-color:rgba(var(--primary-rgb), 0.2)">
            <div style="display:flex; align-items:center; gap:12px">
                <span style="font-size:1.2rem">✨</span>
                <div>
                    <div style="font-weight:700; font-size:0.95rem">Automatic AI Mode</div>
                    <div style="font-size:0.7rem; color:var(--text-muted)">Sensor-driven corrections</div>
                </div>
            </div>
            <div id="sw-auto" class="toggle" onclick="togAuto()"></div>
        </div>

        <div class="switch-row" style="background:rgba(255, 75, 92, 0.08); border-color:rgba(255, 75, 92, 0.2)">
            <div style="display:flex; align-items:center; gap:12px">
                <span style="font-size:1.2rem">🧹</span>
                <div>
                    <div style="font-weight:700; font-size:0.95rem">Smart Clean cycle</div>
                    <div style="font-size:0.7rem; color:var(--text-muted)">Drain & Refill sequence</div>
                </div>
            </div>
            <div id="sw-clean" class="toggle" onclick="togClean()"></div>
        </div>

        <div id="clean-banner" style="display:none; background:var(--card-bg); border:1px solid var(--danger); border-radius:20px; padding:20px; margin-bottom:20px">
            <div style="display:flex; justify-content:space-between; align-items:center">
                <div>
                    <div id="clean-phase-label" style="font-weight:800; color:var(--danger); font-size:1.1rem">---</div>
                    <div id="clean-phase-sub" style="color:var(--text-muted); font-size:0.75rem; margin-top:4px">---</div>
                </div>
                <div id="clean-phase-icon" style="font-size:1.8rem">⏳</div>
            </div>
            <div class="progress-mini" style="height:8px; margin-top:15px">
                <div id="clean-prog" class="progress-bar" style="background:var(--danger)"></div>
            </div>
        </div>

        <div class="grid" style="margin-top:20px">
            <button id="b1" class="btn outline" onclick="tog(1)">💧 Fill Pump</button>
            <button id="b2" class="btn outline" onclick="tog(2)">🔄 TDS Pump A</button>
            <button id="b3" class="btn outline" onclick="tog(3)">🧂 TDS Pump B</button>
            <button id="b4" class="btn outline" onclick="tog(4)">🔼 pH Up</button>
            <button id="b5" class="btn outline" onclick="tog(5)">🔽 pH Down</button>
            <button id="b6" class="btn outline" onclick="tog(6)">🛑 Drain Valve</button>
        </div>
    </div>

    <!-- SETTINGS PAGE -->
    <div id="set" class="page">
        <h2 style="font-family:var(--font-heading); font-size:1.8rem; margin-bottom:24px">Configuration</h2>
        
        <div class="card" style="margin-bottom:20px">
            <div class="card-title">📡 Network Access</div>
            <input id="ssid" placeholder="WiFi Network Name">
            <input id="pass" type="password" placeholder="Access Password">
            <button class="btn" onclick="saveWifi()">Update Network</button>
        </div>

        <div class="card">
            <div class="card-title">🕒 Feeding Schedule</div>
            <div id="sched-list"></div>
            <button class="btn outline" style="margin-bottom:20px" onclick="addSched()">+ Add Feed Time</button>
            
            <div class="card-title">⚖️ Dispense Amount</div>
            <div style="display:flex; align-items:center; gap:15px; margin-bottom:20px">
                <input id="dur" type="number" value="1" min="0.5" max="5" step="0.5" style="margin-bottom:0">
                <span style="font-weight:700; color:var(--text-muted)">SEC</span>
            </div>
            
            <button class="btn success" onclick="saveSet()">Apply All Settings</button>
        </div>
    </div>

    <!-- NAVIGATION DOCK -->
    <nav class="dock">
        <div class="dock-item active" onclick="nav('home', this)">
            <div class="dock-icon">🏠</div>
            <div class="dock-label">Dash</div>
        </div>
        <div class="dock-item" onclick="nav('ctrl', this)">
            <div class="dock-icon">⚡</div>
            <div class="dock-label">Ops</div>
        </div>
        <div class="dock-item" onclick="nav('set', this)">
            <div class="dock-icon">⚙️</div>
            <div class="dock-label">Setup</div>
        </div>
    </nav>

    <script>
        var ws, scheds = [];
        var feedingStartMs = 0, feedDurSec = 1, workTimer = null;
        const $ = (id) => document.getElementById(id);

        function con() {
            ws = new WebSocket('ws://' + location.hostname + '/ws');
            ws.onopen = () => { 
                $('conn-dot').className = 'status-dot';
                $('conn-txt').innerText = 'LIVE';
            };
            ws.onclose = () => { 
                $('conn-dot').className = 'status-dot disc';
                $('conn-txt').innerText = 'OFFLINE';
                setTimeout(con, 3000); 
            };
            ws.onmessage = (e) => {
                const d = JSON.parse(e.data);
                if(d.type == 'sensors') updSensors(d);
                if(d.type == 'states') updStates(d);
                if(d.type == 'settings') updSet(d);
            };
        }

        function updSensors(d) {
            const lp = Math.max(0, Math.min(100, ((15 - d.level)/12)*100));
            $('w-lvl').style.height = lp + '%';
            $('t-txt').innerText = lp.toFixed(0) + '%';
            
            $('ph-val').innerText = d.ph.toFixed(1);
            $('ph-bar').style.width = ((d.ph / 14) * 100) + '%';
            $('ph-stat').innerText = d.ph < 6 ? 'pH: ACIDIC' : (d.ph > 7.5 ? 'pH: ALKALINE' : 'pH: STABLE');
            
            $('tds-val').innerHTML = Math.round(d.tds) + '<span class="unit">ppm</span>';
            
            $('turb-val').innerHTML = Math.round(d.turb) + '<span class="unit">NTU</span>';
            $('turb-bar').style.width = Math.min(100, (d.turb / 3000) * 100) + '%';
            
            const r = $('ring');
            const txt = $('sys-status');
            if(lp < 15 || d.ph < 5.5 || d.ph > 8.5 || d.turb > 1000) {
                r.style.boxShadow = '0 0 50px rgba(255, 75, 92, 0.4)';
                r.style.borderColor = 'var(--danger)';
                txt.innerText = 'ALERT';
                txt.style.color = 'var(--danger)';
            } else {
                r.style.boxShadow = '0 0 50px rgba(0, 210, 255, 0.2)';
                r.style.borderColor = 'var(--glass-border)';
                txt.innerText = 'STABLE';
                txt.style.color = 'var(--primary)';
            }
        }

        function updStates(d) {
            if(d.auto) $('sw-auto').classList.add('checked'); else $('sw-auto').classList.remove('checked');
            
            for(let i=1; i<=6; i++) {
                const b = $('b'+i);
                if(d['p'+i]) b.classList.add('active'); else b.classList.remove('active');
            }

            const sc = $('sw-clean');
            const banner = $('clean-banner');
            if(d.cleaning) {
                sc.classList.add('checked');
                banner.style.display = 'block';
                if(d.cleanPhase === 0) {
                    $('clean-phase-label').innerText = 'Draining Tank...';
                    $('clean-phase-sub').innerText   = 'Clearing dirty water via solenoid';
                    $('clean-phase-icon').innerText  = '🚰';
                    $('clean-prog').style.width      = '40%';
                } else {
                    $('clean-phase-label').innerText = 'Fresh Refill...';
                    $('clean-phase-sub').innerText   = 'Adding clean water to optimal level';
                    $('clean-phase-icon').innerText  = '💧';
                    $('clean-prog').style.width      = '80%';
                }
            } else {
                sc.classList.remove('checked');
                banner.style.display = 'none';
            }

            if(d.feeding) {
                if(!feedingStartMs) feedingStartMs = Date.now();
                $('feed-working').style.display = 'block';
                $('feed-idle').style.display = 'none';
                if(!workTimer) workTimer = setInterval(() => {
                    const elapsed = (Date.now() - feedingStartMs) / 1000;
                    const pct = Math.min(100, (elapsed / feedDurSec) * 100);
                    $('work-bar').style.width = pct + '%';
                    $('work-time').innerText = elapsed.toFixed(1) + 's';
                }, 100);
            } else {
                feedingStartMs = 0;
                $('feed-working').style.display = 'none';
                $('feed-idle').style.display = 'flex';
                clearInterval(workTimer); workTimer = null;
            }

            if(d.ct) $('time-disp').innerText = d.ct;
            if(d.dt) $('date-disp').innerText = d.dt;
        }

        function updSet(d) {
            if(d.times) { scheds = d.times; renSched(); }
            if(d.d) { $('dur').value = d.d; feedDurSec = parseFloat(d.d); }
            if(d.nr > -1) {
                const hr12 = (d.nh % 12) || 12;
                const ap = d.nh >= 12 ? 'PM' : 'AM';
                $('next-feed').innerText = `Next: ${hr12}:${z(d.nm)} ${ap} (in ${Math.floor(d.nr/60)}m)`;
            } else $('next-feed').innerText = 'No schedules set';
            if(d.lf > -1) $('last-feed').innerText = d.lf < 60 ? `${d.lf}s ago` : `${Math.floor(d.lf/60)}m ago`;
        }

        function nav(p, el) {
            document.querySelectorAll('.page').forEach(x => x.classList.remove('active'));
            $(p).classList.add('active');
            document.querySelectorAll('.dock-item').forEach(x => x.classList.remove('active'));
            el.classList.add('active');
        }

        function tog(id) { ws.send(JSON.stringify({cmd:'toggle', id:id})); }
        function togAuto() { ws.send(JSON.stringify({cmd:'auto', val:!$('sw-auto').classList.contains('checked')})); }
        function togClean() { ws.send(JSON.stringify({cmd:'clean', val:!$('sw-clean').classList.contains('checked')})); }
        function feed() { if(confirm('Dispense food now?')) ws.send(JSON.stringify({cmd:'feed'})); }

        function renSched() {
            let html = '';
            scheds.forEach((t, i) => {
                const hr12 = (t[0] % 12) || 12;
                const ap = t[0] >= 12 ? 'PM' : 'AM';
                html += `<div style="display:flex; gap:8px; margin-bottom:12px; align-items:center;">
                    <select id="h${i}" onchange="updSchedAlt(${i})" style="margin:0">${[...Array(12).keys()].map(x => `<option value="${x+1}" ${x+1===hr12?'selected':''}>${z(x+1)}</option>`).join('')}</select>
                    <select id="m${i}" onchange="updSchedAlt(${i})" style="margin:0">${[...Array(60).keys()].map(x => `<option value="${x}" ${x===t[1]?'selected':''}>${z(x)}</option>`).join('')}</select>
                    <select id="ap${i}" onchange="updSchedAlt(${i})" style="margin:0"><option value="AM" ${ap==='AM'?'selected':''}>AM</option><option value="PM" ${ap==='PM'?'selected':''}>PM</option></select>
                    <button class="btn danger" style="width:45px; padding:12px; border-radius:12px" onclick="delSched(${i})">×</button>
                </div>`;
            });
            $('sched-list').innerHTML = html;
        }
        
        function updSchedAlt(i) {
            let h = parseInt($('h'+i).value);
            if (h === 12 && $('ap'+i).value === 'AM') h = 0;
            else if (h < 12 && $('ap'+i).value === 'PM') h += 12;
            scheds[i] = [h, parseInt($('m'+i).value)];
        }

        function z(n) { return n<10?'0'+n:n; }
        function addSched() { if(scheds.length < 5) { scheds.push([8, 0]); renSched(); } }
        function delSched(i) { scheds.splice(i, 1); renSched(); }
        function saveSet() { ws.send(JSON.stringify({cmd:'save_settings', times:scheds, d:$('dur').value})); alert('Saved'); }
        function saveWifi() { ws.send(JSON.stringify({cmd:'save_wifi', s:$('ssid').value, p:$('pass').value})); alert('Rebooting...'); }

        window.onload = con;
    </script>
</body>
</html>
)rawliteral";
dow.onload = con;
    </script>
</body>
</html>
)rawliteral";

#endif
