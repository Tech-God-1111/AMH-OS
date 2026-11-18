(() => {
  const $ = id => document.getElementById(id);
  const lock = $("lockscreen"), desktop = $("desktop"), pin = $("pin");
  let latestStatus = null;
  function toast(message) { const el=$("toast"); el.textContent=message; el.classList.add("show"); setTimeout(()=>el.classList.remove("show"),2400); }
  function clock() { const d=new Date(); const t=d.toLocaleTimeString([], {hour:"2-digit",minute:"2-digit"}); $("clock").textContent=t; $("lock-time").textContent=t; }
  clock(); setInterval(clock,1000);
  function unlock() { if(pin.value==="1234"){lock.classList.add("hidden");desktop.classList.remove("hidden");$("pin-error").textContent="";pin.value="";}else{$("pin-error").textContent="Incorrect demo PIN. Try 1234.";pin.value="";} }
  $("unlock").addEventListener("click",unlock); pin.addEventListener("keydown",e=>{if(e.key==="Enter")unlock();});
  function lockAgain(){desktop.classList.add("hidden");lock.classList.remove("hidden");$("menu-panel").classList.add("hidden");}
  $("lock-again").onclick=lockAgain; $("menu-lock").onclick=lockAgain;
  function connection(value){$("connection-dot").classList.toggle("online",value);$("connection-label").textContent=value?"ESP32 connected":"USB disconnected";$("dock-usb").textContent=value?"● ESP32 ONLINE":"● OFFLINE";}
  document.addEventListener("amh-serial-state",e=>{connection(e.detail);if(e.detail)toast("USB serial connected");});
  async function connect(){try{await window.AMHSerial.connect();}catch(e){toast(e.message);}}
  $("connect-main").onclick=connect;
  window.AMHSerial.onMessage(msg=>{
    if(msg.type==="status"){latestStatus=msg;renderStatus();}
    if(msg.type==="hello")toast(msg.name+" "+msg.version+" ready");
    if(msg.type==="command_result"){const out=$("terminal-output");if(out)out.textContent+="\n"+msg.output+"\n";}
    if(msg.type==="error")toast(msg.message||"USB communication error");
  });
  function renderStatus(){
    const s=latestStatus||{};
    const wifi=s.wifi||{};
    const vals={"device-name":s.name||"AMH-OS","device-version":s.version||"0.2.0","device-runtime":s.runtime||"MicroPython","device-heap":s.free_heap?Math.round(s.free_heap/1024)+" KB":"—","device-uptime":s.uptime_ms?Math.floor(s.uptime_ms/1000)+" sec":"—","device-cpu":s.frequency_mhz?s.frequency_mhz+" MHz":"—","device-wifi":wifi.mode||"Unknown","device-id":s.device_id||"—"};
    Object.keys(vals).forEach(id=>{const el=$(id);if(el)el.textContent=vals[id];});
  }
  const content={
    computer:()=>'<div class="stat-grid"><div class="stat"><small>DEVICE</small><strong id="device-name">AMH-OS</strong></div><div class="stat"><small>VERSION</small><strong id="device-version">0.2.0</strong></div><div class="stat"><small>RUNTIME</small><strong id="device-runtime">MicroPython</strong></div><div class="stat"><small>FREE HEAP</small><strong id="device-heap">Connect USB</strong></div></div><p>Connect the ESP32-S3 to load live hardware information.</p>',
    monitor:()=>'<div class="stat-grid"><div class="stat"><small>UPTIME</small><strong id="device-uptime">—</strong></div><div class="stat"><small>CPU FREQUENCY</small><strong id="device-cpu">—</strong></div><div class="stat"><small>WI-FI MODE</small><strong id="device-wifi">—</strong></div><div class="stat"><small>DEVICE ID</small><strong id="device-id">—</strong></div></div><p>Values are read from the connected ESP32-S3, not simulated.</p><button class="primary" id="refresh-status">Refresh status</button>',
    files:()=>'<div class="file-row"><span>📁 firmware</span><span>System</span></div><div class="file-row"><span>📁 web</span><span>Interface</span></div><div class="file-row"><span>📄 boot.py</span><span>Startup</span></div><div class="file-row"><span>📄 main.py</span><span>Startup</span></div><p>Demo view of the AMH-OS application layout.</p>',
    settings:()=>'<div class="stat-grid"><div class="stat"><small>INTERFACE</small><strong>AMH Desktop</strong></div><div class="stat"><small>TRANSPORT</small><strong>USB Serial</strong></div></div><p>Display: browser fullscreen recommended. Serial speed: 115200 baud.</p><button class="primary" id="settings-connect">Connect ESP32</button>',
    about:()=>'<div class="brand-mark">A</div><h2>AMH-OS Desktop</h2><p>Version 0.2.0 · School demonstration build</p><p>MicroPython-powered ESP32-S3 system core with a browser-rendered desktop environment.</p><p><b>Hardware:</b> ESP32-S3-WROOM-1-N16R8</p>',
    terminal:()=>'<div id="terminal-output" class="terminal">AMH-OS Terminal [demo]\nType help to see available commands.\n$ </div><div class="terminal-row"><input id="terminal-command" placeholder="help, version, sys, wifi"><button id="terminal-send">Run</button></div>',
  };
  const titles={computer:"Computer",monitor:"System Monitor",files:"File Manager",settings:"Settings",about:"About AMH-OS",terminal:"Terminal"};
  function openApp(name){
    $("menu-panel").classList.add("hidden");
    if(name==="menu"){$("menu-panel").classList.toggle("hidden");return;}
    const area=$("window-area");area.innerHTML="";
    const win=document.createElement("section");win.className="app-window";
    win.innerHTML='<div class="window-head"><span>'+titles[name]+'</span><button class="window-close" aria-label="Close">×</button></div><div class="window-body">'+(content[name]?content[name]():"")+'</div>';
    win.querySelector(".window-close").onclick=()=>win.remove();area.appendChild(win);
    $("active-title").textContent=titles[name]||"Workspace";
    const refresh=$("refresh-status");if(refresh)refresh.onclick=()=>window.AMHSerial.send({type:"status"}).catch(e=>toast(e.message));
    const sc=$("settings-connect");if(sc)sc.onclick=connect;
    const run=()=>{const input=$("terminal-command");if(!input)return;const command=input.value.trim().toLowerCase();if(!command)return;const out=$("terminal-output");out.textContent+="\n$ "+command;window.AMHSerial.send({type:"command",command}).catch(e=>{out.textContent+="\n"+e.message;});input.value="";};
    const send=$("terminal-send");if(send)send.onclick=run;
    const input=$("terminal-command");if(input)input.addEventListener("keydown",e=>{if(e.key==="Enter")run();});
    if(name==="computer"||name==="monitor")window.AMHSerial.send({type:"status"}).catch(()=>{});
  }
  document.querySelectorAll("[data-app]").forEach(el=>el.addEventListener("click",()=>openApp(el.dataset.app)));
  document.addEventListener("keydown",e=>{if(e.key==="Escape")$("menu-panel").classList.add("hidden");});
})();