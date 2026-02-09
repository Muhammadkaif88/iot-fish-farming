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
        :root { --primary: #007bff; --bg: #f4f6f9; --card-bg: #ffffff; --text: #333; }
        body { font-family: 'Segoe UI', sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 20px; }
        h1 { text-align: center; color: #444; }
        .container { max-width: 1000px; margin: auto; display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; }
        .card { background: var(--card-bg); padding: 20px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); text-align: center; }
        .card h2 { margin-top: 0; font-size: 1.2rem; color: #666; }
        .value { font-size: 2.5rem; font-weight: bold; color: var(--primary); margin: 10px 0; }
        .unit { font-size: 1rem; color: #999; }
        .controls { grid-column: 1 / -1; display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin-top: 20px; }
        .btn { border: none; padding: 15px; border-radius: 8px; font-size: 1rem; cursor: pointer; transition: 0.2s; font-weight: 600; color: white; background: #555; }
        .btn.active { background: #28a745; }
        .btn:active { transform: scale(0.98); }
        .btn-feed { background: #ffc107; color: #000; grid-column: 1 / -1; }
        
        /* Toggle Switch */
        .switch-container { display: flex; align-items: center; justify-content: space-between; margin-bottom: 20px; background: white; padding: 15px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); }
    </style>
</head>
<body>
    <h1>🐟 IoT Smart Fish Farm</h1>

    <div class="switch-container">
        <span><strong>Automation Mode</strong></span>
        <label class="switch">
            <input type="checkbox" id="autoMode" checked onchange="toggleAuto(this.checked)">
            <span class="slider round"></span>
        </label>
    </div>

    <div class="container">
        <div class="card">
            <h2>Water Level</h2>
            <div class="value" id="val-level">--</div>
            <div class="unit">cm (Distance)</div>
        </div>
        <div class="card">
            <h2>TDS Value</h2>
            <div class="value" id="val-tds">--</div>
            <div class="unit">ppm</div>
        </div>
        <div class="card">
            <h2>pH Level</h2>
            <div class="value" id="val-ph">--</div>
            <div class="unit">pH</div>
        </div>
        <div class="card">
            <h2>Turbidity</h2>
            <div class="value" id="val-turb">--</div>
            <div class="unit">Volts</div>
        </div>
    </div>

    <div class="controls">
        <button class="btn btn-feed" onclick="feedFish()">🍖 Feed Now</button>
        
        <button id="btn-p1" class="btn" onclick="togglePump(1)">Fill Pump</button>
        <button id="btn-p2" class="btn" onclick="togglePump(2)">TDS Correction 1</button>
        <button id="btn-p3" class="btn" onclick="togglePump(3)">TDS Correction 2</button>
        <button id="btn-p4" class="btn" onclick="togglePump(4)">pH Up</button>
        <button id="btn-p5" class="btn" onclick="togglePump(5)">pH Down</button>
        <button id="btn-sol" class="btn" onclick="togglePump(6)">Cleaning Valve</button>
    </div>

    <div class="card" style="grid-column: 1 / -1; text-align: left; background: #fff3cd;">
        <h2 style="color: #856404;">📶 WiFi Configuration</h2>
        <div class="switch-container" style="display: block;">
            <div style="margin-bottom: 10px;">
                <label><strong>Router SSID:</strong></label>
                <input type="text" id="ssid" placeholder="Network Name" style="width: 100%; padding: 8px; margin-top: 5px;">
            </div>
            <div style="margin-bottom: 10px;">
                <label><strong>Password:</strong></label>
                <input type="password" id="pass" placeholder="Password" style="width: 100%; padding: 8px; margin-top: 5px;">
            </div>
             <button class="btn" style="background: #e0a800; color: #000; width: 100%;" onclick="saveWiFi()">Connect to Router</button>
             <p style="font-size: 0.8rem; color: #666; margin-top: 5px;">* Device will restart after saving.</p>
        </div>
    </div>
    
    <div class="card" style="grid-column: 1 / -1; text-align: left;">
        <h2>⚙️ Settings</h2>
        <div class="switch-container" style="flex-wrap: wrap; justify-content: start; gap: 20px;">
            <div>
                <label><strong>Feeding Time (IST):</strong></label>
                <div style="display: flex; gap: 5px; align-items: center;">
                    <input type="number" id="feedHour" min="1" max="12" placeholder="HH" style="width: 50px;"> :
                    <input type="number" id="feedMinute" min="0" max="59" placeholder="MM" style="width: 50px;">
                    <select id="feedAmPm">
                        <option value="AM">AM</option>
                        <option value="PM">PM</option>
                    </select>
                </div>
            </div>
            <div>
                <label><strong>Servo Duration (sec):</strong></label>
                <input type="number" id="feedDuration" min="1" max="10" value="1" style="width: 60px;">
            </div>
             <button class="btn" style="background: #007bff; padding: 10px 20px;" onclick="saveSettings()">Save Settings</button>
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;

        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = function(event) { console.log('Connection opened'); };
            websocket.onclose = function(event) { console.log('Connection closed'); setTimeout(initWebSocket, 2000); };
            websocket.onmessage = function(event) {
                var data = JSON.parse(event.data);
                if (data.type === 'sensors') {
                    document.getElementById('val-level').innerText = data.level.toFixed(1);
                    document.getElementById('val-tds').innerText = data.tds.toFixed(0);
                    document.getElementById('val-ph').innerText = data.ph.toFixed(1);
                    document.getElementById('val-turb').innerText = data.turb.toFixed(2);
                } else if (data.type === 'states') {
                    updateButton('btn-p1', data.p1);
                    updateButton('btn-p2', data.p2);
                    updateButton('btn-p3', data.p3);
                    updateButton('btn-p4', data.p4);
                    updateButton('btn-p5', data.p5);
                    updateButton('btn-sol', data.sol);
                    document.getElementById('autoMode').checked = data.auto;
                } else if (data.type === 'settings') {
                    // Convert 24h to 12h format for display
                    var h24 = data.h;
                    var m = data.m;
                    var period = "AM";
                    var h12 = h24;
                    
                    if (h24 >= 12) {
                        period = "PM";
                        if (h24 > 12) h12 = h24 - 12;
                    }
                    if (h12 === 0) h12 = 12; // Midnight handling

                    document.getElementById('feedHour').value = h12;
                    document.getElementById('feedMinute').value = m;
                    document.getElementById('feedAmPm').value = period;
                    document.getElementById('feedDuration').value = data.d;
                }
            };
        }

        function updateButton(id, state) {
            var btn = document.getElementById(id);
            if (state) btn.classList.add('active');
            else btn.classList.remove('active');
        }

        function toggleAuto(checked) { websocket.send(JSON.stringify({cmd: 'auto', val: checked})); }
        function feedFish() { websocket.send(JSON.stringify({cmd: 'feed'})); }
        function togglePump(id) { websocket.send(JSON.stringify({cmd: 'toggle', id: id})); }
        
        function saveSettings() {
            var h12 = parseInt(document.getElementById('feedHour').value);
            var m = parseInt(document.getElementById('feedMinute').value);
            var period = document.getElementById('feedAmPm').value;
            var dur = parseInt(document.getElementById('feedDuration').value);
            
            if (!isNaN(h12) && !isNaN(m)) {
                // Convert 12h back to 24h for Backend
                var h24 = h12;
                if (period === "PM" && h24 < 12) h24 += 12;
                if (period === "AM" && h24 === 12) h24 = 0;

                websocket.send(JSON.stringify({
                    cmd: 'save_settings',
                    h: h24,
                    m: m,
                    d: dur
                }));
                alert("Settings Saved!");
            } else {
                alert("Please enter valid time!");
            }
        }
        
        function saveWiFi() {
            var ssid = document.getElementById('ssid').value;
            var pass = document.getElementById('pass').value;
            if(ssid.length > 0) {
                 if(confirm("Device will restart to connect to " + ssid)) {
                    websocket.send(JSON.stringify({
                        cmd: 'save_wifi',
                        s: ssid,
                        p: pass
                    }));
                 }
            } else {
                alert("SSID cannot be empty");
            }
        }

        window.onload = initWebSocket;
    </script>
</body>
</html>
)rawliteral";

#endif
