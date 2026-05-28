const { createApp, reactive, onMounted, computed } = Vue;

createApp({
  setup() {
    const state = reactive({
      snapshot: null,
      dut: { port: null, present: false },
      history: [],
      error: '',
    });

    async function refreshDut() {
      try {
        state.dut = await fetch('/api/dut').then(r => r.json());
      } catch (e) { /* transient */ }
    }

    async function refreshHistory() {
      state.history = await fetch('/api/history').then(r => r.json());
    }

    async function startRun(flashOnly = false) {
      state.error = '';
      const r = await fetch('/api/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ flash_only: flashOnly }),
      }).then(r => r.json());
      if (!r.ok) state.error = r.err || 'failed to start';
    }

    function subscribe() {
      const es = new EventSource('/api/events');
      es.onmessage = (e) => {
        state.snapshot = JSON.parse(e.data);
        if (state.snapshot.state.overall === 'pass' ||
            state.snapshot.state.overall === 'fail') {
          refreshHistory();
        }
      };
    }

    const overall = computed(() => state.snapshot?.state?.overall ?? 'idle');
    const steps = computed(() => state.snapshot?.state?.steps ?? []);
    const run = computed(() => state.snapshot?.state ?? {});
    const logLines = computed(() => state.snapshot?.state?.log_tail ?? []);
    const canStart = computed(() => state.dut.present && overall.value !== 'running');

    onMounted(() => {
      refreshDut();
      refreshHistory();
      subscribe();
      setInterval(refreshDut, 1500);
    });

    return { state, startRun, overall, steps, run, logLines, canStart };
  },
  template: `
    <header>
      <h1>DoggoLights Testing Jig</h1>
      <span :class="['pill', overall]">{{ overall }}</span>
    </header>

    <section class="controls">
      <span :class="['dut-pill', state.dut.present ? 'on' : 'off']">
        DUT: {{ state.dut.port || 'not detected' }}
      </span>
      <button @click="startRun(false)" :disabled="!canStart">Start test</button>
      <button @click="startRun(true)" :disabled="!canStart">Flash only</button>
      <span class="err" v-if="state.error">{{ state.error }}</span>
    </section>

    <section class="dut">
      <div><label>MAC</label><span>{{ run.dut_mac || '-' }}</span></div>
      <div><label>Short</label><span>{{ run.dut_serial_short || '-' }}</span></div>
      <div><label>FW</label><span>{{ run.firmware_version || '-' }}</span></div>
    </section>

    <section class="steps">
      <h2>Steps</h2>
      <ul>
        <li v-for="s in steps" :key="s.name" :class="s.status">
          <span class="name">{{ s.name }}</span>
          <span class="status">{{ s.status }}</span>
          <span class="msg" v-if="s.message">{{ s.message }}</span>
        </li>
      </ul>
    </section>

    <section class="log">
      <h2>Log</h2>
      <pre>{{ logLines.join('\\n') }}</pre>
    </section>

    <section class="history">
      <h2>Recent runs</h2>
      <table>
        <thead><tr>
          <th>Time</th><th>MAC</th><th>FW</th><th>Result</th><th>Duration</th>
        </tr></thead>
        <tbody>
          <tr v-for="r in state.history" :key="r.id" :class="r.overall">
            <td>{{ new Date(r.timestamp * 1000).toLocaleString() }}</td>
            <td>{{ r.mac || '-' }}</td>
            <td>{{ r.firmware || '-' }}</td>
            <td>{{ r.overall }}</td>
            <td>{{ r.duration_s ? r.duration_s.toFixed(1) + 's' : '-' }}</td>
          </tr>
        </tbody>
      </table>
    </section>
  `,
}).mount('#app');
