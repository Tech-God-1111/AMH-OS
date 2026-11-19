(() => {
  "use strict";

  const ENDPOINT = "http://127.0.0.1:8766/api/v1/status";
  const $ = id => document.getElementById(id);
  const safe = value => String(value == null ? "" : value).replace(/[&<>"']/g, c => ({
    "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;"
  }[c]));
  const formatBytes = value => {
    const n = Number(value);
    if (!Number.isFinite(n)) return "—";
    const units = ["B", "KB", "MB", "GB", "TB"];
    let amount = n, index = 0;
    while (amount >= 1024 && index < units.length - 1) {
      amount /= 1024;
      index++;
    }
    return amount.toFixed(index === 0 ? 0 : 1) + " " + units[index];
  };
  const formatUptime = seconds => {
    const n = Math.max(0, Number(seconds) || 0);
    const days = Math.floor(n / 86400);
    const hours = Math.floor((n % 86400) / 3600);
    const minutes = Math.floor((n % 3600) / 60);
    return days + "d " + hours + "h " + minutes + "m";
  };

  function panel() {
    const area = $("task-data");
    if (!area || $("host-task-panel")) return;
    const section = document.createElement("section");
    section.id = "host-task-panel";
    section.style.marginTop = "18px";
    section.innerHTML =
      '<h3>Windows host computer</h3>' +
      '<p class="muted" id="host-monitor-state">Checking optional local monitor…</p>' +
      '<div id="host-monitor-content"></div>' +
      '<p class="muted">Host metrics are read-only. Start tools/launch-host-monitor.ps1 in a separate PowerShell window to enable them.</p>';
    area.appendChild(section);
  }

  function render(data) {
    const state = $("host-monitor-state");
    const content = $("host-monitor-content");
    if (!state || !content) return;
    if (!data || !data.available) {
      state.textContent = "Host monitor offline — ESP32-S3 monitoring continues independently.";
      content.innerHTML = "";
      return;
    }
    state.textContent = "Connected · " + safe(data.computer) + " · Windows host";
    const m = data.memory || {}, d = data.storage || {}, net = data.network || {};
    const rows = [
      ["CPU usage", Number(data.cpu_percent).toFixed(1) + "%"],
      ["CPU cores", data.cpu_physical_cores + " physical / " + data.cpu_logical_cores + " logical"],
      ["RAM", formatBytes(m.used) + " / " + formatBytes(m.total) + " (" + m.percent + "%)"],
      ["System drive", d.error || (formatBytes(d.used) + " used / " + formatBytes(d.total) + " total")],
      ["Windows uptime", formatUptime(data.uptime_seconds)],
      ["Processes", data.process_count],
      ["Running Windows services", data.running_windows_services == null ? "Unavailable" : data.running_windows_services],
      ["Network sent / received", formatBytes(net.bytes_sent) + " / " + formatBytes(net.bytes_received)]
    ];
    content.innerHTML =
      '<div class="stat-grid">' +
      rows.map(row => '<div class="stat"><small>' + safe(row[0]) +
        '</small><strong>' + safe(row[1]) + '</strong></div>').join("") +
      '</div><h3>Top processes by memory</h3>' +
      ((data.top_processes_by_memory || []).map(proc =>
        '<div class="file-row"><span>' + safe(proc.name) + ' · PID ' + safe(proc.pid) +
        '</span><span>' + safe(formatBytes(proc.memory_bytes)) + '</span></div>'
      ).join("") || '<p class="muted">No process data.</p>');
  }

  async function refresh() {
    if (!$("task-data")) return;
    panel();
    try {
      const response = await fetch(ENDPOINT, { cache: "no-store" });
      const data = await response.json();
      render(data);
    } catch (_) {
      render({ available: false });
    }
  }

  const observer = new MutationObserver(() => {
    if ($("task-data")) {
      panel();
      refresh();
    }
  });
  const root = $("window-area");
  if (root) observer.observe(root, { childList: true });
  window.setInterval(() => {
    if ($("task-data")) refresh();
  }, 5000);
  document.addEventListener("amh-serial-state", () => {
    if ($("task-data")) refresh();
  });
})();