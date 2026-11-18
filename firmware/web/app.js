const el = (id) => document.getElementById(id);
const toast = (text) => { const node=el("toast"); node.textContent=text; node.classList.add("show"); setTimeout(()=>node.classList.remove("show"),2400); };
const bytes = (n) => n>=1048576?(n/1048576).toFixed(2)+" MB":n>=1024?(n/1024).toFixed(1)+" KB":(n||0)+" B";
const uptime = (n) => { let s=Math.floor((n||0)/1000),h=Math.floor(s/3600);s%=3600;let m=Math.floor(s/60);s%=60;return h+"h "+m+"m "+s+"s"; };
async function json(path){const r=await fetch(path,{cache:"no-store"});const d=await r.json();if(!r.ok)throw new Error(d.error||"Request failed");return d;}
async function refresh(){
 try{
  const d=await json("/api/status"),s=d.system||{},w=d.wifi||{};
  el("heap").textContent=bytes(s.free_heap);el("freq").textContent=s.frequency_mhz||"—";el("uptime").textContent=uptime(s.uptime_ms);
  el("device").textContent=s.name||"AMH-OS Lite";el("runtime").textContent=s.runtime||"MicroPython";el("version").textContent=s.version||"—";el("footerversion").textContent=s.version||"0.1.0";
  el("deviceid").textContent=s.device_id||"unavailable";el("mode").textContent=w.mode||"unknown";el("ip").textContent=w.ip||"unavailable";
  el("connection").textContent=w.connected?"Wi-Fi connected":w.mode==="setup_ap"?"Setup access point":"Local device";el("dot").classList.toggle("online",!!w.connected);
 }catch(e){el("connection").textContent="Device unavailable";el("dot").classList.remove("online");}
}
async function refreshFiles(){
 try{const d=await json("/api/files"),list=el("files");list.textContent="";(d.items||[]).forEach(item=>{const row=document.createElement("div");row.className="file";const name=document.createElement("span");name.textContent="▰ "+item.name;const type=document.createElement("span");type.textContent=item.type;row.append(name,type);list.appendChild(row);});if(!list.children.length)list.textContent="No entries found.";}
 catch(e){el("files").textContent="Filesystem information unavailable.";}
}
el("wifi-form").addEventListener("submit",async event=>{
 event.preventDefault();const button=event.currentTarget.querySelector("button");button.disabled=true;el("message").textContent="Saving and connecting…";
 try{const r=await fetch("/api/wifi",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({wifi_ssid:el("ssid").value.trim(),wifi_password:el("password").value})});const d=await r.json();if(!r.ok)throw new Error(d.error||"Could not save settings");el("message").textContent="Settings saved. Check serial console for the new IP.";toast("Wi-Fi settings saved");setTimeout(refresh,1000);}
 catch(e){el("message").textContent=e.message;}finally{button.disabled=false;}
});
refresh();refreshFiles();setInterval(refresh,5000);
