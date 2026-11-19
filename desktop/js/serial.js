(function () {
  const state = { port: null, reader: null, writer: null, buffer: "", connected: false };
  const listeners = [];
  function emit(message) { listeners.slice().forEach(fn => { try { fn(message); } catch (_) {} }); }
  function setConnected(value) {
    state.connected = value;
    document.dispatchEvent(new CustomEvent("amh-serial-state", { detail: value }));
  }
  async function connect() {
    if (!("serial" in navigator)) throw new Error("Web Serial is unavailable. Use current Chrome or Edge on localhost.");
    state.port = await navigator.serial.requestPort();
    await state.port.open({ baudRate: 115200 });
    state.writer = state.port.writable.getWriter();
    setConnected(true);
    readLoop();
    await send({ type: "hello" });
    await send({ type: "status" });
  }
  async function readLoop() {
    try {
      const reader = state.port.readable.getReader();
      state.reader = reader;
      const decoder = new TextDecoder();
      while (true) {
        const result = await reader.read();
        if (result.done) break;
        state.buffer += decoder.decode(result.value, { stream: true });
        let split;
        while ((split = state.buffer.indexOf("\n")) >= 0) {
          const line = state.buffer.slice(0, split).trim();
          state.buffer = state.buffer.slice(split + 1);
          if (!line) continue;
          try { emit(JSON.parse(line)); } catch (_) { /* Ignore boot text. */ }
        }
      }
    } catch (error) {
      emit({ type: "error", message: error.message });
    } finally {
      setConnected(false);
      try { if (state.reader) state.reader.releaseLock(); } catch (_) {}
      state.reader = null;
    }
  }
  async function send(payload) {
    if (!state.connected || !state.writer) throw new Error("Connect the ESP32 over USB first.");
    await state.writer.write(new TextEncoder().encode(JSON.stringify(payload) + "\n"));
  }
  function request(payload, expectedType, timeoutMs) {
    return new Promise((resolve, reject) => {
      if (!state.connected) { reject(new Error("Connect the ESP32 over USB first.")); return; }
      let finished = false;
      const done = (fn, value) => {
        if (finished) return;
        finished = true;
        clearTimeout(timer);
        const index = listeners.indexOf(handler);
        if (index >= 0) listeners.splice(index, 1);
        fn(value);
      };
      const handler = message => {
        if (message.type === "error") done(reject, new Error(message.message || "Device error"));
        else if (message.type === expectedType) done(resolve, message);
      };
      const timer = setTimeout(() => done(reject, new Error("ESP32 request timed out.")), timeoutMs || 7000);
      listeners.push(handler);
      send(payload).catch(error => done(reject, error));
    });
  }
  async function disconnect() {
    try { if (state.reader) await state.reader.cancel(); } catch (_) {}
    try { if (state.writer) { await state.writer.close(); state.writer.releaseLock(); } } catch (_) {}
    try { if (state.port) await state.port.close(); } catch (_) {}
    state.port = state.reader = state.writer = null;
    setConnected(false);
  }
  window.AMHSerial = {
    connect, send, request, disconnect,
    onMessage: fn => listeners.push(fn),
    get connected() { return state.connected; }
  };
})();