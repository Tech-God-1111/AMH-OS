(() => {
  const $ = id => document.getElementById(id);
  const lock = $("lockscreen"), desktop = $("desktop"), loginInput = $("pin");
  let latestStatus = null, currentPath = "/", activeApp = "", taskTimer = null;
  const safe = value => String(value == null ? "" : value).replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
  function toast(message) {
    const el=$("toast"); el.textContent=message; el.classList.add("show");
    setTimeout(()=>el.classList.remove("show"),2600);
  }
  function clock() {
    const d=new Date();
    const t=d.toLocaleTimeString([], {hour:"2-digit",minute:"2-digit"});
    $("clock").textContent=t; $("lock-time").textContent=t;
  }
  clock(); setInterval(clock,1000);

  const defaultHash = "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4";
  async function digest(value) {
    if (!window.crypto || !crypto.subtle) return btoa(unescape(encodeURIComponent(value)));
    const bytes = new TextEncoder().encode(value);
    const hash = await crypto.subtle.digest("SHA-256", bytes);
    return Array.from(new Uint8Array(hash)).map(x=>x.toString(16).padStart(2,"0")).join("");
  }
  function loginMode(){ return localStorage.getItem("amh.login.mode") || "pin"; }
  async function unlock() {
    const mode=loginMode();
    if(mode==="none"){showDesktop();return;}
    const expected=localStorage.getItem("amh.login.hash") || defaultHash;
    const actual=await digest(loginInput.value);
    if(actual===expected){showDesktop();}
    else{$("pin-error").textContent="Incorrect credentials.";loginInput.value="";}
  }
  function showDesktop(){lock.classList.add("hidden");desktop.classList.remove("hidden");$("pin-error").textContent="";loginInput.value="";}
  function lockAgain(){desktop.classList.add("hidden");lock.classList.remove("hidden");$("menu-panel").classList.add("hidden");configureLoginScreen();}
  function configureLoginScreen(){
    const mode=loginMode();
    const label=$("login-label");
    if(label) label.textContent=mode==="password"?"PASSWORD":mode==="none"?"NO LOGIN REQUIRED":"PIN";
    loginInput.type=mode==="password"?"password":"password";
    loginInput.placeholder=mode==="password"?"Enter password":"Enter PIN";
    if(mode==="none") $("pin-error").textContent="Login disabled in Settings.";
    else if(!$("pin-error").textContent) $("pin-error").textContent="Default demo PIN: 1234";
  }
  $("unlock").addEventListener("click",unlock);
  loginInput.addEventListener("keydown",e=>{if(e.key==="Enter")unlock();});
  $("lock-again").onclick=lockAgain; $("menu-lock").onclick=lockAgain;
  configureLoginScreen();

  function connection(value){
    $("connection-dot").classList.toggle("online",value);
    $("connection-label").textContent=value?"ESP32 connected":"USB disconnected";
    $("dock-usb").textContent=value?"● ESP32 ONLINE":"● OFFLINE";
  }
  document.addEventListener("amh-serial-state",e=>{connection(e.detail);if(e.detail)toast("USB serial connected");});
  async function connect(){try{await window.AMHSerial.connect();}catch(e){toast(e.message);}}
  $("connect-main").onclick=connect;
  window.AMHSerial.onMessage(msg=>{
    if(msg.type==="status"){latestStatus=msg;renderStatus();}
    if(msg.type==="hello")toast(msg.name+" "+msg.version+" ready");
    if(msg.type==="command_result")appendTerminal(msg.output);
    if(msg.type==="error")toast(msg.message||"USB communication error");
  });
  function renderStatus(){
    const s=latestStatus||{}, wifi=s.wifi||{};
    const vals={"device-name":s.name||"AMH-OS","device-version":s.version||"0.3.0",
      "device-runtime":s.runtime||"MicroPython","device-heap":s.free_heap?Math.round(s.free_heap/1024)+" KB":"—",
      "device-uptime":s.uptime_ms!=null?Math.floor(s.uptime_ms/1000)+" sec":"—",
      "device-cpu":s.frequency_mhz?s.frequency_mhz+" MHz":"—","device-wifi":wifi.mode||"Unknown",
      "device-id":s.device_id||"—"};
    Object.keys(vals).forEach(id=>{const el=$(id);if(el)el.textContent=vals[id];});
  }
  async function request(payload,type,timeout){return window.AMHSerial.request(payload,type,timeout);}

  function appendTerminal(text){
    const out=$("terminal-output"); if(!out)return;
    if(text==="__CLEAR__"){out.textContent="AMH-OS Terminal 0.3.0\nType help for commands.\n";return;}
    out.textContent+="\n"+String(text)+"\n";
    out.scrollTop=out.scrollHeight;
  }
  async function runCommand(){
    const input=$("terminal-command"); if(!input)return;
    const command=input.value.trim(); if(!command)return;
    const out=$("terminal-output"); out.textContent+="\n$ "+command; input.value="";
    try{
      const result=await request({type:"command",command},"command_result",12000);
      if(result.output==="__CLEAR__")out.textContent="AMH-OS Terminal 0.3.0\nType help for commands.\n";
      else appendTerminal(result.output);
    }catch(e){appendTerminal("Error: "+e.message);}
  }

  async function loadFiles(path){
    currentPath=path||currentPath;
    const list=$("file-list"), pathBox=$("file-path");
    if(!list)return;
    list.innerHTML='<div class="file-row">Loading device filesystem…</div>';
    try{
      const result=await request({type:"fs",action:"list",path:currentPath},"fs_result",10000);
      currentPath=result.result.path; pathBox.value=currentPath;
      if(!result.result.entries.length){list.innerHTML='<p class="muted">This directory is empty.</p>';return;}
      list.innerHTML=result.result.entries.map(item=>{
        const dir=item.type==="directory";
        return '<div class="file-row" data-path="'+safe(item.path)+'" data-type="'+safe(item.type)+'"><span>'+ (dir?"📁":"📄") +' '+safe(item.name)+'</span><span>'+ (dir?"Folder":safe(item.size)+" B") +'</span><span><button class="tiny" data-open="'+safe(item.path)+'">'+(dir?"Open":"View")+'</button> <button class="tiny danger" data-delete="'+safe(item.path)+'" data-kind="'+(dir?"directory":"file")+'">Delete</button></span></div>';
      }).join("");
      list.querySelectorAll("[data-open]").forEach(btn=>btn.onclick=()=>openFile(btn.dataset.open));
      list.querySelectorAll("[data-delete]").forEach(btn=>btn.onclick=()=>deletePath(btn.dataset.delete,btn.dataset.kind));
    }catch(e){list.innerHTML='<p class="error">'+safe(e.message)+'</p>';}
  }
  async function openFile(path){
    try{
      const result=await request({type:"fs",action:"read",path},"fs_result",10000);
      const preview=$("file-preview"); if(preview)preview.textContent=result.result.content;
      const title=$("file-preview-title");if(title)title.textContent=path;
    }catch(e){toast(e.message);}
  }
  async function deletePath(path,kind){
    if(!confirm("Delete "+path+"? This cannot be undone."))return;
    try{
      await request({type:"fs",action:kind==="directory"?"rmdir":"remove",path},"fs_result",10000);
      toast("Deleted "+path);loadFiles(currentPath);
    }catch(e){toast(e.message);}
  }
  async function createEntry(kind){
    const name=prompt(kind==="directory"?"New folder name":"New file name");
    if(!name)return;
    const path=(currentPath==="/"?"":currentPath)+"/"+name;
    try{
      if(kind==="directory")await request({type:"fs",action:"mkdir",path},"fs_result",10000);
      else await request({type:"fs",action:"write",path,content:""},"fs_result",10000);
      loadFiles(currentPath);toast("Created "+name);
    }catch(e){toast(e.message);}
  }

  async function refreshTasks(){
    const area=$("task-data");if(!area)return;
    try{
      const response=await request({type:"tasks"},"tasks_result",8000), s=response.result;
      const rows=[
        ["Free heap",s.free_heap+" bytes"],
        ["Allocated heap",s.allocated_heap==null?"—":s.allocated_heap+" bytes"],
        ["Storage total",s.storage_total==null?"—":s.storage_total+" bytes"],
        ["Storage free",s.storage_free==null?"—":s.storage_free+" bytes"],
        ["CPU frequency",s.cpu_mhz==null?"—":s.cpu_mhz+" MHz"],
        ["Uptime",Math.floor(s.uptime_ms/1000)+" seconds"]
      ];
      area.innerHTML='<div class="stat-grid">'+rows.map(r=>'<div class="stat"><small>'+safe(r[0])+'</small><strong>'+safe(r[1])+'</strong></div>').join("")+'</div><h3>AMH-OS services</h3>'+s.services.map(x=>'<div class="file-row"><span>'+safe(x.name)+'</span><span>'+safe(x.state)+'</span></div>').join("")+'<p class="muted">'+safe(s.note||"")+'</p>';
    }catch(e){area.innerHTML='<p class="error">'+safe(e.message)+'</p>';}
  }

  async function refreshBle(){
    const area=$("ble-data");if(!area)return;
    try{
      const response=await request({type:"ble",action:"results"},"ble_result",8000);
      const result=response.result, devices=result.devices||[];
      area.innerHTML='<p>BLE: '+(result.status.supported?"Available":"Not available")+' · '+(result.status.scanning?"Scanning":"Idle")+'</p>'+
        (devices.length?devices.map(d=>'<div class="file-row"><span>'+safe(d.name||"Unnamed BLE device")+'</span><span>'+safe(d.address)+' · '+safe(d.rssi)+' dBm</span></div>').join(""):'<p class="muted">No devices found yet. Start a scan.</p>')+
        (result.status.error?'<p class="error">'+safe(result.status.error)+'</p>':"");
    }catch(e){area.innerHTML='<p class="error">'+safe(e.message)+'</p>';}
  }
  async function startBle(){
    try{const r=await request({type:"ble",action:"scan",duration_ms:10000},"ble_result",8000);toast(r.result.message||"BLE scan started");setTimeout(refreshBle,1200);setTimeout(refreshBle,5000);setTimeout(refreshBle,10500);}
    catch(e){toast(e.message);}
  }

  async function saveLogin(){
    const mode=$("login-mode").value, secret=$("login-secret").value;
    if(mode!=="none" && secret.length<4){toast("Use at least 4 characters.");return;}
    localStorage.setItem("amh.login.mode",mode);
    if(mode!=="none")localStorage.setItem("amh.login.hash",await digest(secret));
    else localStorage.removeItem("amh.login.hash");
    $("login-secret").value="";configureLoginScreen();toast("Login preference saved on this computer.");
  }
  function openBrowser(){
    let url=$("browser-url").value.trim();
    if(!url)return;
    if(!/^[a-zA-Z][a-zA-Z0-9+.-]*:/.test(url))url="https://"+url;
    try{const parsed=new URL(url);if(!["http:","https:"].includes(parsed.protocol))throw new Error("Only HTTP and HTTPS URLs are allowed.");window.open(parsed.href,"_blank","noopener,noreferrer");}
    catch(e){toast(e.message||"Invalid URL");}
  }

  const titles={computer:"Computer",monitor:"Task Manager",files:"File Explorer",settings:"Settings",about:"About AMH-OS",terminal:"Terminal",browser:"Web Browser",bluetooth:"Bluetooth"};
  const content={
    computer:()=>'<div class="stat-grid"><div class="stat"><small>DEVICE</small><strong id="device-name">AMH-OS</strong></div><div class="stat"><small>VERSION</small><strong id="device-version">0.3.0</strong></div><div class="stat"><small>RUNTIME</small><strong id="device-runtime">MicroPython</strong></div><div class="stat"><small>FREE HEAP</small><strong id="device-heap">Connect USB</strong></div></div><p>Live information from the ESP32-S3.</p>',
    monitor:()=>'<div id="task-data">Connect the ESP32, then refresh.</div><button class="primary" id="task-refresh">Refresh task manager</button>',
    files:()=>'<div class="toolbar"><input id="file-path" value="/" aria-label="Path"><button id="file-go">Go</button><button id="file-up">Up</button><button id="file-new-file">New file</button><button id="file-new-dir">New folder</button></div><div id="file-list" class="file-list">Connect the ESP32 to browse its filesystem.</div><h3 id="file-preview-title">File preview</h3><pre id="file-preview" class="file-preview">Select a text file to preview it.</pre>',
    terminal:()=>'<pre id="terminal-output" class="terminal">AMH-OS Terminal 0.3.0\nType help for commands.\n</pre><div class="terminal-row"><input id="terminal-command" placeholder="help, ls, cd, cat, mkdir, write, rm, ps, df"><button id="terminal-send">Run</button></div>',
    settings:()=>'<h3>Login</h3><p class="muted">Choose how this browser desktop is unlocked.</p><div class="toolbar"><select id="login-mode"><option value="pin">PIN</option><option value="password">Password</option><option value="none">No login</option></select><input id="login-secret" type="password" placeholder="New PIN or password"><button class="primary" id="login-save">Save login</button></div><p class="muted">This is a convenience lock stored in this browser, not device-level security.</p><h3>Bluetooth</h3><p>Manage BLE discovery on the ESP32-S3.</p><button id="settings-ble">Open Bluetooth</button><h3>Browser</h3><p>AMH-OS opens web pages in a new tab of the host browser.</p><button id="settings-browser">Open browser</button>',
    bluetooth:()=>'<p>ESP32-S3 Bluetooth Low Energy discovery.</p><button class="primary" id="ble-scan">Start 10-second scan</button> <button id="ble-refresh">Refresh results</button><div id="ble-data">Connect the ESP32 first.</div><p class="muted">Discovery only. Watch pairing and data access are not yet implemented.</p>',
    browser:()=>'<p>Open a website in a new tab on this computer.</p><div class="toolbar"><input id="browser-url" placeholder="https://example.com"><button class="primary" id="browser-open">Open</button></div><p class="muted">The ESP32-S3 does not render web pages; your computer's browser does.</p>',
    about:()=>'<div class="brand-mark">A</div><h2>AMH-OS</h2><p>Version 0.3.0 · MicroPython on ESP32-S3</p><p>Device-side shell, bounded filesystem service, resource monitor and BLE discovery.</p><p><b>Board:</b> ESP32-S3-WROOM-1-N16R8</p>'
  };
  function openApp(name){
    $("menu-panel").classList.add("hidden");
    if(name==="menu"){$("menu-panel").classList.toggle("hidden");return;}
    activeApp=name;
    const area=$("window-area");area.innerHTML="";
    const win=document.createElement("section");win.className="app-window";
    win.innerHTML='<div class="window-head"><span>'+safe(titles[name]||name)+'</span><button class="window-close" aria-label="Close">×</button></div><div class="window-body">'+(content[name]?content[name]():"")+'</div>';
    win.querySelector(".window-close").onclick=()=>{win.remove();activeApp="";if(taskTimer){clearInterval(taskTimer);taskTimer=null;}};
    area.appendChild(win);$("active-title").textContent=titles[name]||"Workspace";
    const bind=(id,fn)=>{const el=$(id);if(el)el.onclick=fn;};
    bind("task-refresh",refreshTasks);
    bind("file-go",()=>loadFiles($("file-path").value.trim()||"/"));
    bind("file-up",()=>{const p=currentPath.split("/").filter(Boolean);p.pop();loadFiles("/"+p.join("/"));});
    bind("file-new-file",()=>createEntry("file"));bind("file-new-dir",()=>createEntry("directory"));
    bind("terminal-send",runCommand);
    const terminalInput=$("terminal-command");if(terminalInput)terminalInput.addEventListener("keydown",e=>{if(e.key==="Enter")runCommand();});
    bind("login-save",saveLogin);
    if($("login-mode"))$("login-mode").value=loginMode();
    bind("settings-ble",()=>openApp("bluetooth"));bind("settings-browser",()=>openApp("browser"));
    bind("ble-scan",startBle);bind("ble-refresh",refreshBle);
    bind("browser-open",openBrowser);
    const browserInput=$("browser-url");if(browserInput)browserInput.addEventListener("keydown",e=>{if(e.key==="Enter")openBrowser();});
    if(name==="files")loadFiles("/");
    if(name==="computer")request({type:"status"},"status",8000).then(msg=>{latestStatus=msg;renderStatus();}).catch(()=>{});
    if(name==="monitor"){refreshTasks();taskTimer=setInterval(()=>{if(activeApp==="monitor")refreshTasks();},5000);}
    if(name==="bluetooth")refreshBle();
  }
  document.querySelectorAll("[data-app]").forEach(el=>el.addEventListener("click",()=>openApp(el.dataset.app)));
  document.addEventListener("keydown",e=>{if(e.key==="Escape")$("menu-panel").classList.add("hidden");});
})();