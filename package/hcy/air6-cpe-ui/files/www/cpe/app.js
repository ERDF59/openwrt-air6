'use strict';

const { createApp, reactive, computed, onMounted, onUnmounted, nextTick } = Vue;

const SID0 = '00000000000000000000000000000000';
const NAV = [
  { id: 'overview',   label: '网络概览', icon: 'grid' },
  { id: 'cellular',   label: '蜂窝网络', icon: 'signal' },
  { id: 'wireless',   label: '无线网络', icon: 'wifi' },
  { id: 'clients',    label: '连接设备', icon: 'devices' },
  { id: 'traffic',    label: '速率与流量', icon: 'updown' },
  { id: 'security',   label: '网络与安全', icon: 'shield' },
  { id: 'services',   label: '服务与插件', icon: 'services' },
  { id: 'interfaces', label: '设备与接口', icon: 'chip' },
  { id: 'sms',        label: '短信',     icon: 'sms' },
  { id: 'tools',        label: '工具与日志', icon: 'gauge' },
  { id: 'system',     label: '系统',     icon: 'gear' },
];

const TITLES = {
  overview: ['网络概览', '实时了解蜂窝连接、无线网络与设备状态。'],
  cellular: ['蜂窝网络', 'SIM、注册、频段与模组控制。'],
  sms:      ['短信', '收发短信与通知。'],
  traffic:  ['速率与流量', '接口实时速率、累计流量与峰值。'],
  security: ['网络与安全', 'WAN、DNS、DHCP 与安全入口。'],
  services: ['服务与插件', '系统服务运行状态与重启。'],
  interfaces: ['设备与接口', '网络接口与无线设备状态。'],
  wireless: ['无线网络', '2.4 GHz / 5 GHz / 6 GHz 配置与射频状态。'],
  clients:  ['连接设备', '无线关联与 LAN 邻居记录；缓存不代表持续在线。'],
  tools:    ['工具与日志', '网络诊断、模组快捷控制、系统日志与外观。'],
  home: ['我的首页','自选模块与布局。'],
  system:   ['系统', '固件、内存、指示灯与维护。'],
};

const store = reactive({
  token: sessionStorage.getItem('a6.token') || '',
  view: (location.hash || '#/overview').replace(/^#\//, '') || 'overview',
  loginError: '', busy: false,
  toast: '', toastTimer: 0,
  status: null, accel: null, board: null, info: null, ca: null, caErr: '',
  samples: [], seq: 0, rxRate: 0, txRate: 0,
  sigHist: [], sigSeq: 0, smsUnread: 0,
  lastRx: null, lastTx: null, lastTs: 0, lastDev: '',
  statusUpdated: 0, statusError: '', clock: Date.now(), sessionEpoch: 0,
  wifiError: '', ratBusy: false, actionBusy: false,
  clients: [], clientsLoading: false,
  wifi: null, wifiLoading: false, wifiApplying: false, wifiNote: '',
  atOut: '', atBusy: false, atCmd: 'AT+CSQ', simSlot: '', simBusy: false, bandOut: '', bandBusy: false,
  timers: [],
  editing: false, resetTick: 0,
});

function toast(msg) {
  store.toast = msg;
  clearTimeout(store.toastTimer);
  store.toastTimer = setTimeout(() => { store.toast = ''; }, 2600);
}

const pendingRpc = new Map();
async function ubus(obj, method, args, sid) {
  const readOnly = /^(get_|list_)/.test(method) || ['board','info','devices','assoclist'].includes(method);
  const key = JSON.stringify([sid || store.token || SID0, obj, method, args || {}]);
  if (!readOnly) return rpcCall(obj, method, args, sid);
  if (pendingRpc.has(key)) return pendingRpc.get(key);
  const promise = rpcCall(obj, method, args, sid).finally(() => pendingRpc.delete(key));
  pendingRpc.set(key, promise);
  return promise;
}

async function rpcCall(obj, method, args, sid) {
  const body = {
    jsonrpc: '2.0', id: 1, method: 'call',
    params: [sid || store.token || SID0, obj, method, args || {}],
  };
  const epoch = store.sessionEpoch;
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 45000);
  let res;
  try { res = await fetch('/ubus', {
    signal: controller.signal,
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });
  if (!res.ok) throw new Error('HTTP ' + res.status);
  const j = await res.json();
  if (epoch !== store.sessionEpoch) { const error=new Error('会话已变更'); error.staleSession=true; throw error; }
  if (j.error) throw new Error(j.error.message || 'ubus 调用失败');
  const r = j.result || [];
  if (r[0] !== 0) { const e = new Error(r[0] === 6 ? '会话已过期，请重新登录' : 'ubus ' + r[0]); e.code = r[0]; if(r[0] === 6 && store.token && ((obj === 'luci.air6modem' && method === 'get_status') || (obj === 'system' && ['board','info'].includes(method)))) logout(); throw e; }
  return r[1];
  } finally { clearTimeout(timer); }
}

function isAuthError(e) {
  return e && (e.code === 6 || /permission|access denied/i.test(e.message || ''));
}

function logout() {
  const token = store.token;
  store.sessionEpoch++;
  store.timers.forEach(clearInterval); store.timers = [];
  if (token) ubus('session', 'destroy', {}, token).catch(() => {});
  store.editing = false;
  store.token = '';
  sessionStorage.removeItem('a6.token');
  store.status = null; store.wifi = null; store.clients = [];
  store.accel = null; store.board = null; store.info = null; store.ca = null;
  store.statusUpdated = 0; store.statusError = ''; store.caErr = '';
  store.samples = []; store.sigHist = []; store.smsUnread = 0;
  store.lastRx = null; store.lastTx = null; store.lastTs = 0; store.lastDev = '';
  store.rxRate = 0; store.txRate = 0; store.atOut = ''; store.bandOut = ''; store.simSlot = '';
}

async function doLogin(pass) {
  store.busy = true; store.loginError = '';
  try {
    const r = await ubus('session', 'login',
      { username: 'root', password: pass, timeout: 3600 }, SID0);
    if (!r || !r.ubus_rpc_session) throw new Error('登录失败');
    store.token = r.ubus_rpc_session;
    sessionStorage.setItem('a6.token', store.token);
    await boot();
  } catch (e) {
    store.loginError = (e.code === 6) ? '密码错误，请重试' : '无法登录：' + e.message;
  } finally {
    store.busy = false;
  }
}

function pickWan(interfaces) {
  const list = interfaces || [];
  const n = store.status && store.status.network;
  const dev = n && (n.l3_device || n.device);
  if (dev) {
    const hit = list.find(i => i.name === dev);
    if (hit) return hit;
  }
  const cm = list.filter(i => /^ccmni/.test(i.name));
  if (cm.length) return cm.sort((a, b) => (b.rx_bytes + b.tx_bytes) - (a.rx_bytes + a.tx_bytes))[0];
  return null;
}

function computeRates(a) {
  const dev = pickWan(a && a.interfaces);
  if (!dev) { store.rxRate = 0; store.txRate = 0; store.lastRx = null; store.lastDev = ''; return; }
  const now = (a.ts ? a.ts * 1000 : Date.now());
  if (store.lastRx != null && store.lastDev === dev.name && now > store.lastTs && dev.rx_bytes >= store.lastRx && dev.tx_bytes >= store.lastTx) {
    const dt = Math.max((now - store.lastTs) / 1000, 0.5);
    const rx = Math.max(0, (dev.rx_bytes - store.lastRx) * 8 / dt / 1e6);
    const tx = Math.max(0, (dev.tx_bytes - store.lastTx) * 8 / dt / 1e6);
    store.rxRate = rx; store.txRate = tx;
    store.samples.push({ rx: rx, tx: tx });
    if (store.samples.length > 60) store.samples.shift();
    store.seq++;
  }
  else { store.rxRate = 0; store.txRate = 0; }
  store.lastDev = dev.name;
  store.lastRx = dev.rx_bytes; store.lastTx = dev.tx_bytes; store.lastTs = now;
}

async function refreshStatus() {
  try {
    const epoch = store.sessionEpoch;
    const status = await ubus('luci.air6modem', 'get_status');
    if (!store.token || epoch !== store.sessionEpoch) return;
    store.status = status; store.statusUpdated = Date.now(); store.statusError = '';
    const r = signalOf(store.status).rsrp;
    if (r != null) {
      store.sigHist.push(r);
      if (store.sigHist.length > 40) store.sigHist.shift();
      store.sigSeq++;
    }
  } catch (e) {
    if(e.staleSession) return;
    store.statusError = '状态刷新失败'; store.status = null;
    if (isAuthError(e)) logout();
  }
}

async function refreshCa() {
  try {
    const r = await ubus('air6.cpe', 'get_ca');
    if (r && r.ok && r.raw) {
      const j = JSON.parse(r.raw);
      store.ca = (j && j.ok !== false && Array.isArray(j.cells) && !r.stale) ? j : null;
      store.caErr = store.ca ? '' : (r.stale ? '载波数据已过期，等待重新采集' : '载波采集失败');
    } else {
      store.ca = null;
      store.caErr = (r && r.error) || '载波数据尚未就绪';
    }
  } catch (e) { store.ca = null; store.caErr = '载波数据读取失败'; if (isAuthError(e)) logout(); }
}

async function refreshUnread() {
  try {
    const r = await ubus('luci.air6modem', 'list_sms');
    requireSuccess(r);
    const n = ((r && r.output) || '').split('\n').filter(l => /^\+CMGL:.*UNREAD/.test(l)).length;
    store.smsUnread = n;
  } catch (e) {}
}

async function refreshAccel() {
  try {
    const a = await ubus('luci.air6modem', 'get_accel_stats');
    store.accel = a; computeRates(a);
  } catch (e) { if (isAuthError(e)) logout(); }
}

async function refreshSystem() {
  try {
    store.board = await ubus('system', 'board');
    store.info = await ubus('system', 'info');
  } catch (e) { if (isAuthError(e)) logout(); }
}

function startTimers() {
  store.timers.forEach(clearInterval);
  store.timers = [];
  if (!store.token) return;
  store.timers.push(setInterval(() => { store.clock = Date.now(); }, 1000));
  store.timers.push(setInterval(() => { if (!document.hidden) refreshStatus(); }, 10000));
  store.timers.push(setInterval(() => { if (!document.hidden) refreshSystem(); }, 30000));
  store.timers.push(setInterval(() => { if (!document.hidden) refreshAccel(); }, 4000));
  store.timers.push(setInterval(() => { if (!document.hidden) refreshCa(); }, 15000));
  store.timers.push(setInterval(() => { if (!document.hidden) refreshUnread(); }, 120000));
}

async function boot() {
  await Promise.allSettled([refreshStatus(), refreshAccel(), refreshSystem(), refreshCa()]);
  if (store.token) { refreshUnread(); startTimers(); }
}

/* ---------- helpers ---------- */
function numFrom(re, text) {
  const m = re.exec(text || '');
  return m ? parseFloat(m[1]) : null;
}

function signalOf(st) {
  if (!st) return { rsrp: null, rsrq: null, sinr: null, rssi: null };
  const src = (st.cesq || '') + ' ' + (st.signal || '');
  return {
    rsrp: numFrom(/RSRP=(-?\d+(?:\.\d+)?)/, src),
    rsrq: numFrom(/RSRQ=(-?\d+(?:\.\d+)?)/, src),
    sinr: numFrom(/SINR=(-?\d+(?:\.\d+)?)/, src),
    rssi: numFrom(/RSSI=(-?\d+(?:\.\d+)?)/, src),
  };
}

const OPERATORS = {
  '46000': '中国移动', '46002': '中国移动', '46004': '中国移动', '46007': '中国移动',
  '46001': '中国联通', '46006': '中国联通', '46009': '中国联通',
  '46003': '中国电信', '46005': '中国电信', '46011': '中国电信',
  '44010': 'NTT docomo', '44020': 'SoftBank', '44000': 'SoftBank',
};

function opName(code) { return !code ? '—' : (OPERATORS[code] || code); }

function fmtBytes(n) {
  n = Number(n) || 0;
  if (n < 1024) return Math.round(n) + ' B';
  const u = ['KB', 'MB', 'GB', 'TB'];
  let i = -1;
  do { n /= 1024; i++; } while (n >= 1024 && i < u.length - 1);
  return n.toFixed(n >= 100 ? 0 : 1) + ' ' + u[i];
}

function fmtUptime(sec) {
  sec = Number(sec) || 0;
  const d = Math.floor(sec / 86400);
  const h = Math.floor((sec % 86400) / 3600);
  const m = Math.floor((sec % 3600) / 60);
  if (d) return d + ' 天 ' + h + ' 小时';
  if (h) return h + ' 小时 ' + m + ' 分';
  return m + ' 分';
}

function ratShort(rat) {
  const t = String(rat || '').toUpperCase();
  if (t.includes('NR') || t.includes('5G')) return '5G';
  if (t.includes('LTE') || t.includes('4G')) return '4G';
  if (t.includes('WCDMA') || t.includes('3G')) return '3G';
  return t || '—';
}

function signalPct(rsrp) {
  if (rsrp == null) return 0;
  return Math.max(0, Math.min(100, Math.round((rsrp + 120) / 60 * 100)));
}

function bandOfDev(dev) {
  if (/^rai/.test(dev)) return '5G';
  if (/^rax/.test(dev)) return '6G';
  return '2.4G';
}

function channelOptions(band) {
  if (band === '2g') return Array.from({length: 14}, (_, i) => i);
  if (band === '5g') return [0, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165];
  return [0, 5, 37, 69, 101, 133, 165, 197, 229];
}

function htmodeOptions(radio) {
  const mt = radio.mt || {};
  const widths = [20, 40, 80, 160, 320].filter(w => w <= (mt.max_bw_mhz || (radio.band === '2g' ? 40 : 160)));
  const out = [];
  for (const [prefix, enabled, max] of [['HT', mt.ht, 40], ['VHT', mt.vht, 160], ['HE', mt.he, 160], ['EHT', mt.eht, 320]]) {
    if (enabled) widths.filter(w => w <= max).forEach(w => out.push(prefix + w));
  }
  if (radio.htmode && !out.includes(radio.htmode)) out.unshift(radio.htmode);
  return out;
}

/* ---------- grid mixin ---------- */
function caAllCells() {
  const age = store.clock / 1000 - Number(store.ca && store.ca.ts);
  return store.ca && Number.isFinite(age) && age >= -30 && age <= 60 && Array.isArray(store.ca.cells) ? store.ca.cells : [];
}
function caDlCells() { return caAllCells().filter(c => c.dir === 'dl'); }
function caIsActive() { return ['lte','nr'].some(rat => caDlCells().filter(c => c.rat === rat && c.state === 2).length > 1); }
function caBandText(c) { return (c.rat === 'nr' ? 'n' : 'B') + c.band; }
function caSummaryText() {
  if (!store.ca) return '载波数据采集不可用';
  if (store.ca.ca && !store.ca.ca.valid) return caDlCells().length ? 'CA 状态未知 · 显示服务小区' : '载波接口未返回有效数据';
  const d = caDlCells();
  if (!d.length) return '暂无有效载波数据';
  if (d.length === 1) return '单载波 · ' + caBandText(d[0]) + ' ' + d[0].bw;
  const active = ['lte', 'nr'].map(rat => ({rat, count: d.filter(c => c.rat === rat && c.state === 2).length}));
  const aggregated = active.filter(x => x.count > 1);
  if (aggregated.length) return aggregated.map(x => x.rat.toUpperCase() + ' ' + x.count + 'CC').join(' / ') + ' 聚合激活 · ' + d.map(caBandText).join(' + ');
  const dual = active.every(x => x.count > 0);
  return (dual ? 'LTE + NR 双载波，未确认同制式 CA' : d.length + ' 个下行载波已配置，未确认聚合激活') + ' · ' + d.map(caBandText).join(' + ');
}
function caAgeText() {
  const ts = store.ca && store.ca.ts;
  if (!ts) return '等待数据…';
  const s = Math.max(0, Math.floor(store.clock / 1000) - ts);
  if (s > 60) return '数据已过期';
  if (s < 90) return '更新于 ' + s + ' 秒前';
  return '更新于 ' + Math.floor(s / 60) + ' 分钟前';
}
function caLteText() {
  if (!caAllCells().length) return '—';
  const v = store.ca && store.ca.ca && store.ca.ca.valid ? store.ca.ca.lte_active : null;
  if (v == null) return '—';
  return v ? '已激活' : '未激活';
}
function caBandsText() {
  if (!caAllCells().length) return '—';
  const b = [...new Set(caDlCells().filter(c => c.rat === 'nr').map(c => c.band))];
  return (b && b.length) ? b.map(x => 'n' + x).join(' / ') : '—';
}

function gridPage(page) {
  return {
    data() { return { gs: null }; },
    computed: {
      editing: {
        get() { return store.editing; },
        set(v) { store.editing = !!v; },
      },
      resetTick() { return store.resetTick; },
    },
    watch: {
      editing(v) {
        if (this.gs) this.gs.setStatic(!v);
        if (!v) this.saveLayout();
      },
      resetTick() { this.resetLayout(); },
    },
    mounted() { this.$nextTick(() => this.initGrid()); },
    beforeUnmount() { if (this.gs) { this.gs.destroy(false); this.gs = null; } },
    methods: {
      initGrid() {
        const el = this.$refs.grid;
        if (!el || !window.GridStack || this.gs || !el.querySelector('.grid-stack-item')) return;
        this.gs = window.GridStack.init({
          column: 12, cellHeight: 96, margin: 12, float: false,
          handle: '.drag-handle', columnOpts: { breakpointForWindow: true, breakpoints: [{ w: 600, c: 1 }, { w: 1000, c: 6 }] },
        }, el);
        try {
          const saved = localStorage.getItem('a6.layout.' + page);
          if (saved) {
            const layout=JSON.parse(saved);
            if(page==='overview' && !localStorage.getItem('a6.overview.equal-columns.v1')) {
              const carrier=layout.find(i=>i.id==='carrier'),device=layout.find(i=>i.id==='device');
              if(carrier && device){const y=Math.min(carrier.y,device.y);Object.assign(carrier,{x:0,y,w:6});Object.assign(device,{x:6,y,w:6});}
              localStorage.setItem('a6.layout.'+page,JSON.stringify(layout));
              localStorage.setItem('a6.overview.equal-columns.v1','1');
            }
            this.gs.load(layout, false);
          }
        } catch (e) {}
        this.gs.setStatic(!store.editing);
        this.gs.on('change', () => this.saveLayout());
      },
      saveLayout() {
        if (!this.gs || this.gs.getColumn() !== 12) return;
        try {
          const items = this.gs.save(false).map(i => ({ x: i.x, y: i.y, w: i.w, h: i.h, id: i.id }));
          localStorage.setItem('a6.layout.' + page, JSON.stringify(items));
        } catch (e) {}
      },
      resetLayout() {
        if (!confirm('恢复该页面的默认布局？')) return;
        localStorage.removeItem('a6.layout.' + page);
        location.reload();
      },
    },
  };
}

/* ---------- data loaders ---------- */
async function readLeases() {
  const out = {};
  try {
    const r = await ubus('air6.cpe', 'get_leases');
    const txt = typeof r === 'string' ? r : (r && r.data ? r.data : '');
    txt.split('\n').forEach(line => {
      const p = line.trim().split(/\s+/);
      if (p.length >= 4) out[p[1].toLowerCase()] = { ip: p[2], host: p[3] === '*' ? '' : p[3] };
    });
  } catch (e) {}
  return out;
}

async function refreshClients() {
  store.clientsLoading = true;
  try {
    const devs = ((await ubus('iwinfo', 'devices')).devices) || [];
    const leases = await readLeases();
    const rows = [];
    for (const dev of devs) {
      if (/^apcli/.test(dev)) continue;
      if (!/^ra/.test(dev)) continue;
      let r = {};
      try { r = await ubus('iwinfo', 'assoclist', { device: dev }); } catch (e) { continue; }
      for (const st of (r.results || [])) {
        const mac = String(st.mac || '').toLowerCase();
        const l = leases[mac] || {};
        const sig = st.signal != null ? st.signal : st.signal_avg;
        const rate = (st.rx && st.rx.rate) || (st.tx && st.tx.rate) || (st.rate && (st.rate.rx || st.rate.tx));
        rows.push({
          mac: mac, host: l.host || st.hostname || '未知设备', ip: l.ip || '—',
          dev: dev, band: bandOfDev(dev),
          signal: sig != null ? sig : null,
          rate: rate ? Math.round(Number(rate) / 1000) : null,
        });
      }
    }
    try {
      const n=await ubus('air6.cpe','get_neighbors');
      for(const line of (n.output || '').split('\n')) {
        const m=/^(\S+)\s+.*?lladdr\s+([0-9a-f:]{17})\s+(REACHABLE|STALE|DELAY|PROBE|PERMANENT)/i.exec(line);
        if(!m) continue;
        const mac=m[2].toLowerCase(); if(rows.some(r=>r.mac===mac)) continue;
        const lease=leases[mac] || {};
        rows.push({mac,host:lease.host || '有线 / LAN 设备',ip:m[1],dev:'br-lan',band:'LAN',signal:null,rate:null,neighbor:true,state:m[3]==='REACHABLE' ? '最近可达' : '邻居缓存'});
      }
    } catch(e) {}
    store.clients = rows;
  } catch (e) { if (isAuthError(e)) logout(); }
  finally { store.clientsLoading = false; }
}

async function loadWifi() {
  store.wifiLoading = true; store.wifiError = '';
  try {
    store.wifi = await ubus('air6.cpe', 'get_wifi');
  } catch (e) {
    store.wifiError = '无线配置读取失败，请重试';
    toast(isAuthError(e) ? '会话已失效' : ('无线数据读取失败：' + e.message));
    if (isAuthError(e)) logout();
  } finally { store.wifiLoading = false; }
}

async function applyWifi(radio, iface) {
  if (store.wifiApplying) return;
  if (!confirm('应用后无线连接会短暂断开，确定保存此频段设置？')) return;
  store.wifiApplying = true;
  try {
    const r = await ubus('air6.cpe', 'set_wifi', {
      radio: radio.section, iface: iface.section,
      ssid: iface.ssid, key: iface.key, encryption: iface.encryption,
      channel: Number(radio.channel) || 0, htmode: radio.htmode,
      disabled: radio.disabled ? 1 : 0, iface_disabled: iface.disabled ? 1 : 0,
    });
    if (!r || !r.ok) throw new Error((r && r.error) || '设备未确认应用');
    toast('已应用，无线正在重启（客户端会短暂断开）');
    await new Promise(resolve => setTimeout(resolve, 4000));
    await loadWifi();
  } catch (e) {
    toast('应用失败：' + e.message);
    loadWifi();
  } finally { store.wifiApplying = false; }
}

async function sendAt() {
  const cmd = (store.atCmd || '').trim();
  if (!cmd || store.atBusy) return;
  store.atBusy = true;
  try {
    const r = await ubus('luci.air6modem', 'run_at', { command: cmd });
    store.atOut = (r && r.output) ? r.output : '(无输出)';
  } catch (e) { store.atOut = '错误：' + e.message; }
  finally { store.atBusy = false; }
}

async function readSim() {
  store.simBusy = true;
  try {
    const r = await ubus('luci.air6modem', 'run_at', { command: 'AT+QUIMSLOT?' });
    const m = /\+QUIMSLOT:\s*(\d+)/.exec((r && r.output) || '');
    store.simSlot = m ? m[1] : '';
    toast(m ? ('当前 SIM 槽位：' + m[1]) : '未读到 SIM 槽位');
  } catch (e) { store.simSlot = ''; toast('读取槽位失败：' + e.message); }
  finally { store.simBusy = false; }
}

async function readBand() {
  store.bandBusy = true;
  try {
    const r = await ubus('luci.air6modem', 'run_at', { command: 'AT+AIR6BAND?' });
    store.bandOut = (r && r.output) || '';
    toast(store.bandOut ? '已读取当前频段掩码' : '未读到频段掩码');
  } catch (e) { store.bandOut = '错误：' + e.message; toast('读取频段失败：' + e.message); }
  finally { store.bandBusy = false; }
}

function requireSuccess(r) {
  if (!r || r.error || (r.code != null && r.code !== 0) || r.success === false || r.result === false || r.ok === false || /(?:^|[\r\n])(?:ERROR|\+(?:CME|CMS) ERROR)(?::|[\r\n]|$)/.test(r.output || '')) {
    throw new Error((r && (r.error || r.output)) || '设备未确认操作');
  }
  return r;
}

async function setRat(mode) {
  if (store.ratBusy) return false;
  store.ratBusy = true;
  try {
    requireSuccess(await ubus('luci.air6modem', 'set_rat', { mode: mode }));
    toast('网络模式已切换');
    setTimeout(refreshStatus, 2500);
    return true;
  } catch (e) { toast('切换失败：' + e.message); return false; }
  finally { store.ratBusy = false; }
}

async function modemAction(method) {
  if (store.actionBusy) return;
  store.actionBusy = true;
  try {
    requireSuccess(await ubus('luci.air6modem', method));
    toast(method === 'ifup' ? '正在拨号…' : '已断开');
    setTimeout(refreshStatus, 2500);
  } catch (e) { toast('操作失败：' + e.message); }
  finally { store.actionBusy = false; }
}

async function setLedsOff(off) {
  try {
    await ubus('uci', 'set', { config: 'system', section: '@system[0]', values: { leds_off: off ? '1' : '0' } });
    await ubus('uci', 'commit', { config: 'system' });
    toast(off ? '面板灯已关闭' : '面板灯已开启');
    return true;
  } catch (e) { toast('操作失败：' + e.message); return false; }
}

async function loadLeds() {
  try {
    const r = await ubus('uci', 'get', { config: 'system', section: '@system[0]', option: 'leds_off' });
    return String(r && r.value) === '1';
  } catch (e) { return false; }
}

async function doReboot() {
  if (!window.confirm('确定重启路由器？网络会中断约 1 分钟。')) return;
  try { await ubus('system', 'reboot'); toast('设备正在重启…'); }
  catch (e) { toast('重启失败：' + e.message); }
}

/* ---------- shared widgets ---------- */
const ICONS = {
  layout: 'M4 4h6v6H4zM14 4h6v6h-6zM4 14h6v6H4zM14 17h6M17 14v6',
  reset: 'M4 12a8 8 0 1 0 3-6M4 4v5h5',
  logout: 'M10 4H5v16h5M14 8l4 4-4 4M9 12h9',
  updown: 'M8 4v13m0 0l-3-3m3 3l3-3M16 20V7m0 0l-3 3m3-3l3 3',
  shield: 'M12 3l7 3v5c0 4-3 7-7 9-4-2-7-5-7-9V6z',
  services: 'M4 6h16M4 12h16M4 18h16',
  chip: 'M9 3v2m6-2v2M9 19v2m6-2v2M3 9h2m-2 6h2m14-6h2m-2 6h2M7 5h10v14H7zM10 9h4v6h-4z',
  gauge: 'M4 13a8 8 0 1 1 16 0M12 13l4-4',
  wave: 'M2 12c2-4 4-4 6 0s4 4 6 0 4-4 6 0',
  sms: 'M4 5h16v11H9l-5 4z',
  grid: 'M3 3h7v7H3zM14 3h7v7h-7zM3 14h7v7H3zM14 14h7v7h-7z',
  signal: 'M2 20h3v-4H2zm5 0h3V9H7zm5 0h3V5h-3zm5 0h3V2h-3z',
  wifi: 'M2 8.5C8 3.5 16 3.5 22 8.5M5 12c4.5-3.6 9.5-3.6 14 0M8 15.5c2.6-2 5.4-2 8 0',
  devices: 'M3 4h13v10H3zM6 18h7M16 10h5v10h-5zM8 20h5',
  gear: 'M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7zM19 12l1.8-1-1.8-3.1-2 .8a7 7 0 0 0-1.7-1L15 5.5h-3.6l-.3 2.2a7 7 0 0 0-1.7 1l-2-.8L5.6 11 7.4 12l-.2 1 .2 1-1.8 1 1.8 3.1 2-.8a7 7 0 0 0 1.7 1l.3 2.2H15l.3-2.2a7 7 0 0 0 1.7-1l2 .8 1.8-3.1-1.8-1z',
};

const Icon = {
  props: ['name'],
  template: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"><path :d="path"/></svg>',
  computed: { path() { return ICONS[this.name] || ''; } },
};

const TrafficChart = {
  props: ['samples', 'seq'],
  template: '<canvas ref="cv" class="chart"></canvas>',
  mounted() {
    this.onResize = () => this.resize();
    window.addEventListener('resize', this.onResize);
    this.resizeObserver = new ResizeObserver(() => this.resize());
    if(this.$refs.cv) this.resizeObserver.observe(this.$refs.cv);
    nextTick(() => this.resize());
  },
  unmounted() { window.removeEventListener('resize', this.onResize); if(this.resizeObserver) this.resizeObserver.disconnect(); },
  methods: {
    resize() {
      const c = this.$refs.cv;
      if (!c) return;
      const rect = c.getBoundingClientRect();
      const dpr = window.devicePixelRatio || 1;
      c.width = Math.max(1, Math.round(rect.width * dpr));
      c.height = Math.max(1, Math.round(rect.height * dpr));
      c._dpr = dpr; c._w = rect.width; c._h = rect.height;
      this.draw();
    },
    draw() {
      const c = this.$refs.cv;
      if (!c || !c._w) return;
      const dpr = c._dpr || 1;
      const ctx = c.getContext('2d');
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      const w = c._w, h = c._h;
      ctx.clearRect(0, 0, w, h);
      const s = this.samples || [];
      const max = Math.max(1, ...s.map(v => Math.max(v.rx, v.tx)));
      const pad = 6;
      ctx.strokeStyle = 'rgba(120,130,155,.12)';
      ctx.lineWidth = 1;
      for (let i = 0; i <= 3; i++) {
        const y = pad + (h - pad * 2) * i / 3;
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      }
      if (s.length < 2) return;
      const stepX = w / Math.max(1, s.length - 1);
      const yOf = v => pad + (h - pad * 2) * (1 - Math.min(1, v / max));
      const line = (key, color, fill) => {
        ctx.beginPath();
        s.forEach((v, i) => { const x = i * stepX, y = yOf(v[key]); i ? ctx.lineTo(x, y) : ctx.moveTo(x, y); });
        if (fill) {
          const g = ctx.createLinearGradient(0, 0, 0, h);
          g.addColorStop(0, fill); g.addColorStop(1, 'rgba(255,255,255,0)');
          ctx.save();
          ctx.lineTo(w, h); ctx.lineTo(0, h); ctx.closePath();
          ctx.fillStyle = g; ctx.fill();
          ctx.restore();
          ctx.beginPath();
          s.forEach((v, i) => { const x = i * stepX, y = yOf(v[key]); i ? ctx.lineTo(x, y) : ctx.moveTo(x, y); });
        }
        ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.lineJoin = 'round'; ctx.lineCap = 'round'; ctx.stroke();
      };
      line('rx', '#ef3e6d', 'rgba(239,62,109,.16)');
      line('tx', '#4c7cf3', null);
    },
  },
  watch: { seq() { this.draw(); } },
};

/* ---------- login ---------- */
const LoginView = {
  template: `
  <div class="login">
    <div class="login-card fade-up">
      <div class="logo">A6</div>
      <h1>Air 6 连接管理</h1>
      <p>SoftBank Air 6 · 5G CPE</p>
      <form @submit.prevent="submit">
        <div class="field">
          <label>管理员密码</label>
          <input type="password" v-model="pass" autocomplete="current-password" placeholder="请输入路由器管理员密码" autofocus>
        </div>
        <button class="btn primary" style="width:100%" :disabled="loading">{{ loading ? '登录中…' : '登录' }}</button>
        <div class="err">{{ err }}</div>
      </form>
      <div class="hint">使用 root 管理员密码登录；高级设置请登录后进入 LuCI。</div>
    </div>
  </div>`,
  data() { return { pass: '', err: '', loading: false }; },
  methods: {
    async submit() {
      this.err = ''; this.loading = true;
      await doLogin(this.pass);
      this.err = store.loginError;
      this.loading = false;
    },
  },
};

/* ---------- overview ---------- */
const OverviewView = {
  mixins: [gridPage('overview')],
  components: { TrafficChart, Icon },
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div v-if="st && st.initializing" class="banner">模组正在启动，部分数据会在 20–40 秒后出现。</div>
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="hero" gs-w="4" gs-h="4">
        <div class="grid-stack-item-content card widget hero-card">
          <div class="widget-head"><h3>蜂窝连接</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body hero-body">
            <div class="hero-left">
              <div class="hero-status">
                <span class="pill" :class="online ? 'ok' : ''"><i class="pill-dot"></i>{{ online ? '已连接' : '未连接' }}</span>
                <span class="hero-op-inline">{{ op }}</span>
              </div>
              <div class="hero-rat"><b>{{ rat }}</b><span class="chip rat-chip">{{ ratLong }}</span><span class="chip band-chip">{{ band }}</span></div>
              <div class="hero-op">{{ st && st.registration ? st.registration : '未注册' }}</div>
              <div class="hero-divider"></div>
              <dl class="hero-kv">
                <div><dt>网络地址</dt><dd>{{ ipaddr }}</dd></div>
                <div><dt>主载波频点</dt><dd>{{ st && st.arfcn ? st.arfcn : '—' }}</dd></div>
                <div><dt>当前 APN</dt><dd>{{ st && st.apn ? st.apn : '—' }}</dd></div>
                <div><dt>模组温度</dt><dd>{{ st && st.temperature ? st.temperature + ' °C' : '—' }}</dd></div>
              </dl>
            </div>
            <div class="hero-side">
              <span class="spark-title">信号趋势</span>
              <svg class="spark-big" viewBox="0 0 160 56" preserveAspectRatio="none"><polyline :points="sparkPointsBig"/></svg>
              <span class="spark-label">RSRP · 最近 40 次采样</span>
            </div>
          </div>
        </div>
      </div>

      <div class="grid-stack-item" gs-id="traffic" gs-w="8" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>实时流量</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body chart-body">
            <div class="rates">
              <div><small>↓ 下载</small><b>{{ rx.toFixed(2) }}<span>Mbps</span></b></div>
              <div><small>↑ 上传</small><b>{{ tx.toFixed(2) }}<span>Mbps</span></b></div>
            </div>
            <traffic-chart :samples="samples" :seq="seq"></traffic-chart>
            <div class="legend"><span class="rx"><i></i>下载</span><span class="tx"><i></i>上传</span><span style="margin-left:auto">纵轴自适应</span></div>
          </div>
        </div>
      </div>

      <div class="grid-stack-item" gs-id="rsrp" gs-w="3" gs-h="2">
        <div class="grid-stack-item-content card widget metric">
          <div class="widget-head"><span class="metric-title"><icon name="signal"></icon><small>RSRP</small></span><span class="drag-handle">⠿</span></div>
          <div class="widget-body"><b>{{ sig.rsrp != null ? sig.rsrp : '—' }}<span>dBm</span></b><span class="hint">参考信号接收功率</span></div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="rsrq" gs-w="3" gs-h="2">
        <div class="grid-stack-item-content card widget metric">
          <div class="widget-head"><span class="metric-title"><icon name="gauge"></icon><small>RSRQ</small></span><span class="drag-handle">⠿</span></div>
          <div class="widget-body"><b>{{ sig.rsrq != null ? sig.rsrq : '—' }}<span>dB</span></b><span class="hint">参考信号接收质量</span></div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="sinr" gs-w="3" gs-h="2">
        <div class="grid-stack-item-content card widget metric">
          <div class="widget-head"><span class="metric-title"><icon name="wave"></icon><small>SINR</small></span><span class="drag-handle">⠿</span></div>
          <div class="widget-body"><b>{{ sig.sinr != null ? sig.sinr : '—' }}<span>dB</span></b><span class="hint">信干噪比</span></div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="clients" gs-w="3" gs-h="2">
        <div class="grid-stack-item-content card widget metric">
          <div class="widget-head"><span class="metric-title"><icon name="devices"></icon><small>无线客户端</small></span><span class="drag-handle">⠿</span></div>
          <div class="widget-body"><b>{{ clientCount }}<span>台</span></b><span class="hint">无线客户端</span></div>
        </div>
      </div>

      <div class="grid-stack-item" gs-id="carrier" gs-x="0" gs-w="6" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>载波聚合</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="flex" style="align-items:center;margin-bottom:8px">
              <span class="chip" :class="caActive ? 'rat-chip' : ''">{{ caSummary }}</span>
              <span class="muted" style="font-size:12px;margin-left:auto">{{ caErr || caNote }}</span>
            </div>
            <table>
              <thead><tr><th>载波</th><th>方向</th><th>频段</th><th>ARFCN</th><th>PCI</th><th>带宽</th><th>状态</th></tr></thead>
              <tbody>
                <tr v-for="(c,i) in caAll" :key="i">
                  <td>{{ c.rat === 'nr' ? 'NR' : 'LTE' }} {{ c.role || (c.idx === 0 ? 'PCC' : 'SCC' + c.idx) }}</td>
                  <td>{{ c.dir === 'dl' ? '下行' : '上行' }}</td>
                  <td><span class="chip band-chip">{{ (c.rat === 'nr' ? 'n' : 'B') + c.band }}</span></td>
                  <td class="mono">{{ c.arfcn }}</td><td class="mono">{{ c.pci }}</td>
                  <td>{{ c.bw }}</td><td>{{ c.state === 2 ? '激活' : c.state === 0 || c.state === 1 ? '未激活' : '未知' }}</td>
                </tr>
                <tr v-if="!caAll.length"><td colspan="7" class="empty">暂无载波数据</td></tr>
              </tbody>
            </table>
            <p class="muted" style="font-size:12px;margin:10px 0 0">LTE CA 激活：{{ caLte }} · NR 载波频段：{{ caBands }}。LTE 与 NR 分别判断聚合；双连接单独显示。</p>
          </div>
        </div>
      </div>

      <div class="grid-stack-item" gs-id="device" gs-x="6" gs-w="6" gs-h="5" gs-min-h="5">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>设备状态</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="stat-grid device-details">
              <div class="stat wide"><span>型号</span><b>{{ board.model || '—' }}</b></div>
              <div class="stat"><span>主机名</span><b>{{ board.hostname || '—' }}</b></div>
              <div class="stat"><span>内核</span><b>{{ board.kernel || '—' }}</b></div>
              <div class="stat wide"><span>固件版本</span><b :title="release">{{ release }}</b></div>
              <div class="stat"><span>运行时长</span><b>{{ uptime }}</b></div>
              <div class="stat"><span>模组温度</span><b>{{ temp }}</b></div>
              <div class="stat"><span>内存占用</span><b>{{ memUsed }}<i>/ {{ memTotal }}</i></b><div class="mini-bar"><i :style="{width: memPct + '%'}"></i></div></div>
              <div class="stat"><span>流量累计</span><b>{{ totalRx }} ↓</b><b>{{ totalTx }} ↑</b></div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  computed: {
    board() { return store.board || {}; },
    st() { return store.status || null; },
    sig() { return signalOf(store.status); },
    online() { return !!(store.status && store.status.network && store.status.network.up); },
    rat() { return ratShort(store.status && store.status.rat); },
    ratLong() { return this.rat === '5G' ? 'NR5G' : (this.rat === '4G' ? 'LTE' : this.rat); },
    band() { const b = store.status && store.status.band; return b ? String(b).toUpperCase() : ''; },
    caAll() { return caAllCells(); },
    caErr() { return store.caErr; },
    caLte() { return caLteText(); },
    caBands() { return caBandsText(); },
    caActive() { return caIsActive(); },
    caSummary() { return caSummaryText(); },
    caNote() { return caAgeText(); },
    op() { return opName(store.status && store.status.operator); },
    ipaddr() {
      const n = store.status && store.status.network;
      const a = n && n['ipv4-address'];
      return (a && a[0] && a[0].address) ? a[0].address : '—';
    },
    pct() { return signalPct(this.sig.rsrp); },
    sparkPointsBig() {
      const h = store.sigHist;
      if (h.length < 2) return '0,48 160,48';
      return h.map((v, i) => {
        const x = (i / (h.length - 1)) * 160;
        const y = 50 - Math.max(0, Math.min(1, (v + 140) / 100)) * 44;
        return x.toFixed(1) + ',' + y.toFixed(1);
      }).join(' ');
    },
    C() { return 2 * Math.PI * 50; },
    clientCount() { return store.clients.filter(c=>!c.neighbor).length; },
    rx() { return store.rxRate || 0; },
    tx() { return store.txRate || 0; },
    samples() { return store.samples; },
    seq() { return store.seq; },
    temp() { const t = store.status && store.status.temperature; return t ? t + ' °C' : '—'; },
    release() { const r = store.board && store.board.release; return r ? (r.description || (r.distribution + ' ' + r.version)) : '—'; },
    uptime() { return fmtUptime(store.info && store.info.uptime); },
    memUsed() { const m = store.info && store.info.memory; return m ? fmtBytes(m.total - m.available) : '—'; },
    memTotal() { const m = store.info && store.info.memory; return m ? fmtBytes(m.total) : '—'; },
    memPct() { const m = store.info && store.info.memory; return m ? Math.round((m.total - m.available) / m.total * 100) : 0; },
    totalRx() { const d = pickWan(store.accel && store.accel.interfaces); return d ? fmtBytes(d.rx_bytes) : '—'; },
    totalTx() { const d = pickWan(store.accel && store.accel.interfaces); return d ? fmtBytes(d.tx_bytes) : '—'; },
  },
  mounted() { refreshClients(); this.timer = setInterval(() => { if (!document.hidden) refreshClients(); }, 15000); },
  beforeUnmount() { clearInterval(this.timer); },
};

/* ---------- cellular ---------- */
const CellularView = {
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <div class="cellular-grid">
      <div class="cellular-panel" gs-id="sim" gs-x="0" gs-y="0" gs-w="6" gs-h="6">
        <div class="card widget">
          <div class="widget-head"><h3>SIM 与注册</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <dl class="kv">
              <div><dt>运营商</dt><dd>{{ op }}</dd></div>
              <div><dt>注册状态</dt><dd>{{ st && st.registration || '—' }}</dd></div>
              <div><dt>网络制式</dt><dd>{{ st && st.rat || '—' }}</dd></div>
              <div><dt>当前频段</dt><dd>{{ st && st.band || '—' }}</dd></div>
              <div><dt>SIM 槽</dt><dd>{{ simSlot || '—' }}</dd></div>
              <div><dt>ICCID</dt><dd class="mono" :title="st && st.iccid">{{ st && st.iccid || '—' }}</dd></div>
              <div><dt>IMSI</dt><dd class="mono" :title="st && st.imsi">{{ st && st.imsi || '—' }}</dd></div>
              <div><dt>IMEI</dt><dd class="mono" :title="st && st.imei">{{ st && st.imei || '—' }}</dd></div>
            </dl>
            <div class="row-actions mt16">
              <button class="btn ghost" :disabled="simBusy" @click="reloadSim">{{ simBusy ? '读取中…' : '读取槽位' }}</button>
              <button class="btn ghost" @click="act('ifdown')">断开</button>
              <button class="btn primary" @click="act('ifup')">重新拨号</button>
            </div>
          </div>
        </div>
      </div>

      <div class="cellular-panel" gs-id="mode" gs-x="6" gs-y="0" gs-w="6" gs-h="6">
        <div class="card widget">
          <div class="widget-head"><h3>网络模式</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="seg">
              <button :disabled="ratBusy" :class="{on: ratMode===21}" @click="setMode(21)">自动</button>
              <button :disabled="ratBusy" :class="{on: ratMode===19}" @click="setMode(19)">4G + 5G</button>
              <button :disabled="ratBusy" :class="{on: ratMode===3}" @click="setMode(3)">仅 4G</button>
              <button :disabled="ratBusy" :class="{on: ratMode===15}" @click="setMode(15)">仅 5G</button>
            </div>
            <p class="muted" style="font-size:12px;margin:14px 0 0">按钮高亮表示本页最近成功提交的模式；当前制式见 SIM 与注册。切换后会重新注册，约 10–30 秒。</p>
            <div class="field" style="margin-top:18px"><label>APN</label><input v-model="apn" @input="apnTouched = true" placeholder="ctnet"></div>
            <div class="field"><label>PDP 类型</label><select v-model="iptype" :disabled="networkLoading"><option value="">保持当前配置</option><option value="1">IPv4</option><option value="2">IPv6</option><option value="3">IPv4 / IPv6</option></select></div>
            <div class="row-actions"><button class="btn ghost" :disabled="apnBusy || networkLoading" @click="saveApn">{{ apnBusy ? '保存中…' : '保存 APN' }}</button></div>

          </div>
        </div>
      </div>

      <div class="cellular-panel" gs-id="band-lock">
        <div class="card widget"><div class="widget-head"><h3>频段选择与锁定</h3></div><div class="widget-body">

            <div class="row-actions"><button class="btn ghost" :disabled="bandBusy" @click="loadBand">{{ bandBusy ? '读取中…' : '读取当前掩码' }}</button></div>
            <div class="band-lock-panel">
              <p class="muted">限制允许搜网的频段，不等于同时聚合。应用会短暂中断蜂窝网络，并清除已有小区锁定。</p>
              <div class="band-choice"><b>LTE</b><label v-for="b in lteOptions" :key="b"><input type="checkbox" v-model="lteSelected" :value="b" :disabled="bandApplying || bandReading">B{{ b }}</label></div>
              <div class="band-choice"><b>NR</b><label v-for="b in nrOptions" :key="b"><input type="checkbox" v-model="nrSelected" :value="b" :disabled="bandApplying || bandReading">n{{ b }}</label></div>
              <div class="row-actions"><button class="btn ghost" :disabled="bandApplying || bandReading" @click="readBandLock">读取当前选择</button><button class="btn primary" :disabled="bandApplying || bandReading || !bandReady" @click="applyBandLock('apply')">应用所选频段</button><button class="btn ghost" :disabled="bandApplying" @click="applyBandLock('reset')">恢复授权全集</button></div>
              <p class="muted">{{ bandMessage }}</p>
            </div>
            <pre class="at" v-if="bandOut">{{ bandOut }}</pre>
        </div></div>
      </div>
      <div class="cellular-panel" gs-id="license">
        <div class="card widget"><div class="widget-head"><h3>频段授权</h3><button class="btn ghost" :disabled="licenseBusy" @click="readLicense">{{ licenseBusy ? '读取中…' : '读取机器码与状态' }}</button></div><div class="widget-body">
          <p class="muted">授权绑定本机机器码。与 SIM PIN、运营商网络锁分别管理。</p>
          <div class="field"><label>本机机器码</label><input :value="machineId" readonly placeholder="点击读取机器码" class="mono"></div>
          <div class="row-actions"><button class="btn ghost" :disabled="!machineId" @click="copyMachine">复制机器码</button><span class="muted">{{ licenseState || '尚未读取授权状态' }}</span></div>
          <div class="field mt16"><label>解锁码（A1. 开头，共 89 个字符）</label><input v-model.trim="licenseToken" maxlength="110" autocomplete="off" spellcheck="false" placeholder="粘贴本机解锁码或完整 AT 命令"></div>
          <div class="row-actions"><button class="btn primary" :disabled="licenseBusy || bandApplying || !licenseToken" @click="applyLicense">验证并应用授权</button><button class="btn ghost" :disabled="licenseBusy || bandApplying" @click="clearLicense">清除授权</button></div>
          <p class="muted">{{ licenseMessage }}</p>
        </div></div>
      </div>
      <div class="cellular-panel" gs-id="at" gs-x="0" gs-y="6" gs-w="6" gs-h="4">
        <div class="card widget">
          <div class="widget-head"><h3>AT 控制台</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="at-row">
              <input v-model="atCmd" placeholder="AT+CSQ" @keyup.enter="send">
              <button class="btn primary" :disabled="atBusy" @click="send">{{ atBusy ? '发送中…' : '发送' }}</button>
            </div>
            <pre class="at">{{ atOut || '输出会显示在这里' }}</pre>
          </div>
        </div>
      </div>
      <div class="cellular-panel" gs-id="cells" gs-x="6" gs-y="6" gs-w="6" gs-h="4">
        <div class="card widget">
          <div class="widget-head"><h3>小区信息</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="flex" style="margin-bottom:10px">
              <button class="btn ghost" @click="loadCells">刷新</button>
              <span class="muted" style="font-size:12px">{{ cellsNote }}</span>
            </div>
            <p class="muted">{{ endcText }}</p>
            <h4>服务小区与邻区</h4>
            <div style="overflow-x:auto"><table><thead><tr><th>角色 / RAT</th><th>PCI</th><th>ARFCN</th><th>频段</th><th>带宽</th><th>RSRP</th><th>RSRQ</th><th>SINR</th></tr></thead>
              <tbody><tr v-for="(c,i) in apiCells" :key="i"><td>{{ c.role === 'serving' ? '服务' : '邻区' }} / {{ c.rat }}</td><td>{{ c.pci ?? '—' }}</td><td>{{ c.arfcn ?? '—' }}</td><td>{{ c.band ? (c.rat === 'NR' ? 'n' : 'B') + c.band : '未知' }}</td><td>{{ c.dl_bandwidth_mhz != null ? c.dl_bandwidth_mhz + ' MHz' : '未知' }}</td><td>{{ c.rsrp ?? '—' }} dBm</td><td>{{ c.rsrq ?? '—' }} dB</td><td>{{ c.sinr ?? '—' }} dB</td></tr><tr v-if="!apiCells.length"><td colspan="8">暂无有效测量</td></tr></tbody></table></div>
            <h4>主辅载波 · CA</h4>
            <div style="overflow-x:auto"><table><thead><tr><th>RAT / 角色</th><th>频段</th><th>PCI / ARFCN</th><th>下行 / 上行带宽</th><th>状态</th><th>MIMO 下 / 上</th><th>MCS 下 / 上</th></tr></thead><tbody><tr v-for="(c,i) in telemetry.carriers || []" :key="i"><td>{{ c.rat }} {{ c.role }}</td><td>{{ c.rat === 'NR' ? 'n' : 'B' }}{{ c.band }}</td><td>{{ c.pci }} / {{ c.arfcn }}</td><td>{{ c.dl_bandwidth_mhz ?? '—' }} / {{ c.ul_bandwidth_mhz ?? '—' }} MHz</td><td>{{ c.role === 'PCC' ? '主载波' : c.active ? '已激活' : '已配置 / 未激活' }}</td><td>{{ c.dl_mimo }} / {{ c.ul_mimo }}</td><td>{{ c.dl_mcs }} / {{ c.ul_mcs }}</td></tr><tr v-if="!(telemetry.carriers || []).length"><td colspan="7">暂无载波数据</td></tr></tbody></table></div>
            <h4>主辅小区测量</h4>
            <div style="overflow-x:auto"><table><thead><tr><th>RAT / 角色</th><th>RSRP / RSRQ / SNR 报告索引</th><th>RSSI</th><th>Rank / 路损</th><th>最佳波束</th><th>波束 RSRP / SSB SNR</th></tr></thead><tbody><tr v-for="(m,i) in telemetry.measurements || []" :key="i"><td>{{ m.rat }} {{ m.role }} {{ m.index }}</td><td>{{ m.rsrp_index }} / {{ m.rsrq_index }} / {{ m.snr_index }}</td><td>{{ m.rssi_dbm ?? '—' }} dBm</td><td>{{ m.rank_indicator || '—' }} / {{ m.path_loss_db }} dB</td><td>{{ m.best_beam_id ?? '—' }}</td><td>{{ m.best_beam_rsrp_dbm ?? '—' }} dBm / {{ m.ssb_snr_db ?? '—' }} dB</td></tr></tbody></table></div>
            <p class="muted">测量报告索引按 API 原值显示；邻区未报告的频段和带宽显示未知。EN-DC 可用不代表双连接已激活。</p>
          </div>
        </div>
      </div>

    </div>
  </div>`,
  data() { return { ratMode: null, cellsNote: '', telemetry: {}, apn: '', apnTouched: false, apnBusy: false, iptype: '', networkIface: '', networkLoading: true, lteOptions:[1,3,5,7,8,20,28,38,40,41,42,43], nrOptions:[1,3,5,7,8,20,28,38,40,41,77,78,79], lteSelected:[], nrSelected:[], bandReady:false, bandReading:false, bandApplying:false, bandMessage:'展开后读取实际频段', bandJobTimer:0, licenseBusy:false, machineId:'',licenseState:'',licenseToken:'',licenseMessage:'' }; },
  computed: {
    editing() { return false; },
    st() { return store.status || null; },
    op() { return opName(store.status && store.status.operator); },
    simSlot() { return store.simSlot; },
    ratBusy() { return store.ratBusy; },
    simBusy() { return store.simBusy; },
    bandBusy() { return store.bandBusy; },
    bandOut() { return store.bandOut; },
    atOut() { return store.atOut; },
    atBusy() { return store.atBusy; },
    atCmd: {
      get() { return store.atCmd; },
      set(v) { store.atCmd = v; },
    },
    caAll() { return caAllCells(); },
    caActive() { return caIsActive(); },
    caSummary() { return caSummaryText(); },
    caNote() { return caAgeText(); },
    caLte() { return caLteText(); },
    caBands() { return caBandsText(); },
    apiCells() {
      const seen = new Set();
      return (this.telemetry.cells || []).filter(c => {
        const key = [c.rat, c.pci, c.arfcn].join(':');
        if (seen.has(key)) return false;
        seen.add(key); return true;
      }).map(c => {
        const carrier = (this.telemetry.carriers || []).find(v => v.rat === c.rat && v.pci === c.pci && v.arfcn === c.arfcn);
        return carrier ? {...c, band: carrier.band, dl_bandwidth_mhz: carrier.dl_bandwidth_mhz} : c;
      });
    },
    endcText() {
      const e = this.telemetry.endc;
      if (!e || !e.valid) return 'EN-DC：未知';
      return 'EN-DC 支持：' + (e.available ? '是' : '否') + ' · NR 双连接限制：' + (e.restricted ? '有' : '无') + ' · NSA 注册：' + (e.nsa_registered ? '是' : '否');
    },

  },
  mounted() {
    this.readBandLock();
    this.loadCells();
    this.apn = (store.status && store.status.apn) || '';
    ubus('luci.air6modem','get_network').then(n=>{ this.networkIface=n.interface || ''; if(['1','2','3'].includes(String(n.iptype))) this.iptype=String(n.iptype); }).catch(e=>toast('PDP 配置读取失败：'+e.message)).finally(()=>{this.networkLoading=false;});
    this.$watch(() => store.status, () => this.loadCells());
    this.$watch(() => store.status && store.status.apn, v => { if (!this.apnTouched) this.apn = v || ''; });
  },
  beforeUnmount() { clearTimeout(this.bandJobTimer); },
  methods: {
    async readLicense() {
      if(this.licenseBusy) return;this.licenseBusy=true;
      try {
        const id= requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BANDID?'}));
        const m=/\+AIR6BANDID:\s*([0-9a-fA-F]{64})/.exec(id.output || '');
        if(!m) throw new Error('模组未返回有效机器码');this.machineId=m[1];
        const state=requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BAND?'}));this.licenseState=(state.output || '').split('\n').find(l=>l.includes('+AIR6BAND:')) || '未读到授权状态';
      }catch(e){this.licenseMessage='读取失败：'+e.message;}
      finally{this.licenseBusy=false;}
    },
    async copyMachine() {
      try { await navigator.clipboard.writeText(this.machineId);toast('机器码已复制'); }
      catch(e) { toast('浏览器未允许复制，请选中机器码后手动复制'); }
    },
    async clearLicense() {
      if(this.licenseBusy || this.bandApplying) return;
      if(!confirm('确认清除本机频段授权？清除后恢复原厂频段范围，蜂窝连接可能短暂中断；重新授权需要本机解锁码。')) return;
      this.licenseBusy=true;this.licenseMessage='正在清除并读回授权状态…';
      try {
        requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BAND=0'}));
        const r=requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BAND?'}));
        this.licenseState=(r.output || '').split('\n').find(l=>l.includes('+AIR6BAND:')) || '';
        if(!/TARGET=FACTORY(?:,|\s|$)/.test(this.licenseState) || /LICENSE=VALID(?:,|\s|$)/.test(this.licenseState)) throw new Error('读回未确认授权已清除，请重新读取状态');
        this.licenseToken='';this.licenseMessage='授权已清除，已读回确认恢复原厂频段范围';this.readBandLock();toast('频段授权已清除');
      } catch(e) {this.licenseMessage='清除失败：'+e.message;}
      finally {this.licenseBusy=false;}
    },
    async applyLicense() {
      if(this.licenseBusy || this.bandApplying) return;
      const token=this.licenseToken.replace(/^AT\+AIR6BAND="(A1\.[A-Za-z0-9_-]{86})"$/,'$1');
      if(!/^A1\.[A-Za-z0-9_-]{86}$/.test(token)){this.licenseMessage='解锁码格式不正确，应为 A1. 开头，共 89 个字符';return;}
      this.licenseBusy=true;this.licenseMessage='正在提交并读回授权…';
      try {
        requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BAND="'+token+'"'}));
        const r=requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+AIR6BAND?'}));
        this.licenseState=(r.output || '').split('\n').find(l=>l.includes('+AIR6BAND:')) || '';
        if(!/LICENSE=VALID/.test(this.licenseState) || !/TARGET=EU25/.test(this.licenseState))throw new Error('授权读回未确认有效，请核对机器码和解锁码');
        this.licenseToken='';this.licenseMessage='授权有效，已读回确认 EU25';this.readBandLock();toast('频段授权已生效');
      }catch(e){this.licenseMessage='授权失败：'+e.message;}
      finally{this.licenseBusy=false;}
    },
    openBandPanel(event) { if(event.target.open && !this.bandReady && !this.bandApplying) this.readBandLock(); },
    async readBandLock() {
      if(this.bandReading) return; this.bandReading=true; this.bandMessage='正在读取实际频段…';
      try {
        const r=await ubus('air6.cpe','get_band_lock'); requireSuccess(r);
        const selected=(hex,bands)=>bands.filter(b=>{const chunk=parseInt(hex.slice(Math.floor((b-1)/32)*8,Math.floor((b-1)/32)*8+8),16);return Number.isFinite(chunk) && ((chunk >>> ((b-1)%32)) & 1);});
        this.lteSelected=selected(r.lte_mask,this.lteOptions);this.nrSelected=selected(r.nr_mask,this.nrOptions);this.bandReady=true;this.bandMessage='已读取模组当前频段';
      } catch(e) {this.bandReady=false;this.bandMessage='读取失败：'+e.message;}
      finally {this.bandReading=false;}
    },
    async applyBandLock(action) {
      if(this.bandApplying || this.licenseBusy) return;
      if(action==='apply' && (!this.lteSelected.length || !this.nrSelected.length)) {toast('LTE 和 NR 各需至少选择一个频段');return;}
      if(!confirm('确认'+(action==='reset'?'恢复授权全集':'应用所选频段')+'？蜂窝连接会短暂中断，已有小区锁定将清除。')) return;
      this.bandApplying=true;this.bandMessage='正在提交…';
      try {requireSuccess(await ubus('air6.cpe','set_band_lock',{action,lte:this.lteSelected.join(','),nr:this.nrSelected.join(',')}));this.pollBandJob(0);}
      catch(e) {this.bandApplying=false;this.bandMessage=e.message;}
    },
    async pollBandJob(count) {
      try {const r=await ubus('air6.cpe','get_band_job');
        if(r.done) {this.bandApplying=false;this.bandMessage=r.code===0?'频段已应用，模组读回及数据恢复验证通过':'操作未通过验证：'+(r.output || r.code);if(r.code===0){await this.readBandLock();refreshStatus();refreshCa();}return;}
        this.bandMessage='正在应用、读回和恢复数据连接…（'+(count*3)+' 秒）';
      } catch(e) {this.bandMessage='等待设备响应：'+e.message;}
      if(count>=100){this.bandMessage='操作耗时较长，请稍后读取实际频段确认';this.bandApplying=false;return;}
      this.bandJobTimer=setTimeout(()=>this.pollBandJob(count+1),3000);
    },
    reloadSim() { readSim(); },
    async saveApn() {
      this.apn = this.apn.trim();
      if (!/^[A-Za-z0-9][A-Za-z0-9.-]{0,99}$/.test(this.apn)) { toast('请输入有效的 APN（字母、数字、点或连字符）'); return; }
      this.apnBusy = true;
      try {
        const iface = this.networkIface || 'modem5g';
        await ubus('uci', 'set', { config: 'network', section: iface, values: { apn: this.apn, auto_conf: '0', ...(this.iptype ? {iptype:this.iptype} : {}) } });
        await ubus('uci', 'commit', { config: 'network' });
        requireSuccess(await ubus('luci.air6modem', 'ifup'));
        toast('APN 已保存，正在重新拨号');
        setTimeout(refreshStatus, 4000);
      } catch (e) { toast('保存失败：' + e.message); }
      finally { this.apnBusy = false; }
    },
    async setMode(m) { if (await setRat(m)) this.ratMode = m; },
    act(m) { modemAction(m); },
    loadBand() { readBand(); },
    async loadCells() {
      this.cellsNote = '读取中…';
      try {
        const r = await ubus('luci.air6modem', 'get_cells');
        if (r && r.initializing) throw new Error('模组正在初始化，请稍后重试');
        if (r.stale) throw new Error('API 数据已过期');
        this.telemetry = r;
        this.cellsNote = '模组 API · 缓存 ' + r.cache_age + ' 秒';
      } catch (e) { this.cellsNote = '读取失败：' + e.message; }
    },
    send() { sendAt(); },
  },
};

/* ---------- wireless ---------- */
const WirelessView = {
  mixins: [gridPage('wireless-compact-v2')],
  data() { return { visiblePasswords: {} }; },
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <div v-if="loading" class="banner">正在读取无线配置…</div>
    <div v-if="wifiError" class="banner">{{ wifiError }} <button class="btn ghost" @click="reload">重试</button></div>
    <div v-if="!loading && !wifiError && !radios.length" class="banner">未检测到可用无线射频</div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" v-for="radio in radios" :key="radio.section" :gs-id="'wifi-' + radio.section" gs-w="4" gs-h="6">
        <div class="grid-stack-item-content card widget radio-card">
          <div class="widget-head">
            <div class="band"><b>{{ bandName(radio.band) }}</b><small>{{ radio.ifname }} · {{ radio.band }}</small></div>
            <span class="drag-handle">⠿</span>
          </div>
          <div class="widget-body" v-if="radio.ifaces.length">
            <div v-if="!supported(radio)" class="banner">当前驱动未报告该频段的可用射频；配置已保留，待驱动支持后可应用。</div>
            <fieldset :disabled="applying || !supported(radio)" class="wifi-fields">
            <div class="sub" v-if="radio.mt">信道 {{ radio.mt.channel }} · {{ radio.mt.bandwidth_mhz }} MHz · {{ radio.mt.htmode }}</div>
            <div v-else class="sub">{{ radio.htmode }}</div>
            <div class="field"><label>SSID</label><input v-model="radio.ifaces[0].ssid" maxlength="32"></div>
            <div class="field">
              <label>加密方式</label>
              <select v-model="radio.ifaces[0].encryption">
                <option value="sae-mixed">WPA2/WPA3 混合</option>
                <option value="sae">WPA3 (SAE)</option>
                <option value="psk2">WPA2 (PSK)</option>
                <option value="owe">OWE（增强型开放网络）</option>
                <option value="none">开放网络（无密码）</option>
              </select>
            </div>
            <div class="field" v-if="!['none','owe'].includes(radio.ifaces[0].encryption)"><label>密码</label><div class="wifi-password"><input v-model="radio.ifaces[0].key" :type="visiblePasswords[radio.section] ? 'text' : 'password'" autocomplete="new-password"><button type="button" class="password-toggle" :aria-label="visiblePasswords[radio.section] ? '隐藏密码' : '显示密码'" :aria-pressed="!!visiblePasswords[radio.section]" @click="visiblePasswords[radio.section] = !visiblePasswords[radio.section]">{{ visiblePasswords[radio.section] ? '隐藏' : '显示' }}</button></div></div>
            <div class="field">
              <label>信道</label>
              <select v-model.number="radio.channel">
                <option v-for="ch in channels(radio)" :key="ch" :value="ch">{{ ch === 0 ? '自动' : ('信道 ' + ch) }}</option>
              </select>
            </div>
            <div class="field">
              <label>带宽</label>
              <select v-model="radio.htmode">
                <option v-for="h in htmodeList(radio)" :key="h" :value="h">{{ h }}</option>
              </select>
            </div>
            <div class="flex" style="justify-content:space-between;margin:6px 0 14px">
              <span style="font-size:12px;color:var(--muted)">启用该频段</span>
              <label class="switch">
                <input type="checkbox" :checked="!radio.disabled" @change="radio.disabled = $event.target.checked ? 0 : 1">
                <i></i>
              </label>
            </div>
            <div class="flex" style="justify-content:space-between;margin:6px 0 14px">
              <span>启用此 SSID</span><label class="switch"><input type="checkbox" :checked="!radio.ifaces[0].disabled" @change="radio.ifaces[0].disabled = $event.target.checked ? 0 : 1"><i></i></label>
            </div>
            <button class="btn primary" style="width:100%" :disabled="applying" @click="apply(radio)">应用本频段</button>
            </fieldset>
          </div>
          <div class="widget-body muted" v-else>该射频尚未配置 AP 接口，请在 LuCI 中添加。</div>
        </div>
      </div>
    </div>
    <p class="muted" style="font-size:12px;margin-top:12px">应用后该频段会重启，无线客户端会短暂断开；修改 SSID/密码后需要重新连接。</p>
  </div>`,
  computed: {
    loading() { return store.wifiLoading; },
    wifiError() { return store.wifiError; },
    radios() { return (store.wifi && store.wifi.radios) || []; },
    applying() { return store.wifiApplying; },
  },
  watch: {
    radios() {
      this.$nextTick(() => {
        if (!this.gs) this.initGrid();
        else this.$refs.grid.querySelectorAll('.grid-stack-item').forEach(el => {
          if (!el.gridstackNode) this.gs.makeWidget(el);
        });
      });
    },
  },
  mounted() { loadWifi(); },
  methods: {
    bandName(b) { return b === '2g' ? '2.4 GHz' : b === '5g' ? '5 GHz' : b === '6g' ? '6 GHz' : b; },
    supported(r) { return !r.mt || (r.mt.max_bw_mhz > 0 && (r.band !== '6g' || r.mt.support_6g)); },
    reload() { loadWifi(); },
    channels(r) { return [...new Set([...channelOptions(r.band), Number(r.channel) || 0])].sort((a,b) => a-b); },
    htmodeList(r) { return htmodeOptions(r); },
    apply(r) { if (r.ifaces && r.ifaces.length) applyWifi(r, r.ifaces[0]); },
  },
};

/* ---------- clients ---------- */
const ClientsView = {
  mixins: [gridPage('clients')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="clients" gs-w="12" gs-h="5">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head">
            <h3>LAN 与无线设备 <span class="muted" style="font-weight:400;font-size:12px">共 {{ rows.length }} 台</span></h3>
            <div class="flex"><button class="btn ghost" @click="refresh">{{ loading ? '刷新中…' : '刷新' }}</button><span class="drag-handle">⠿</span></div>
          </div>
          <div class="widget-body">
            <table>
              <thead><tr><th>设备</th><th>IP 地址</th><th>MAC</th><th>连接</th><th>信号</th><th>速率</th></tr></thead>
              <tbody>
                <tr v-for="c in rows" :key="c.mac">
                  <td><b style="font-size:13px">{{ c.host }}</b></td>
                  <td class="mono">{{ c.ip }}</td>
                  <td class="mono">{{ c.mac }}</td>
                  <td>{{ c.band }}<small v-if="c.neighbor" class="muted neighbor-state">{{ c.state }}</small></td>
                  <td><span :class="sigClass(c.signal)">{{ c.signal != null ? c.signal + ' dBm' : '—' }}</span></td>
                  <td class="mono">{{ c.rate != null ? c.rate + ' Mbps' : '—' }}</td>
                </tr>
                <tr v-if="!rows.length"><td colspan="6" class="empty">{{ loading ? '读取中…' : '尚无设备记录' }}</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { timer: 0 }; },
  computed: {
    rows() { return store.clients; },
    loading() { return store.clientsLoading; },
  },
  mounted() {
    refreshClients();
    this.timer = setInterval(() => { if (!document.hidden) refreshClients(); }, 12000);
  },
  unmounted() { clearInterval(this.timer); },
  methods: {
    refresh() { refreshClients(); },
    sigClass(s) { if (s == null) return 'muted'; return s >= -60 ? 'sig-good' : s >= -75 ? 'sig-mid' : 'sig-bad'; },
  },
};

/* ---------- system ---------- */
const SystemView = {
  components: {},
  mixins: [gridPage('system-no-duplicate-diag-v2')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <home-settings v-if="homeLoaded"></home-settings>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="mem" gs-w="12" gs-h="3">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>内存与存储</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="flex" style="justify-content:space-between;font-size:12px">
              <span class="muted">内存</span><span>{{ memUsed }} / {{ memTotal }}</span>
            </div>
            <div class="mem-bar"><i :style="{width: memPct + '%'}"></i></div>
            <div class="flex" style="justify-content:space-between;font-size:12px;margin-top:14px">
              <span class="muted">可写存储</span><span>{{ rootUsed }} / {{ rootTotal }}</span>
            </div>
            <div class="mem-bar"><i :style="{width: rootPct + '%'}"></i></div>
            <div class="mem-row" style="margin-top:14px"><span class="muted">系统负载（1/5/15 分钟）</span><span>{{ load }}</span></div>
            <div class="load-row" v-for="(l, idx) in loads" :key="idx">
              <span class="muted">{{ l.label }}</span>
              <b>{{ l.value }}</b>
              <div class="mini-bar"><i :class="l.cls" :style="{width: l.pct + '%'}"></i></div>
            </div>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="leds" gs-w="12" gs-h="2">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>面板灯</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body flex" style="justify-content:space-between">
            <span class="muted" style="font-size:12px">关闭后所有前面板指示灯熄灭（状态服务继续运行）</span>
            <label class="switch">
              <input type="checkbox" :checked="!ledsOff" @change="toggleLeds($event.target.checked)">
              <i></i>
            </label>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="maint" gs-w="12" gs-h="2">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>维护</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body row-actions">
            <button class="btn ghost" @click="openPw">修改登录密码</button>
            <button class="btn danger" @click="reboot">重启路由器</button>
            <a class="btn ghost" href="/cgi-bin/luci" target="_blank">高级设置（LuCI）</a>
          </div>
        </div>
      </div>
    </div>
    <signed-update-view></signed-update-view>
    <div class="modal-mask" v-if="showPw" @click.self="showPw=false">
      <div class="modal card fade-up">
        <h3>修改登录密码</h3>
        <div class="field"><label>新密码（至少 6 位）</label><input type="password" v-model="pw1" autocomplete="new-password"></div>
        <div class="field"><label>确认新密码</label><input type="password" v-model="pw2" autocomplete="new-password"></div>
        <div class="err">{{ pwMsg }}</div>
        <div class="row-actions" style="justify-content:flex-end">
          <button class="btn ghost" @click="showPw=false">取消</button>
          <button class="btn primary" :disabled="pwBusy" @click="savePw">{{ pwBusy ? '保存中…' : '保存' }}</button>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { ledsOff: false, showPw: false, pw1: '', pw2: '', pwMsg: '', pwBusy: false }; },
  computed: {
    homeLoaded() { return homeState.loaded; },
    board() { return store.board || {}; },
    release() { const r = this.board.release; return r ? (r.description || (r.distribution + ' ' + r.version)) : '—'; },
    uptime() { return fmtUptime(store.info && store.info.uptime); },
    temp() { const t = store.status && store.status.temperature; return t ? t + ' °C' : '—'; },
    memUsed() { const m = store.info && store.info.memory; return m ? fmtBytes(m.total - m.available) : '—'; },
    memTotal() { const m = store.info && store.info.memory; return m ? fmtBytes(m.total) : '—'; },
    memPct() { const m = store.info && store.info.memory; return m ? Math.round((m.total - m.available) / m.total * 100) : 0; },
    rootUsed() { const r = store.info && store.info.root; return r ? fmtBytes(r.used * 1024) : '—'; },
    rootTotal() { const r = store.info && store.info.root; return r ? fmtBytes(r.total * 1024) : '—'; },
    rootPct() { const r = store.info && store.info.root; return r ? Math.round(r.used / r.total * 100) : 0; },
    loads() {
      const l = store.info && store.info.load;
      if (!l || !l.length) return [];
      const vals = l.slice(0, 3).map(x => x / 65536);
      const maxv = Math.max.apply(null, vals.concat([0.01]));
      const labels = ['1 分钟', '5 分钟', '15 分钟'];
      return vals.map((v, i) => ({
        label: labels[i] || ('#' + i),
        value: v.toFixed(2),
        pct: Math.max(4, Math.round(v / maxv * 100)),
        cls: (v / 4) > 1 ? 'hot' : ((v / 4) > 0.6 ? 'warm' : ''),
      }));
    },
    load() {
      const l = store.info && store.info.load;
      if (!l) return '—';
      return l.map(x => (x / 65536).toFixed(2)).join(' / ');
    },
  },
  mounted() { loadLeds().then(v => { this.ledsOff = v; }); },
  methods: {
    async toggleLeds(on) { if (await setLedsOff(!on)) this.ledsOff = !on; },
    reboot() { doReboot(); },
    openPw() { this.pw1 = ''; this.pw2 = ''; this.pwMsg = ''; this.showPw = true; },
    async savePw() {
      this.pwMsg = '';
      if ((this.pw1 || '').length < 6) { this.pwMsg = '密码至少 6 位'; return; }
      if (this.pw1 !== this.pw2) { this.pwMsg = '两次输入不一致'; return; }
      this.pwBusy = true;
      try {
        const r = await ubus('luci', 'setPassword', { username: 'root', password: this.pw1 });
        requireSuccess(r);
        if (r.result !== true) throw new Error('设备未确认密码更新');
        toast('密码已更新，请用新密码登录');
        this.showPw = false;
        logout();
      } catch (e) { this.pwMsg = '修改失败：' + e.message; }
      finally { this.pwBusy = false; }
    },
  },
};

/* ---------- sms ---------- */
function decodeUcs2(s) {
  const h = String(s || '').replace(/\s+/g, '');
  if (!h || !/^[0-9a-fA-F]+$/.test(h) || h.length % 4) return s;
  let out = '';
  for (let i = 0; i < h.length; i += 4) out += String.fromCharCode(parseInt(h.substr(i, 4), 16));
  return out;
}

function decodeSmsBody(body) {
  const raw=String(body || '');
  const compact=raw.replace(/\s+/g, '');
  return compact && /^[0-9a-fA-F]+$/.test(compact) && compact.length % 4 === 0 ? decodeUcs2(compact) : raw;
}

function parseSms(text) {
  const lines = String(text || '').split('\n');
  const out = [];
  let cur = null;
  for (const raw of lines) {
    const line = raw.trim();
    const m = /^\+CMGL:\s*(\d+),"([^"]*)","([^"]*)","([^"]*)","([^"]*)"/.exec(line);
    if (m) {
      if (cur) out.push(cur);
      cur = { index: +m[1], status: m[2], number: m[3], date: m[5], body: '' };
      continue;
    }
    if (!cur) continue;
    if (line === 'OK' || line === '') continue;
    cur.body += raw.replace(/\r$/, '') + '\n';
  }
  if (cur) out.push(cur);
  return out.map(m => ({ index: m.index, status: m.status, number: decodeUcs2(m.number), date: m.date, text: decodeSmsBody(m.body.replace(/\n$/, '')) || '(空)' }));
}

const SmsView = {
  mixins: [gridPage('sms')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing">
      <span class="muted">拖拽标题栏移动模块，拖右下角缩放</span>
    </div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="inbox" gs-w="7" gs-h="7">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head">
            <h3>收件箱 <span class="muted" style="font-weight:400;font-size:12px">{{ messages.length }} 条</span></h3>
            <div class="flex"><button class="btn ghost" @click="loadSms">{{ loading ? '读取中…' : '刷新' }}</button><span class="drag-handle">⠿</span></div>
          </div>
          <div class="widget-body">
            <div class="row-actions sms-toolbar"><label><input type="checkbox" :checked="messages.length > 0 && selected.length === messages.length" @change="selected = $event.target.checked ? messages.map(m=>m.index) : []"> 全选</label><button class="btn danger" :disabled="deleting || !selected.length" @click="deleteSelected">删除所选 {{ selected.length || '' }}</button></div>
            <div class="sms-list">
              <div class="sms-item" v-for="m in messages" :key="m.index">
                <div class="sms-meta"><label><input type="checkbox" v-model="selected" :value="m.index"> <b>{{ m.number || '未知号码' }}</b></label><span>{{ /UNREAD/.test(m.status) ? '未读 · ' : '' }}{{ m.date }}</span><button class="btn ghost sms-delete" :disabled="deleting" @click="removeMessages([m.index])">删除</button></div>
                <div class="sms-body">{{ m.text }}</div>
              </div>
              <div v-if="!messages.length" class="empty">{{ loading ? '读取中…' : '暂无短信' }}</div>
            </div>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="compose" gs-w="5" gs-h="7">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>发送短信</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="field"><label>接收号码</label><input v-model="number" placeholder="+8613800000000"></div>
            <div class="field"><label>内容（英文/数字，≤160 字符）</label><textarea v-model="text" rows="6" maxlength="160" class="ta"></textarea></div>
            <button class="btn primary" :disabled="sending" @click="send">{{ sending ? '发送中…' : '发送' }}</button>
            <p class="muted" style="font-size:11.5px;margin-top:10px">中文短信需要模组 UCS2 支持，当前版本仅支持英文/数字。</p>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { loading: false, sending: false, deleting: false, selected: [], messages: [], number: '', text: '' }; },
  mounted() { this.loadSms(); },
  methods: {
    async loadSms() {
      this.loading = true;
      try {
        const r = await ubus('luci.air6modem', 'list_sms');
        requireSuccess(r);
        this.messages = parseSms(r && r.output);
        this.selected = this.selected.filter(index=>this.messages.some(m=>m.index===index));
        store.smsUnread = this.messages.filter(m => /UNREAD/.test(m.status)).length;
      } catch (e) { toast('读取短信失败：' + e.message); }
      finally { this.loading = false; }
    },
    deleteSelected() { return this.removeMessages([...this.selected]); },
    async removeMessages(indexes) {
      if(this.deleting || !indexes.length || !confirm('确认删除 '+indexes.length+' 条短信？')) return;
      this.deleting=true; let removed=0;
      try { for(const index of indexes) { requireSuccess(await ubus('luci.air6modem','delete_sms',{index})); removed++; } toast('已删除 '+removed+' 条短信'); }
      catch(e) { toast('已删除 '+removed+' 条，后续删除失败：'+e.message); }
      finally { this.deleting=false; await this.loadSms(); }
    },
    async send() {
      const num = (this.number || '').trim();
      const body = (this.text || '').trim();
      if (!/^\+?[0-9]{3,20}$/.test(num)) { toast('号码格式不正确'); return; }
      if (!/^[ -~]{1,160}$/.test(body)) { toast('短信需为 1–160 个英文、数字或 ASCII 符号'); return; }
      if (this.sending) return;
      this.sending = true;
      try {
        const r = await ubus('luci.air6modem', 'send_sms', { number: num, text: body });
        if (r && r.success) { toast('短信已提交'); this.text = ''; this.loadSms(); }
        else toast('发送失败：' + ((r && r.error) || '模组未确认'));
      } catch (e) { toast('发送失败：' + e.message); }
      finally { this.sending = false; }
    },
  },
};

/* ---------- traffic ---------- */
const TrafficView = {
  mixins: [gridPage('traffic-chart-v2')],
  components: { TrafficChart },
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing"><span class="muted">拖拽标题栏移动模块，拖右下角缩放</span></div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="wave" gs-w="12" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>实时波形 <span class="muted" style="font-weight:400;font-size:12px">{{ waveIface || '等待流量' }}</span></h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body chart-body">
            <div class="rates">
              <div><small>↓ 下载</small><b>{{ rxNow.toFixed(2) }}<span>Mbps</span></b></div>
              <div><small>↑ 上传</small><b>{{ txNow.toFixed(2) }}<span>Mbps</span></b></div>
            </div>
            <traffic-chart :samples="wave" :seq="waveSeq"></traffic-chart>
            <div class="legend"><span class="rx"><i></i>下载</span><span class="tx"><i></i>上传</span><span style="margin-left:auto">当前接口 · {{ waveIface || '—' }}</span></div>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="rates" gs-w="8" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>接口速率</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <table>
              <thead><tr><th>接口</th><th>下载</th><th>上传</th><th>累计接收</th><th>累计发送</th></tr></thead>
              <tbody>
                <tr v-for="r in rows" :key="r.name">
                  <td><b>{{ r.name }}</b></td>
                  <td class="mono">{{ r.rx.toFixed(2) }} Mbps</td>
                  <td class="mono">{{ r.tx.toFixed(2) }} Mbps</td>
                  <td class="mono">{{ r.rxTotal }}</td>
                  <td class="mono">{{ r.txTotal }}</td>
                </tr>
                <tr v-if="!rows.length"><td colspan="5" class="empty">读取中…</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="sum" gs-w="4" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>本次运行</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="stat-grid">
              <div class="stat"><span>下载累计</span><b>{{ totalRx }}</b></div>
              <div class="stat"><span>上传累计</span><b>{{ totalTx }}</b></div>
              <div class="stat"><span>峰值下载</span><b>{{ peakRx.toFixed(2) }} <i>Mbps</i></b></div>
              <div class="stat"><span>峰值上传</span><b>{{ peakTx.toFixed(2) }} <i>Mbps</i></b></div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { rows: [], peakRx: 0, peakTx: 0, prev: {}, timer: 0, wave: [], waveSeq: 0, waveIface: '', rxNow: 0, txNow: 0 }; },
  computed: {
    totalRx() { const d = pickWan(store.accel && store.accel.interfaces); return d ? fmtBytes(d.rx_bytes) : '—'; },
    totalTx() { const d = pickWan(store.accel && store.accel.interfaces); return d ? fmtBytes(d.tx_bytes) : '—'; },
  },
  mounted() { this.sample(); this.timer = setInterval(() => { if (!document.hidden) this.sample(); }, 3000); },
  beforeUnmount() { clearInterval(this.timer); },
  methods: {
    async sample() {
      try {
        const a = await ubus('luci.air6modem', 'get_accel_stats');
        const now = Date.now();
        const rows = (a.interfaces || []).map(i => {
          const p = this.prev[i.name];
          let rx = 0, tx = 0;
          if (p) {
            const dt = Math.max((now - p.t) / 1000, 0.5);
            rx = Math.max(0, (i.rx_bytes - p.rx) * 8 / dt / 1e6);
            tx = Math.max(0, (i.tx_bytes - p.tx) * 8 / dt / 1e6);
          }
          this.prev[i.name] = { t: now, rx: i.rx_bytes, tx: i.tx_bytes };
          return { name: i.name, rx, tx, rxTotal: fmtBytes(i.rx_bytes), txTotal: fmtBytes(i.tx_bytes) };
        });
        this.rows = rows;
        const selected = pickWan(a.interfaces);
        const wan = selected && rows.find(r => r.name === selected.name);
        if (wan) { this.peakRx = Math.max(this.peakRx, wan.rx); this.peakTx = Math.max(this.peakTx, wan.tx); }
        const active = wan;
        if (!active) { this.rxNow = 0; this.txNow = 0; this.waveIface = ''; }
        if (active) {
          if (this.waveIface !== active.name) { this.wave = []; this.peakRx = 0; this.peakTx = 0; }
          this.waveIface = active.name;
          this.rxNow = active.rx;
          this.txNow = active.tx;
          this.wave.push({ rx: active.rx, tx: active.tx });
          if (this.wave.length > 60) this.wave.shift();
          this.waveSeq++;
        }
      } catch (e) {}
    },
  },
};

/* ---------- security ---------- */
const SecurityView = {
  mixins: [gridPage('security-aligned-v2')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing"><span class="muted">拖拽标题栏移动模块，拖右下角缩放</span></div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="wan" gs-x="0" gs-y="0" gs-w="6" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>WAN 信息</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <div class="stat-grid">
              <div class="stat"><span>IPv4 地址</span><b>{{ addr }}</b></div>
              <div class="stat"><span>网关</span><b>{{ gw }}</b></div>
              <div class="stat wide"><span>IPv6 地址</span><b :title="addr6">{{ addr6 }}</b></div>
              <div class="stat"><span>协议</span><b>{{ proto }}</b></div>
              <div class="stat"><span>接口</span><b>{{ dev }}</b></div>
              <div class="stat wide"><span>DNS</span><b :title="dns">{{ dns }}</b></div>
            </div>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="dhcp" gs-x="6" gs-y="0" gs-w="6" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>DHCP 租约</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <table>
              <thead><tr><th>主机名</th><th>IP</th><th>MAC</th></tr></thead>
              <tbody>
                <tr v-for="l in leases" :key="l.mac"><td>{{ l.host }}</td><td class="mono">{{ l.ip }}</td><td class="mono">{{ l.mac }}</td></tr>
                <tr v-if="!leases.length"><td colspan="3" class="empty">暂无租约</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="links" gs-w="12" gs-h="2">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>安全与转发</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body row-actions">
            <a class="btn ghost" href="/cgi-bin/luci/admin/network/firewall" target="_blank">防火墙</a>
            <a class="btn ghost" href="/cgi-bin/luci/admin/network/firewall/forwards" target="_blank">端口转发</a>
            <a class="btn ghost" href="/cgi-bin/luci/admin/network/dhcp" target="_blank">DHCP / DNS</a>
            <a class="btn ghost" href="/cgi-bin/luci/admin/network/network" target="_blank">接口配置</a>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { leases: [] }; },
  computed: {
    net() { return (store.status && store.status.network) || {}; },
    addr() { const a = this.net['ipv4-address']; return (a && a[0] && a[0].address) ? a[0].address : '—'; },
    addr6() { const a=this.net['ipv6-address'] || []; return a.length ? a.map(x=>x.address).join(' / ') : '—'; },
    gw() { const r = this.net.route; return (r && r.find(x => x.mask === 0) && r.find(x => x.mask === 0).nexthop) ? r.find(x => x.mask === 0).nexthop : '—'; },
    dns() { const d = this.net['dns-server']; return (d && d.length) ? d.join(', ') : '—'; },
    dev() { return this.net.l3_device || this.net.device || '—'; },
    proto() { return this.net.proto || '—'; },
  },
  mounted() { this.load(); },
  methods: {
    async load() {
      const l = await readLeases();
      this.leases = Object.keys(l).map(m => ({ mac: m, ip: l[m].ip, host: l[m].host || '未知设备' }));
    },
  },
};

/* ---------- services ---------- */
const ServicesView = {
  mixins: [gridPage('services')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing"><span class="muted">拖拽标题栏移动模块，拖右下角缩放</span></div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="svc" gs-w="12" gs-h="6">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head">
            <h3>系统服务 <span class="muted" style="font-weight:400;font-size:12px">{{ services.length }} 项</span></h3>
            <div class="flex"><button class="btn ghost" @click="load">刷新</button><span class="drag-handle">⠿</span></div>
          </div>
          <div class="widget-body">
            <table>
              <thead><tr><th>服务</th><th>状态</th><th class="right">操作</th></tr></thead>
              <tbody>
                <tr v-for="v in services" :key="v.name">
                  <td><b>{{ v.name }}</b></td>
                  <td><span :class="v.running ? 'sig-good' : 'muted'">{{ v.running ? '运行中' : '未运行' }}</span></td>
                  <td class="right"><button v-if="canRestart(v)" class="btn ghost" :disabled="busy===v.name" @click="restart(v)">{{ busy===v.name ? '重启中…' : '重启' }}</button></td>
                </tr>
                <tr v-if="!services.length"><td colspan="3" class="empty">读取中…</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { services: [], busy: '' }; },
  mounted() { this.load(); },
  methods: {
    async load() {
      try {
        const r = await ubus('service', 'list', {});
        const out = [];
        for (const name of Object.keys(r || {})) {
          const inst = (r[name] && r[name].instances) || {};
          const running = Object.keys(inst).some(k => inst[k].running);
          out.push({ name, running });
        }
        out.sort((a, b) => a.name.localeCompare(b.name));
        this.services = out;
      } catch (e) { toast('读取服务失败：' + e.message); }
    },
    canRestart(v) {
      return ['dnsmasq', 'firewall', 'dropbear', 'uhttpd', 'rpcd', 'air6-modem-status', 'air6-panel-led', 'air6-simslot'].includes(v.name);
    },
    async restart(v) {
      if (!confirm('重启服务 ' + v.name + '？')) return;
      this.busy = v.name;
      try {
        requireSuccess(await ubus('luci', 'setInitAction', { name: v.name, action: 'restart' }));
        toast('已提交重启');
        setTimeout(() => this.load(), 1800);
      } catch (e) { toast('重启失败：' + e.message); }
      finally { this.busy = ''; }
    },
  },
};

/* ---------- interfaces ---------- */
const InterfacesView = {
  mixins: [gridPage('interfaces')],
  template: `
  <div :class="{editing: editing}" class="fade-up">
    <div class="page-tools" v-if="editing"><span class="muted">拖拽标题栏移动模块，拖右下角缩放</span></div>
    <div class="grid-stack" ref="grid">
      <div class="grid-stack-item" gs-id="ifaces" gs-w="12" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>网络接口</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <table>
              <thead><tr><th>接口</th><th>状态</th><th>协议</th><th>设备</th><th>IPv4</th><th>运行时长</th></tr></thead>
              <tbody>
                <tr v-for="i in ifs" :key="i.name">
                  <td><b>{{ i.name }}</b></td>
                  <td><span :class="i.up ? 'sig-good' : 'muted'">{{ i.up ? '在线' : '离线' }}</span></td>
                  <td>{{ i.proto }}</td><td class="mono">{{ i.dev }}</td><td class="mono">{{ i.addr }}</td><td>{{ i.uptime }}</td>
                </tr>
                <tr v-if="!ifs.length"><td colspan="6" class="empty">读取中…</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
      <div class="grid-stack-item" gs-id="radios" gs-w="12" gs-h="4">
        <div class="grid-stack-item-content card widget">
          <div class="widget-head"><h3>无线设备</h3><span class="drag-handle">⠿</span></div>
          <div class="widget-body">
            <table>
              <thead><tr><th>频段</th><th>接口</th><th>SSID</th><th>信道</th><th>带宽</th><th>状态</th></tr></thead>
              <tbody>
                <tr v-for="r in radios" :key="r.ifname">
                  <td><b>{{ r.band }}</b></td><td class="mono">{{ r.ifname }}</td><td>{{ r.ssid }}</td>
                  <td class="mono">{{ r.ch }}</td><td>{{ r.bw }}</td><td>{{ r.state }}</td>
                </tr>
                <tr v-if="!radios.length"><td colspan="6" class="empty">读取中…</td></tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>`,
  data() { return { ifs: [], radios: [] }; },
  mounted() { this.load(); },
  methods: {
    async load() {
      try {
        const d = await ubus('network.interface', 'dump');
        this.ifs = (d.interface || []).map(i => ({
          name: i.interface, up: !!i.up, proto: i.proto || '—',
          dev: i.l3_device || i.device || '—',
          addr: (i['ipv4-address'] && i['ipv4-address'][0] && i['ipv4-address'][0].address) || '—',
          uptime: i.up ? fmtUptime(i.uptime) : '—',
        }));
        const w = await ubus('air6.cpe', 'get_wifi');
        this.radios = (w.radios || []).map(r => ({
          band: r.band, ifname: r.ifname, ssid: ((r.ifaces || [])[0] || {}).ssid || '—',
          ch: r.mt ? r.mt.channel : r.channel,
          bw: r.mt ? (r.mt.bandwidth_mhz + ' MHz') : r.htmode,
          state: r.disabled ? '已关闭' : ((r.ifaces || []).every(i => i.disabled) ? '接口已关闭' : '已启用'),
        }));
      } catch (e) { toast('读取接口失败：' + e.message); }
    },
  },
};

function downloadText(name, text) {
  const url = URL.createObjectURL(new Blob([text], {type: 'text/plain;charset=utf-8'}));
  const link = document.createElement('a'); link.href = url; link.download = name;
  link.click(); setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function applyAppearance(theme) {
  const value = ['auto', 'light', 'dark'].includes(theme) ? theme : 'auto';
  const dark = value === 'dark' || (value === 'auto' && window.matchMedia('(prefers-color-scheme: dark)').matches);
  document.documentElement.dataset.theme = dark ? 'dark' : 'light';
  try { localStorage.setItem('a6.theme', value); } catch (e) {}
}

const ToolsView = {
  template: `
  <div class="tools-grid fade-up">
    <section class="card tool-card">
      <h3>网络诊断</h3>
      <div class="field"><label>诊断方式</label><select v-model="tool"><option value="ping4">IPv4 Ping</option><option value="ping6">IPv6 Ping</option><option value="dns">DNS 查询</option><option value="trace">路由追踪</option></select></div>
      <div class="field"><label>域名或 IP 地址</label><input v-model="host" @keyup.enter="diagnose" placeholder="例如 192.168.99.1"></div>
      <button class="btn primary" :disabled="diagBusy" @click="diagnose">{{ diagBusy ? '诊断中…' : '开始诊断' }}</button>
      <span class="tool-result" v-if="diagState">{{ diagState }}</span>
      <pre class="at" v-if="diagOut">{{ diagOut }}</pre>
      <button v-if="diagOut" class="btn ghost mt16" @click="saveDiag">导出诊断结果</button>
    </section>
    <section class="card tool-card">
      <h3>模组快捷控制</h3>
      <dl class="kv"><div><dt>射频状态</dt><dd>{{ radioState }}</dd></div><div><dt>SIM 状态</dt><dd>{{ simState }}</dd></div></dl>
      <div class="row-actions mt16"><button class="btn ghost" :disabled="radioBusy" @click="setRadio(true)">开启射频</button><button class="btn danger" :disabled="radioBusy" @click="setRadio(false)">关闭射频</button></div>
      <p class="muted">关闭射频会中断蜂窝连接；本地有线管理仍可使用。</p>
      <div class="field"><label>SIM PIN（仅在 SIM 请求 PIN 时提交）</label><input type="password" inputmode="numeric" v-model="pin" maxlength="8" autocomplete="off" :disabled="pinBusy"></div>
      <button class="btn ghost" :disabled="pinBusy" @click="unlockPin">{{ pinBusy ? '提交中…' : '提交 SIM PIN' }}</button>
      <p class="muted">{{ needsPin ? '请确认 PIN；错误输入会消耗剩余尝试次数。' : '当前 SIM 未请求 PIN。' }}</p>
    </section>
    <section class="card tool-card tool-wide">
      <div class="tool-heading"><h3>系统日志</h3><div class="row-actions"><select v-model="logSource" @change="loadLogs"><option value="system">系统日志</option><option value="kernel">内核日志</option></select><button class="btn ghost" :disabled="logBusy" @click="loadLogs">{{ logBusy ? '读取中…' : '刷新日志' }}</button><button class="btn ghost" :disabled="!logs" @click="saveLogs">导出日志</button></div></div>
      <div class="field"><label>筛选当前日志</label><input v-model="logFilter" placeholder="输入关键词，例如 wifi、modem、error"></div>
      <p class="muted">{{ logStamp ? '读取于 ' + logStamp + ' · 最近最多 250 行' : '进入页面时读取，可手动刷新' }}</p>
      <pre class="at log-output">{{ filteredLogs || (logBusy ? '正在读取…' : '没有匹配的日志') }}</pre>
    </section>
    <section class="card tool-card tool-wide">
      <h3>界面外观</h3>
      <div class="row-actions theme-options"><button v-for="t in themes" :key="t.id" class="btn" :class="theme === t.id ? 'primary' : 'ghost'" @click="setTheme(t.id)">{{ t.label }}</button></div>
      <p class="muted">外观保存在当前浏览器，支持跟随系统切换。三频无线配置保持紧凑排列。</p>
    </section>
  </div>`,
  data() { let theme = 'auto'; try { theme = localStorage.getItem('a6.theme') || 'auto'; } catch(e) {} return {tool:'ping4',host:location.hostname,diagBusy:false,diagOut:'',diagState:'',logs:'',logSource:'system',logFilter:'',logBusy:false,logStamp:'',radioBusy:false,pin:'',pinBusy:false,theme,themes:[{id:'auto',label:'跟随系统'},{id:'light',label:'明亮'},{id:'dark',label:'深色'}]}; },
  computed: {
    filteredLogs() { const q = this.logFilter.trim().toLowerCase(); return q ? this.logs.split('\n').filter(line=>line.toLowerCase().includes(q)).join('\n') : this.logs; },
    radioState() { const s = String(store.status && store.status.radio || ''); return s.includes('STATE_ON') ? '已开启' : s.includes('STATE_OFF') ? '已关闭' : '等待状态更新'; },
    simState() { const raw=String(store.status && store.status.sim || ''); return /READY/i.test(raw) ? 'SIM 已就绪' : /ABSENT|NOT_INSERT/i.test(raw) ? '未检测到 SIM' : /PUK/i.test(raw) ? '需要 PUK' : /PIN/i.test(raw) ? '需要 SIM PIN' : raw || '等待状态更新'; },
    needsPin() { return /(?:SIM PIN|PIN_REQUIRED|REQUIRE_PIN|NOT_VERIFIED)/i.test(String(store.status && store.status.sim || '')) && !/PUK/i.test(String(store.status && store.status.sim || '')); },
  },
  mounted() { this.loadLogs(); },
  methods: {
    async diagnose() {
      if (this.diagBusy) return;
      const host=this.host.trim();
      if (!/^[A-Za-z0-9:][A-Za-z0-9._:-]{0,252}$/.test(host)) { toast('请输入有效的域名或 IP'); return; }
      this.diagBusy=true; this.diagState=''; this.diagOut='';
      try { const r=await ubus('air6.cpe','diagnose',{tool:this.tool,host}); if(r.error) throw new Error(r.error); this.diagOut=r.output || '(无输出)'; this.diagState=r.ok ? '诊断完成' : '未成功完成，请查看输出'; }
      catch(e) { this.diagOut='诊断失败：'+e.message; }
      finally { this.diagBusy=false; }
    },
    async loadLogs() {
      if(this.logBusy) return; this.logBusy=true;
      try { const r=await ubus('air6.cpe','get_logs',{source:this.logSource}); requireSuccess(r); this.logs=r.output || ''; this.logStamp=new Date().toLocaleTimeString(); }
      catch(e) { toast('日志读取失败：'+e.message); }
      finally { this.logBusy=false; }
    },
    saveDiag() { downloadText('air6-'+this.tool+'-diagnostic.txt',this.tool+' '+this.host+'\n'+this.diagOut); },
    saveLogs() { downloadText('air6-'+this.logSource+'-log.txt',this.filteredLogs); },
    setTheme(theme) { this.theme=theme; applyAppearance(theme); },
    async setRadio(enabled) {
      if(this.radioBusy || !confirm(enabled ? '确认开启蜂窝射频？' : '确认关闭蜂窝射频？蜂窝连接将中断。')) return;
      this.radioBusy=true;
      try { requireSuccess(await ubus('luci.air6modem','set_radio',{enabled})); toast(enabled ? '已提交开启射频' : '已提交关闭射频'); setTimeout(refreshStatus,2000); }
      catch(e) { toast('射频操作失败：'+e.message); }
      finally { this.radioBusy=false; }
    },
    async unlockPin() {
      if(this.pinBusy) return;
      if(!this.needsPin) { toast('当前 SIM 未请求 PIN，无需提交；网络锁授权请使用对应 AT 命令。'); return; }
      if(!/^\d{4,8}$/.test(this.pin)) { toast('SIM PIN 需为 4–8 位数字'); return; }
      if(!confirm('确认提交此 SIM PIN？错误 PIN 会消耗剩余尝试次数。')) return;
      this.pinBusy=true;
      try { requireSuccess(await ubus('luci.air6modem','run_at',{command:'AT+CPIN="'+this.pin+'"'})); this.pin=''; toast('PIN 已提交，等待 SIM 就绪'); setTimeout(refreshStatus,3000); }
      catch(e) { toast('PIN 提交失败：'+e.message); }
      finally { this.pinBusy=false; }
    },
  },
};

/* Shared module catalogue: render the existing controls with their original
 * Vue state/methods, instead of screenshots or copies of their business logic. */
const homeState = reactive({name:'我的首页', modules:[], layout:[], landing:false, revision:0, loaded:false,manual:false});
const homeSources = {overview:OverviewView,cellular:CellularView,wireless:WirelessView,clients:ClientsView,traffic:TrafficView,security:SecurityView,services:ServicesView,interfaces:InterfacesView,sms:SmsView,tools:ToolsView,system:SystemView};
const homeModules = [];
for (const [page,source] of Object.entries(homeSources)) {
  const doc=document.createElement('template');doc.innerHTML=source.template;
  const nodes=[...doc.content.querySelectorAll(page==='wireless'?'.grid-stack-item':'[gs-id]')];
  if(page==='tools') nodes.push(...doc.content.querySelectorAll('section.tool-card'));
  nodes.forEach((node,i)=>{
    const id=page+':'+(page==='wireless'?'radios':node.getAttribute('gs-id') || 'tool'+i);
    const card=node.matches('section') ? node : node.querySelector('.card');
    if(!card) return;
    const title=(card.querySelector('h3,h4,.metric-title')?.textContent || (page==='wireless'?'三频无线设置':'模块 '+(i+1))).replace(/\{\{.*?\}\}/g,'').replace(/\s+共\s*台|\s+项$|\s+条$/g,'').trim();
    let html=card.outerHTML;
    if(page==='wireless') {
      for(const [band,label] of [['2g','2.4 GHz 无线设置'],['5g','5 GHz 无线设置'],['6g','6 GHz 无线设置']])
        homeModules.push({id:'wireless:'+band,page,label,group:TITLES[page][0],html:'<div v-if="loading" class="card widget"><div class="widget-body muted">正在读取无线配置…</div></div><div v-for="radio in radios.filter(r=>r.band===\''+band+'\')" :key="radio.section">'+html+'</div>',w:4,h:6});
      return;
    }
    homeModules.push({id,page,label:title,group:TITLES[page][0],html,w:+node.getAttribute('gs-w')||6,h:+node.getAttribute('gs-h')||4});
  });
}
async function loadHome() {
  try { const r=requireSuccess(await ubus('air6.cpe','get_home'));const c=r.config || {};
    homeState.name=c.name || '我的首页';homeState.modules=[...new Set((c.modules || []).map(id=>id==='system:info'?'overview:device':id))].filter(id=>homeModules.some(m=>m.id===id));
    homeState.layout=(c.layout || []).filter(i=>i.id!=='system:info');homeState.manual=!!c.manual;homeState.landing=!!c.landing && homeState.modules.length>0;homeState.revision++;
    if(!homeState.modules.length && location.hash==='#/home'){store.view='overview';location.hash='#/overview';}
    if(homeState.landing && (!location.hash || location.hash==='#/overview')) {store.view='home';location.hash='#/home';}
  }catch(e){toast('读取自定义首页失败：'+e.message);}finally{homeState.loaded=true;}
}
async function persistHome(config) { requireSuccess(await ubus('air6.cpe','set_home',{config:JSON.stringify(config)}));Object.assign(homeState,config);homeState.revision++; }
const HomeSettings = {
  template:`<section class="card tool-card home-settings"><div class="home-heading"><div><h3>自定义首页</h3><p class="muted">把常用功能放在一起，进入首页后可拖动并调整大小。不选模块并保存即可关闭首页。</p></div><span class="home-count">已选 {{selected.length}} / {{catalogue.length}}</span></div><div class="home-config"><div class="field"><label>首页名称</label><input v-model.trim="name" maxlength="24" placeholder="给首页起个名字"></div><label class="home-check home-landing"><input type="checkbox" v-model="landing" :disabled="!selected.length">设为登录后的默认首页</label></div><div class="home-library"><div class="home-library-bar"><strong>选择功能模块</strong><input class="home-search" v-model="query" placeholder="搜索模块或分类" aria-label="搜索首页模块"></div><div class="home-categories" role="group" aria-label="模块分类"><button type="button" :class="{active:category==='全部'}" @click="category='全部'">全部 <span>{{catalogue.length}}</span></button><button type="button" :class="{active:category==='已选'}" @click="category='已选'">已选 <span>{{selected.length}}</span></button><button v-for="g in groups" :key="g" type="button" :class="{active:category===g}" @click="category=g">{{g}}</button></div><div class="home-module-list"><label v-for="m in filtered" :key="m.id" class="home-module-option" :class="{chosen:selected.includes(m.id)}"><input type="checkbox" :value="m.id" v-model="selected"><span><strong>{{m.label}}</strong><small>{{m.group}}</small></span><span class="home-option-mark" aria-hidden="true">{{selected.includes(m.id)?'✓':'+'}}</span></label><p v-if="!filtered.length" class="home-empty">{{category==='已选'?'还没有选择模块，切换分类添加常用功能。':'没有找到匹配的模块。'}}</p></div></div><div class="home-settings-footer"><p :class="message?'home-feedback':'muted'" role="status">{{message || '选择模块后保存，布局可在首页继续调整。'}}</p><div class="row-actions"><button class="btn ghost" :disabled="!selected.length || !homeEnabled" @click="open">打开我的首页</button><button class="btn primary" :disabled="busy" @click="save">{{busy?'保存中…':'保存首页'}}</button></div></div></section>`,
  data(){return{name:homeState.name,selected:[...homeState.modules],landing:homeState.landing,busy:false,message:'',query:'',category:'网络概览'};},
  computed:{homeRevision(){return homeState.revision;},homeEnabled(){return homeState.modules.length>0;},catalogue(){return homeModules;},groups(){return [...new Set(homeModules.map(m=>m.group))];},filtered(){const q=this.query.trim().toLowerCase();return homeModules.filter(m=>(this.category==='全部'||(this.category==='已选'?this.selected.includes(m.id):m.group===this.category))&&(!q||(m.label+' '+m.group).toLowerCase().includes(q)));}},
  watch:{'homeRevision'(){this.name=homeState.name;this.selected=[...homeState.modules];this.landing=homeState.landing;}},
  methods:{async save(){if(this.busy)return;if(this.selected.length && !this.name){this.message='请输入首页名称';return;}this.busy=true;try{await persistHome({name:this.name || homeState.name || '我的首页',modules:this.selected,landing:this.selected.length>0 && this.landing,manual:this.selected.length>0 && homeState.manual,layout:homeState.layout.filter(i=>this.selected.includes(i.id))});this.message=this.selected.length?'已保存，可进入首页自定义布局':'自定义首页已关闭';toast(this.message);}catch(e){this.message=e.message;}finally{this.busy=false;}},open(){store.view='home';location.hash='#/home';}},
};
const CustomHomeView = {
  template:`<div class="custom-home" :class="{editing:editing,'auto-layout':!manual &amp;&amp; !editing}"><div v-if="selected.length" class="home-layout-tools"><span class="muted">{{manual?'自定义布局':'自动排列 · 卡片按内容整齐对齐'}}</span><button class="btn ghost" @click="arrange">自动整理</button></div><div v-if="!selected.length" class="banner">尚未添加模块，请到“系统 → 自定义首页”选择。</div><div v-if="editing" class="banner">拖动模块标题调整位置，拖动右下角调整大小。布局自动保存。</div><div class="grid-stack" ref="grid"><div v-for="m in selected" :key="m.id" class="grid-stack-item" :class="{'home-wide':m.page==='interfaces'||m.page==='clients'}" :gs-id="m.id" :gs-w="m.w" :gs-h="m.h"><div class="grid-stack-item-content home-slot" :id="slotId(m.id)"></div></div></div><component v-for="p in providers" :is="p.component" :key="p.page+':'+revision"></component><p class="err">{{saveError}}</p></div>`,
  data(){return{gs:null,ready:false,saveTimer:0,saveError:''};},
  computed:{manual(){return homeState.manual;},editing(){return store.editing;},resetTick(){return store.resetTick;},revision(){return homeState.revision;},selected(){return homeState.modules.map(id=>homeModules.find(m=>m.id===id)).filter(Boolean).sort((a,b)=>Number(b.html.includes('widget metric'))-Number(a.html.includes('widget metric')));},providers(){if(!this.ready)return[];return [...new Set(this.selected.map(m=>m.page))].map(page=>{const source=homeSources[page],doc=document.createElement('template');doc.innerHTML=source.template;const dialogs=[...doc.content.querySelectorAll('.modal-mask')].map(n=>n.outerHTML).join('');return{page,component:Vue.markRaw({...source,render:undefined,mixins:[],template:'<div>'+this.selected.filter(m=>m.page===page).map(m=>'<teleport to="#'+this.slotId(m.id)+'">'+m.html+'</teleport>').join('')+dialogs+'</div>',computed:{...source.computed,editing(){return store.editing;}},methods:{...source.methods,initGrid(){},saveLayout(){},resetLayout(){}},mounted:source.mounted})};});}},
  watch:{editing(v){if(this.gs)this.gs.setStatic(!v);},revision(){this.rebuild();},resetTick(){if(confirm('恢复自定义首页的默认模块大小与位置？')){homeState.layout=[];homeState.manual=false;this.rebuild();}}},
  mounted(){this.rebuild();},beforeUnmount(){clearTimeout(this.saveTimer);this.flushLayout();if(this.gs)this.gs.destroy(false);},
  methods:{async arrange(){try{await persistHome({name:homeState.name,modules:homeState.modules,layout:[],manual:false,landing:homeState.landing});}catch(e){this.saveError='整理保存失败：'+e.message;}},slotId(id){return 'a6-home-'+id.replace(/[^a-zA-Z0-9_-]/g,'-');},async rebuild(){this.ready=false;if(this.gs){this.gs.destroy(false);this.gs=null;}await this.$nextTick();if(this.selected.length){this.gs=GridStack.init({column:12,cellHeight:96,margin:12,handle:'.widget-head,.tool-heading,h3',columnOpts:{breakpointForWindow:true,breakpoints:[{w:600,c:1},{w:1000,c:6}]}},this.$refs.grid);if(homeState.manual && homeState.layout.length)this.gs.load(homeState.layout,false);this.gs.setStatic(!store.editing);this.gs.on('change',()=>{if(!store.editing)return;homeState.manual=true;clearTimeout(this.saveTimer);this.saveTimer=setTimeout(()=>this.flushLayout(),700);});}this.ready=true;},async flushLayout(){if(!this.gs || this.gs.getColumn()!==12)return;const layout=this.gs.save(false).map(i=>({id:i.id,x:i.x,y:i.y,w:i.w,h:i.h}));homeState.layout=layout;try{requireSuccess(await ubus('air6.cpe','set_home',{config:JSON.stringify({name:homeState.name,modules:homeState.modules,layout,manual:homeState.manual,landing:homeState.landing})}));this.saveError='';}catch(e){this.saveError='布局保存失败：'+e.message;}}},
};
const SignedUpdateView = {
  template:`<section class="card tool-card"><h3>固件与 modem 更新</h3><p class="muted">当前运行槽：<strong>{{currentSlot?currentSlot.toUpperCase():'读取中…'}}</strong></p><p class="muted">仅接受由本机信任的发布密钥签发的 AIR6 更新包。先上传并校验，校验通过后才可刷入。</p><div class="row-actions"><button class="btn" :class="kind==='firmware'?'primary':'ghost'" :disabled="busy" @click="select('firmware')">更新固件</button><button class="btn" :class="kind==='modem'?'primary':'ghost'" :disabled="busy" @click="select('modem')">更新 modem</button></div><div class="field mt16"><label>{{kind==='firmware'?'系统固件签名包':'modem 签名包'}}（.bin）</label><input type="file" accept=".bin,.tar" :disabled="busy" @change="choose"></div><p class="muted">{{filename}}</p><button class="btn ghost" :disabled="busy || !file" @click="verify">{{busy?'处理中…':'上传并校验签名'}}</button><progress v-if="busy" :value="progress" max="100"></progress><p role="status" class="muted">{{message}}</p><div v-if="verified" class="update-review"><div class="field" v-if="kind==='firmware'"><label>选择刷入槽位</label><select v-model="targetSlot" :disabled="busy" @change="ack=false"><option v-for="slot in verified.allowed_slots || [verified.slot]" :key="slot" :value="slot">{{slot.toUpperCase()}} 槽{{slot===currentSlot?'（当前运行槽）':'（非当前槽）'}}</option></select></div><dl class="kv"><div><dt>版本</dt><dd>{{verified.version}}</dd></div><div><dt>机型</dt><dd>{{verified.board}}</dd></div><div><dt>目标槽</dt><dd>{{targetSlot.toUpperCase()}}</dd></div><div><dt>签名</dt><dd>已验证 · SHA-256 全部一致</dd></div></dl><p>{{kind==='firmware'?'将进入内存升级环境，覆盖所选槽的 boot 和 rootfs，完成后从该槽启动。所选槽的系统与数据将被替换。':'将替换当前槽的 modem；蜂窝网络会中断，完成后重启。NV、校准和频段授权分区不写入。'}}</p><label class="home-check"><input type="checkbox" v-model="ack">我已备份并确认更新目标</label><button class="btn danger mt16" :disabled="busy || !ack" @click="flash">确认刷入并重启</button></div><pre v-if="job.output" class="at">{{job.output}}</pre></section>`,
  data(){return{kind:'firmware',file:null,filename:'',busy:false,progress:0,message:'',verified:null,ack:false,job:{},timer:0,uploadId:'',currentSlot:'',targetSlot:''};},async mounted(){try{const r=requireSuccess(await ubus('air6.cpe','update_status'));this.currentSlot=r.current_slot||'';}catch(e){this.message=e.message;}},beforeUnmount(){clearTimeout(this.timer);},
  methods:{select(k){this.kind=k;this.verified=null;this.ack=false;this.message='';},choose(e){this.file=e.target.files[0]||null;this.filename=this.file?.name||'';this.verified=null;this.ack=false;},async verify(){if(this.busy||!this.file)return;this.busy=true;this.verified=null;this.ack=false;this.progress=0;try{if(this.file.size>384*1024*1024)throw Error('更新包超过 384 MiB');const r=requireSuccess(await ubus('air6.cpe','update_begin',{kind:this.kind,size:this.file.size}));this.uploadId=r.id;this.job={};this.message='正在上传更新包…';if(r.upload_transport==='cgi'){store.timers.forEach(clearInterval);store.timers=[];try{await Promise.allSettled([...pendingRpc.values()]);store.timers.forEach(clearInterval);store.timers=[];await new Promise((resolve,reject)=>{const body=new FormData();body.append('sessionid',store.token);body.append('filename','/tmp/air6-update/upload.tar');body.append('filedata',this.file);const xhr=new XMLHttpRequest(),started=performance.now();xhr.open('POST','/cgi-bin/cgi-upload');xhr.timeout=180000;xhr.upload.onprogress=e=>{if(!e.lengthComputable)return;this.progress=Math.min(100,Math.round(e.loaded/e.total*100));const sent=Math.min(this.file.size,e.loaded),rate=sent/1048576/Math.max((performance.now()-started)/1000,.1);this.message=e.loaded===e.total?'传输完成，服务器正在检查上传文件…':'正在上传 '+(sent/1048576).toFixed(1)+' / '+(this.file.size/1048576).toFixed(1)+' MiB · '+this.progress+'% · '+rate.toFixed(1)+' MiB/s';};xhr.onload=()=>{try{const result=JSON.parse(xhr.responseText);if(xhr.status!==200 || result.error || result.size!==this.file.size)throw Error(result.message||result.error?.message||'上传结果或大小校验失败');this.progress=100;resolve();}catch(e){reject(e);}};xhr.onerror=()=>reject(Error('上传连接中断，请重新上传'));xhr.ontimeout=()=>reject(Error('上传超时，请重新上传'));xhr.send(body);});}finally{startTimers();}}else{const chunkSize=Math.min(262144,r.chunk_size||32768),started=performance.now();this.job={};this.message='正在上传更新包…';for(let offset=0;offset<this.file.size;offset+=chunkSize){const data=await new Promise((resolve,reject)=>{const reader=new FileReader();reader.onload=()=>resolve(reader.result.split(',')[1]);reader.onerror=()=>reject(Error('读取更新包失败'));reader.readAsDataURL(this.file.slice(offset,offset+chunkSize));});const end=Math.min(offset+chunkSize,this.file.size);const result=requireSuccess(await ubus('air6.cpe','update_chunk',{id:r.id,offset,data}));if(result.offset!==end)throw Error('上传偏移校验失败，请重新上传');this.progress=Math.round(end/this.file.size*100);const seconds=(performance.now()-started)/1000,rate=end/1048576/Math.max(seconds,.1);this.message='正在上传 '+(end/1048576).toFixed(1)+' / '+(this.file.size/1048576).toFixed(1)+' MiB · '+this.progress+'% · '+rate.toFixed(1)+' MiB/s';}}requireSuccess(await ubus('air6.cpe','update_verify',{id:r.id}));this.message='上传完成，正在校验签名、机型、槽位及镜像哈希…';this.poll();}catch(e){this.message='校验失败：'+e.message;this.busy=false;}},async poll(){try{const r=requireSuccess(await ubus('air6.cpe','update_status'));this.job=r;this.currentSlot=r.current_slot||this.currentSlot;if(r.running){this.timer=setTimeout(()=>this.poll(),1500);return;}this.busy=false;if(r.verified){this.verified=r.verified;this.targetSlot=r.verified.slot;this.currentSlot=r.verified.current_slot||this.currentSlot;this.message='签名及镜像校验通过，请核对目标后刷入。';}else this.message=r.error?'校验或更新失败：'+r.error:r.output||'更新任务结束';}catch(e){this.busy=false;this.message=e.message;}},async flash(){if(this.busy||!this.verified||!this.ack)return;if(!confirm('确认刷入已验证的'+(this.kind==='firmware'?'系统固件':'modem')+'到 '+this.targetSlot.toUpperCase()+' 槽并重启？'))return;this.busy=true;try{requireSuccess(await ubus('air6.cpe','update_flash',{id:this.uploadId,digest:this.verified.digest,target_slot:this.targetSlot}));this.verified=null;this.message='正在刷写并读回验证，请勿断电。设备随后会重启。';this.poll();}catch(e){this.busy=false;this.message=e.message;}}},
};

homeSources.updates=SignedUpdateView;
homeModules.push({id:'system:updates',page:'updates',label:'固件与 modem 更新',group:'系统',html:SignedUpdateView.template,w:12,h:6});
SystemView.components = {...SystemView.components,HomeSettings,SignedUpdateView};
/* ---------- shell ---------- */
const Shell = {
  components: { OverviewView, CellularView, WirelessView, ClientsView, SystemView, SmsView, TrafficView, SecurityView, ServicesView, InterfacesView, ToolsView, CustomHomeView, Icon },
  template: `
  <div class="app">
    <aside class="side">
      <div class="brand">
        <div class="logo">A6</div>
        <div><b>连接管理</b><small>SoftBank Air 6</small></div>
      </div>
      <div class="nav-label">设备工作台</div>
      <nav class="nav">
        <a v-for="n in nav" :key="n.id" :href="'#/' + n.id" :aria-current="view === n.id ? 'page' : null" :class="{active: view === n.id}" @click.prevent="go(n.id)">
          <icon :name="n.icon"></icon>{{ n.label }}
          <span v-if="n.id === 'sms' && unread" class="badge">{{ unread }}</span>
        </a>
      </nav>
      <div class="side-foot">
        <div class="row"><span class="dot" :class="{offline: stale}"></span>{{ stale ? '状态未更新' : '设备已连接' }}</div>
        <nav class="nav nav-sub">
          <a @click="toggleLayout"><icon :name="editing ? 'grid' : 'layout'"></icon>{{ editing ? '完成布局' : '自定义布局' }}</a>
          <a v-if="editing" @click="resetLayout"><icon name="reset"></icon>恢复默认布局</a>
          <a @click="logout"><icon name="logout"></icon>退出登录</a>
        </nav>
      </div>
    </aside>
    <main class="main">
      <header class="top">
        <div>
          <div class="eyebrow">YOUR CONNECTION, AT A GLANCE</div>
          <h1>{{ title }}</h1>
          <p>{{ subtitle }}</p>
        </div>
        <div class="top-actions"><div class="updated">{{ updated }}</div><button class="btn ghost refresh-state" :disabled="refreshing" @click="refreshNow">{{ refreshing ? '刷新中…' : '刷新状态' }}</button></div>
        <button class="btn ghost mobile-logout" @click="logout">退出登录</button>
      </header>
      <div v-if="stale" class="banner">状态刷新中断，请检查与路由器的连接。此前数据显示可能已过期。</div>
      <component :is="viewComp"></component>
    </main>
    <div class="toast" role="status" aria-live="polite" v-if="toastMsg">{{ toastMsg }}</div>
  </div>`,
  data() { return { now: Date.now(), refreshing: false }; },
  computed: {
    nav() { return homeState.modules.length ? [{id:'home',label:homeState.name,icon:'grid'},...NAV] : NAV; },
    view() { return store.view; },
    viewComp() {
      return { home:'CustomHomeView', overview: 'OverviewView', cellular: 'CellularView', wireless: 'WirelessView', clients: 'ClientsView', traffic: 'TrafficView', security: 'SecurityView', services: 'ServicesView', interfaces: 'InterfacesView', sms: 'SmsView', tools: 'ToolsView', system: 'SystemView' }[store.view] || 'OverviewView';
    },
    title() { if(store.view==='home')return homeState.name;return (TITLES[store.view] || TITLES.overview)[0]; },
    subtitle() { if(store.view==='home')return '自选功能模块 · 拖拽调整位置与大小';return (TITLES[store.view] || TITLES.overview)[1]; },
    stale() { return !!store.statusError || (!!store.statusUpdated && store.clock - store.statusUpdated > 30000); },
    updated() { return store.statusUpdated ? '状态更新于 ' + new Date(store.statusUpdated).toTimeString().slice(0, 8) : '等待状态更新…'; },
    toastMsg() { return store.toast; },
    unread() { return store.smsUnread; },
    editing() { return store.editing; },
  },
  mounted() {
    loadHome();
    this.clock = setInterval(() => { this.now = Date.now(); }, 1000);
  },
  beforeUnmount() { clearInterval(this.clock); },
  methods: {
    async refreshNow() {
      if(this.refreshing) return; this.refreshing=true;
      try {
        await Promise.allSettled([refreshStatus(), refreshAccel(), refreshSystem(), refreshCa()]);
        if(store.token) toast(store.statusError ? '状态读取失败，请检查设备连接' : '状态已刷新');
      } finally { this.refreshing=false; }
    },
    go(id) { store.view = id; location.hash = '#/' + id; },
    toggleLayout() { store.editing = !store.editing; },
    resetLayout() { store.resetTick++; },
    logout() { logout(); },
  },
};

/* ---------- app ---------- */
const App = {
  components: { LoginView, Shell },
  template: `<login-view v-if="!authed"></login-view><shell v-else></shell>`,
  computed: { authed() { return !!store.token; } },
  mounted() {
    window.addEventListener('hashchange', () => {
      const v = (location.hash || '#/overview').replace(/^#\//, '');
      if (TITLES[v]) store.view = v;
    });
    let theme='auto'; try { theme=localStorage.getItem('a6.theme') || 'auto'; } catch(e) {}
    applyAppearance(theme);
    const media=window.matchMedia('(prefers-color-scheme: dark)');
    this.themeListener=()=>{ let t='auto'; try { t=localStorage.getItem('a6.theme') || 'auto'; } catch(e) {} if(t==='auto') applyAppearance(t); };
    media.addEventListener('change',this.themeListener); this.themeMedia=media;
    this.visibilityListener=()=>{if(!document.hidden && store.token) Promise.allSettled([refreshStatus(),refreshAccel(),refreshSystem(),refreshCa()]);};
    document.addEventListener('visibilitychange',this.visibilityListener);
    if (store.token) boot();
  },
};

createApp(App).mount('#app');
