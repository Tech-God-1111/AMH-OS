(function () {
  const state = { port: null, reader: null, writer: null, buffer: "", connected: false };
  const listeners = [];
  function emit(message) { listeners.forEach(fn => fn(message)); }
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
          try { emit(JSON.parse(line)); } catch (_) { /* Ignore boot text and non-JSON serial output. */ }
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
  async function disconnect() {
    try { if (state.reader) await state.reader.cancel(); } catch (_) {}
    try { if (state.writer) { await state.writer.close(); state.writer.releaseLock(); } } catch (_) {}
    try { if (state.port) await state.port.close(); } catch (_) {}
    state.port = state.reader = state.writer = null;
    setConnected(false);
  }
  window.AMHSerial = { connect, send, disconnect, onMessage: fn => listeners.push(fn), get connected() { return state.connected; } };
})();