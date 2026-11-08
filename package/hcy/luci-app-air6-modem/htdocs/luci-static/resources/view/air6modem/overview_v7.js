'use strict';
'require view';
'require rpc';
'require poll';
'require ui';
'require air6modem.common_v2 as common';
'require air6modem.unlock_v3 as unlock';

/* 5G 模组总览（参考 luci-app-5gmodem 的信息密度重排）
 *
 * 版式：状态条（运营商/注册/频段/RAT + 关键指标）
 *       → 网络模式 + 频段集合（原厂/全频段，胶囊分组）
 *       → 信号 / 拨号 / 小区 / SIM
 *       → 工程诊断
 */

const callStatus = rpc.declare({ object: 'luci.air6modem', method: 'get_status', expect: {} });
const callCells = rpc.declare({ object: 'luci.air6modem', method: 'get_cells', expect: {} });
const callDevices = rpc.declare({ object: 'luci-rpc', method: 'getNetworkDevices', expect: { '': {} } });
const runAt = rpc.declare({ object: 'luci.air6modem', method: 'run_at', params: [ 'command' ] });
const setRat = rpc.declare({ object: 'luci.air6modem', method: 'set_rat', params: [ 'mode' ] });
const ifup = rpc.declare({ object: 'luci.air6modem', method: 'ifup' });
const ifdown = rpc.declare({ object: 'luci.air6modem', method: 'ifdown' });

const RING_R = 52;
const RING_C = 2 * Math.PI * RING_R;

const LTE_FACTORY = [ 1, 8, 41, 42 ];
const LTE_FULL = [ 1, 3, 5, 7, 8, 20, 28, 38, 40, 41, 42, 43 ];
const NR_FACTORY = [ 3, 28, 77, 79 ];
const NR_FULL = [ 1, 3, 5, 7, 8, 20, 28, 38, 40, 41, 77, 78, 79 ];

const RAT_MODES = [
	[ 21, '自动' ],
	[ 19, '4G + 5G' ],
	[ 15, '仅 5G' ]
];

let lastSample = null;
let trafficDevice = null;
let licState = { target: '', license: '' };
let currentBand = '';

function netValue(n, key) {
	let a = n?.[key];
	return Array.isArray(a) ? a.map(x => x.address ? `${x.address}/${x.mask}` : String(x)).join(' ') : '';
}

function shortRat(rat) {
	let t = String(rat || '').toUpperCase();
	if (t.indexOf('5G') >= 0 || t.indexOf('NR') >= 0) return '5G';
	if (t.indexOf('LTE') >= 0 || t.indexOf('4G') >= 0) return '4G';
	return t || _('未知');
}

function ratClass(rat) {
	let t = String(rat || '').toUpperCase();
	if (t.indexOf('5G') >= 0 || t.indexOf('NR') >= 0) return 'a6-badge a6-badge-5g';
	if (t.indexOf('LTE') >= 0 || t.indexOf('4G') >= 0) return 'a6-badge a6-badge-lte';
	return 'a6-badge a6-badge-off';
}

function ring(rsrp) {
	let pct = common.signalPercent(rsrp);
	let q = common.signalQuality(rsrp);
	return E('div', { 'class': 'a6-ring', id: 'a6-ring' }, [
		E('svg', { viewBox: '0 0 120 120' }, [
			E('defs', {}, E('linearGradient', { id: 'a6-ring-grad', x1: '0', y1: '0', x2: '1', y2: '1' }, [
				E('stop', { offset: '0%', 'stop-color': '#2f7cf6' }),
				E('stop', { offset: '100%', 'stop-color': '#41d1a7' })
			])),
			E('circle', { 'class': 'a6-ring-track', cx: 60, cy: 60, r: RING_R }),
			E('circle', {
				'class': 'a6-ring-fill', cx: 60, cy: 60, r: RING_R,
				'stroke-dasharray': `${RING_C}`,
				'stroke-dashoffset': `${RING_C * (1 - pct / 100)}`,
				stroke: 'url(#a6-ring-grad)'
			})
		]),
		E('div', { 'class': 'a6-ring-face' }, [
			E('span', { 'class': 'a6-ring-val', id: 'a6-rsrp' }, rsrp == null || rsrp === '' ? '--' : rsrp),
			E('span', { 'class': 'a6-ring-unit' }, 'dBm'),
			E('span', { 'class': 'a6-ring-cap ' + (q.cls || ''), id: 'a6-signal-cap' }, q.text)
		])
	]);
}

function bar(id, label, cls) {
	return E('div', { 'class': 'a6-bar-row' }, [
		E('span', {}, label),
		E('div', { 'class': 'a6-bar-track' }, E('div', { 'class': 'a6-bar-fill ' + cls, id: `${id}-bar` })),
		E('span', { 'class': 'a6-bar-val', id: `${id}-val` }, '0.0 Mbps')
	]);
}

function simLabel(sim) { return common.simText(sim); }

function updateRing(rsrp) {
	let pct = common.signalPercent(rsrp);
	let fill = document.querySelector('#a6-ring .a6-ring-fill');
	if (fill) fill.setAttribute('stroke-dashoffset', `${RING_C * (1 - pct / 100)}`);
	let val = document.getElementById('a6-rsrp');
	if (val) val.textContent = rsrp == null || rsrp === '' ? '--' : rsrp;
	let cap = document.getElementById('a6-signal-cap');
	if (cap) { let q = common.signalQuality(rsrp); cap.textContent = q.text; cap.className = 'a6-ring-cap ' + (q.cls || ''); }
}

function barPercent(bps) {
	let mbps = Math.max(0, bps || 0) * 8 / 1000000;
	return Math.min(100, Math.log1p(mbps) / Math.log1p(1000) * 100);
}

function updateTraffic(devices) {
	let stat = devices?.[trafficDevice]?.stats;
	let now = Date.now() / 1000, rx = +stat?.rx_bytes, tx = +stat?.tx_bytes;
	if (!Number.isFinite(rx) || !Number.isFinite(tx)) return;
	let down = 0, up = 0;
	if (lastSample && now > lastSample.time) {
		down = Math.max(0, (rx - lastSample.rx) / (now - lastSample.time));
		up = Math.max(0, (tx - lastSample.tx) / (now - lastSample.time));
	}
	lastSample = { time: now, rx, tx };
	for (let item of [ [ 'a6-down', down ], [ 'a6-up', up ] ]) {
		let fill = document.getElementById(`${item[0]}-bar`);
		let val = document.getElementById(`${item[0]}-val`);
		if (fill) fill.style.width = `${barPercent(item[1])}%`;
		if (val) val.textContent = common.formatRate(item[1]);
	}
}

function setText(id, value) {
	let el = document.getElementById(id);
	if (el) el.textContent = (value == null || value === '') ? '-' : '' + value;
}

function setBadge(id, rat) {
	let el = document.getElementById(id);
	if (!el) return;
	el.className = ratClass(rat);
	el.textContent = shortRat(rat);
}

function signalOf(data, key) {
	let m = String(data.cesq || '').match(new RegExp(`${key}=(-?[0-9.]+)`));
	return m ? m[1] : '';
}

function dnsList(network) {
	return (network?.['dns-server'] || []).slice(0, 2).join(' / ');
}

function gateway(network) {
	let route = (network?.route || []).find(r => r.target == '0.0.0.0');
	return route?.nexthop || '';
}

/* 频段胶囊：原厂集合常亮；扩展集合需要授权，未授权时灰掉并可点击去解锁 */
function pill(kind, band, on, extended) {
	return E('span', {
		'class': 'a6-pill' + (on ? ' a6-pill-on' : '') + (extended ? ' a6-pill-ext' : ''),
		'data-kind': kind, 'data-band': band,
		title: (kind == 'LTE' ? 'B' : 'n') + band + (extended ? _('（扩展频段）') : _('（原厂频段）'))
	}, (kind == 'LTE' ? 'B' : 'n') + band);
}

function bandGroup(kind, full, factory, unlocked) {
	let cur = currentBand;
	return E('div', { 'class': 'a6-pill-group' }, [
		E('span', { 'class': 'a6-pill-label' }, kind),
		E('div', { 'class': 'a6-pills' }, full.map(b => {
			let ext = factory.indexOf(b) < 0;
			let on = !ext || unlocked;
			let isCur = String(cur).toUpperCase().replace(/^B|^N/, '') == String(b);
			let node = pill(kind, b, on, ext);
			if (isCur) node.classList.add('a6-pill-cur');
			if (!on) node.addEventListener('click', () => unlock.open());
			return node;
		}))
	]);
}

function renderBands() {
	let box = document.getElementById('a6-bands');
	if (!box) return;
	let unlocked = licState.target && licState.target !== 'FACTORY';
	while (box.firstChild) box.removeChild(box.firstChild);
	box.appendChild(bandGroup('LTE', LTE_FULL, LTE_FACTORY, unlocked));
	box.appendChild(bandGroup('NR', NR_FULL, NR_FACTORY, unlocked));

	let note = document.getElementById('a6-band-note');
	if (note) {
		note.textContent = unlocked
			? _('已解锁：全频段集合（EU25）可用。')
			: _('未解锁：只有原厂集合可用，灰色胶囊为扩展频段，点击“解锁频段”后开放。');
	}
}

function applyStatus(data) {
	if (!data || !data.available) return;

	let m = String(data.signal || '').match(/RSRP=(-?[0-9.]+)/);
	let rsrp = m ? m[1] : signalOf(data, 'RSRP');
	updateRing(rsrp);

	currentBand = data.band || '';
	setText('a6-op', common.operatorName(data.operator) || data.operator || _('未注册'));
	setText('a6-op-code', common.operatorName(data.operator) ? data.operator : '');
	setBadge('a6-rat', data.rat);
	setText('a6-reg', common.registrationText(data.registration));
	setText('a6-band-badge', data.band || '--');
	setText('a6-pci-badge', data.pci ? 'PCI ' + data.pci : '');
	setText('a6-live-note', data.initializing ? _('等待模组就绪…')
		: _('实时刷新 · %s 秒前').format(data.cache_age ?? '-'));

	setText('a6-t-rsrp', signalOf(data, 'RSRP'));
	setText('a6-t-rsrq', signalOf(data, 'RSRQ'));
	setText('a6-t-sinr', signalOf(data, 'SINR'));
	setText('a6-t-rssi', signalOf(data, 'RSSI'));

	for (let [ id, cls ] of [
		[ 'a6-t-rsrp', common.valueQuality(rsrp, -85, -100) ],
		[ 'a6-t-rsrq', common.valueQuality(signalOf(data, 'RSRQ'), -10, -15) ],
		[ 'a6-t-sinr', common.valueQuality(signalOf(data, 'SINR'), 15, 0) ]
	]) {
		let el = document.getElementById(id);
		if (el) el.className = cls;
	}

	let n = data.network || {};
	setText('a6-conn', n.up ? _('已连接') : _('未连接'));
	setText('a6-apn', n.data?.nw_apn);
	setText('a6-ipv4', netValue(n, 'ipv4-address'));
	setText('a6-gw', gateway(n));
	setText('a6-dns', dnsList(n));
	setText('a6-uptime', n.up ? common.formatUptime(n.uptime) : '');
	setText('a6-iface', n.l3_device);

	setText('a6-strip-ip', netValue(n, 'ipv4-address').split(' ')[0] || '-');
	setText('a6-strip-uptime', n.up ? common.formatUptime(n.uptime) : '-');
	setText('a6-strip-temp', data.temperature ? data.temperature + ' °C' : '-');
	setText('a6-strip-apn', n.data?.nw_apn || '-');

	setText('a6-band2', data.band);
	setText('a6-pci', data.pci);
	setText('a6-arfcn', data.arfcn);
	setText('a6-tac', data.tac);
	setText('a6-cellid', data.cellid);

	setText('a6-imei', data.imei);
	setText('a6-imsi', data.imsi);
	setText('a6-iccid', data.iccid);
	setText('a6-sim-state', simLabel(data.sim));
	setText('a6-temp', data.temperature ? `${data.temperature} °C` : '-');

	renderBands();

	let next = n.l3_device || null;
	if (next !== trafficDevice) { trafficDevice = next; lastSample = null; }
}

return view.extend({
	load() { return Promise.all([ callStatus(), callCells() ]); },

	update(data) { applyStatus(data); },

	render(result) {
		let data = result[0] || {}, cells = result[1] || {};
		let rsrp = signalOf(data, 'RSRP');
		let n = data.network || {};

		let page = E('div', { 'class': 'a6-page' }, [
			common.style(),
			E('style', {}, `
.a6-strip{display:grid;grid-template-columns:minmax(0,1.2fr) repeat(4,minmax(0,.7fr));gap:1rem;align-items:center;
  padding:1rem 1.25rem;border-radius:1rem;border:1px solid color-mix(in srgb,currentColor 12%,transparent);
  background:color-mix(in srgb,currentColor 4%,transparent)}
@media(max-width:900px){.a6-strip{grid-template-columns:1fr 1fr}}
.a6-strip-main{display:flex;align-items:center;gap:.9rem}
.a6-strip-op b{font-size:1.05rem}
.a6-strip-op small{display:block;opacity:.6;font-size:.78rem;margin-top:.15rem}
.a6-strip-kv small{display:block;opacity:.6;font-size:.74rem;margin-bottom:.15rem}
.a6-strip-kv b{font-size:.95rem;font-variant-numeric:tabular-nums;word-break:break-all}
.a6-bandline{display:flex;align-items:center;gap:.5rem;margin-top:.35rem;flex-wrap:wrap}
.a6-band-badge{display:inline-flex;align-items:center;padding:.15rem .55rem;border-radius:999px;font-weight:700;font-size:.82rem;
  background:color-mix(in srgb,#2f7cf6 20%,transparent);color:#2f7cf6}
.a6-mode-row{display:flex;gap:.5rem;flex-wrap:wrap;margin:.2rem 0 .8rem}
.a6-mode{flex:0 0 auto}
.a6-pill-group{display:grid;grid-template-columns:3.2rem minmax(0,1fr);gap:.6rem;align-items:start;margin:.55rem 0}
.a6-pill-label{font-size:.8rem;opacity:.65;padding-top:.25rem}
.a6-pills{display:flex;flex-wrap:wrap;gap:.4rem}
.a6-pill{display:inline-flex;align-items:center;justify-content:center;min-width:3rem;padding:.28rem .55rem;border-radius:.55rem;
  font-size:.8rem;font-weight:600;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  color:color-mix(in srgb,currentColor 45%,transparent);cursor:default}
.a6-pill-on{background:linear-gradient(135deg,#2f7cf6,#3b82f6);border-color:transparent;color:#fff}
.a6-pill-cur{box-shadow:0 0 0 2px color-mix(in srgb,#41d1a7 70%,transparent)}
.a6-pill-ext{border-style:dashed}
.a6-pill-ext:not(.a6-pill-on){cursor:pointer}
.a6-band-state{display:flex;align-items:center;gap:.8rem;flex-wrap:wrap;margin-bottom:.4rem}
.a6-band-state b{font-weight:700}
`),

			/* 状态条 */
			E('div', { 'class': 'a6-strip' }, [
				E('div', { 'class': 'a6-strip-main' }, [
					ring(rsrp),
					E('div', { 'class': 'a6-strip-op' }, [
						E('b', { id: 'a6-op' }, common.operatorName(data.operator) || data.operator || _('未注册')),
						E('small', { id: 'a6-reg' }, common.registrationText(data.registration)),
						E('div', { 'class': 'a6-bandline' }, [
							E('span', { 'class': ratClass(data.rat), id: 'a6-rat' }, shortRat(data.rat)),
							E('span', { 'class': 'a6-band-badge', id: 'a6-band-badge' }, data.band || '--'),
							E('span', { 'class': 'a6-hero-code', id: 'a6-pci-badge' }, data.pci ? 'PCI ' + data.pci : '')
						])
					])
				]),
				E('div', { 'class': 'a6-strip-kv' }, [ E('small', {}, 'IPv4'), E('b', { id: 'a6-strip-ip' }, '-') ]),
				E('div', { 'class': 'a6-strip-kv' }, [ E('small', {}, _('在线时长')), E('b', { id: 'a6-strip-uptime' }, '-') ]),
				E('div', { 'class': 'a6-strip-kv' }, [ E('small', {}, _('模组温度')), E('b', { id: 'a6-strip-temp' }, '-') ]),
				E('div', { 'class': 'a6-strip-kv' }, [ E('small', {}, 'APN'), E('b', { id: 'a6-strip-apn' }, '-') ])
			]),

			/* 网络模式 + 频段集合 */
			E('div', { 'class': 'a6-grid', 'style': 'margin-top:1rem' }, [
				common.card(_('网络模式'), E('div', {}, [
					E('div', { 'class': 'a6-mode-row' }, RAT_MODES.map(m => E('button', {
						'class': 'a6-btn a6-mode', 'data-mode': m[0],
						click: ui.createHandlerFn(this, 'handleRat', m[0])
					}, m[1]))),
					E('p', { 'class': 'a6-note' }, _('切换后模组会重新搜网，期间数据会短暂中断。'))
				])),

				common.card(_('频段集合'), E('div', {}, [
					E('div', { 'class': 'a6-band-state' }, [
						E('b', { id: 'a6-band-set' }, '…'),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleUnlock') }, '🔓 ' + _('解锁频段'))
					]),
					E('div', { id: 'a6-bands' }),
					E('p', { 'class': 'a6-note', id: 'a6-band-note' }, _('读取中…'))
				]))
			]),

			/* 信号 + 速率 */
			E('div', { 'class': 'a6-grid-wide', 'style': 'margin-top:1rem' }, [
				common.card(_('信号与速率'), E('div', {}, [
					E('div', { 'class': 'a6-tiles' }, [
						common.tile('RSRP', signalOf(data, 'RSRP'), 'dBm', 'a6-t-rsrp'),
						common.tile('RSRQ', signalOf(data, 'RSRQ'), 'dB', 'a6-t-rsrq'),
						common.tile('SINR', signalOf(data, 'SINR'), 'dB', 'a6-t-sinr'),
						common.tile('RSSI', signalOf(data, 'RSSI'), 'dBm', 'a6-t-rssi')
					]),
					E('div', { 'class': 'a6-bars', 'style': 'margin-top:.8rem' }, [
						bar('a6-down', _('下行'), 'a6-down'),
						bar('a6-up', _('上行'), 'a6-up')
					])
				])),
				common.card(_('连接操作'), E('div', {}, [
					E('div', { 'class': 'a6-mode-row' }, [
						E('button', { 'class': 'a6-btn a6-btn-primary', click: ui.createHandlerFn(this, 'handleReconnect') }, '↻ ' + _('重新拨号')),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleRefresh') }, '⟳ ' + _('刷新状态'))
					]),
					E('p', { 'class': 'a6-note', id: 'a6-live-note' }, _('实时刷新'))
				]))
			]),

			/* 拨号 / 小区 / SIM */
			E('div', { 'class': 'a6-grid', 'style': 'margin-top:1rem' }, [
				common.card(_('拨号与网络'), common.kv([
					[ _('连接状态'), E('span', { id: 'a6-conn' }, n.up ? _('已连接') : _('未连接')) ],
					[ 'APN', E('span', { id: 'a6-apn' }, n.data?.nw_apn || '-') ],
					[ 'IPv4', E('span', { id: 'a6-ipv4' }, netValue(n, 'ipv4-address')) ],
					[ _('网关'), E('span', { id: 'a6-gw' }, gateway(n)) ],
					[ 'DNS', E('span', { id: 'a6-dns' }, dnsList(n)) ],
					[ _('在线时长'), E('span', { id: 'a6-uptime' }, n.up ? common.formatUptime(n.uptime) : '-') ],
					[ _('数据接口'), E('span', { id: 'a6-iface' }, n.l3_device) ]
				])),

				common.card(_('小区信息'), common.kv([
					[ _('频段'), E('span', { id: 'a6-band2' }, data.band) ],
					[ 'PCI', E('span', { id: 'a6-pci' }, data.pci) ],
					[ 'EARFCN', E('span', { id: 'a6-arfcn' }, data.arfcn) ],
					[ 'TAC', E('span', { id: 'a6-tac' }, data.tac) ],
					[ _('小区 ID'), E('span', { id: 'a6-cellid' }, data.cellid) ]
				])),

				common.card(_('模组与 SIM'), common.kv([
					[ 'IMEI', E('span', { id: 'a6-imei' }, data.imei) ],
					[ 'IMSI', E('span', { id: 'a6-imsi' }, data.imsi) ],
					[ 'ICCID', E('span', { id: 'a6-iccid' }, data.iccid) ],
					[ _('SIM 状态'), E('span', { id: 'a6-sim-state' }, simLabel(data.sim)) ],
					[ _('模组温度'), E('span', { id: 'a6-temp' }, data.temperature ? `${data.temperature} °C` : '-') ]
				]))
			]),

			E('details', { 'class': 'a6-card', 'style': 'margin-top:1rem' }, [
				E('summary', { 'style': 'cursor:pointer;font-weight:600' }, _('工程诊断（AT+ECELL 原始返回）')),
				E('pre', { 'class': 'a6-raw a6-cell-raw' },
					common.resultText(cells.serving) + '\n\n' + common.resultText(cells.engineering))
			])
		]);

		trafficDevice = n.l3_device || null;
		poll.add(() => callStatus().then(applyStatus), 10);
		poll.add(() => callDevices().then(updateTraffic), 3);
		poll.add(() => callCells().then(c => {
			let raw = page.querySelector('.a6-cell-raw');
			if (raw) raw.textContent = common.resultText(c?.serving) + '\n\n' + common.resultText(c?.engineering);
		}), 60);

		requestAnimationFrame(() => applyStatus(data));
		requestAnimationFrame(() => this.refreshBands());

		return page;
	},

	handleRat(mode) {
		return setRat(mode).then(r => {
			let ok = r && (r.code === 0 || /success/i.test(r.output || ''));
			ui.addNotification(null,
				E('p', {}, ok ? _('网络模式已切换，正在重新搜网…') : _('切换失败：') + common.resultText(r)),
				ok ? 'info' : 'danger');
			return callStatus().then(applyStatus);
		}).catch(e => common.callError(_('网络模式'), e));
	},

	handleReconnect() {
		return ifdown().then(() => ifup()).then(() => {
			ui.addNotification(null, E('p', {}, _('已重新拨号')), 'info');
			return callStatus().then(applyStatus);
		}).catch(e => common.callError(_('重新拨号'), e));
	},

	handleRefresh() {
		return Promise.all([ callStatus(), callCells() ]).then(r => {
			applyStatus(r[0]);
			let raw = document.querySelector('.a6-cell-raw');
			if (raw) raw.textContent = common.resultText(r[1]?.serving) + '\n\n' + common.resultText(r[1]?.engineering);
			this.refreshBands();
			ui.addNotification(null, E('p', {}, _('状态已刷新')));
		});
	},

	handleUnlock() {
		unlock.open(() => this.refreshBands());
	},

	refreshBands() {
		let node = document.getElementById('a6-band-set');
		let bands = document.getElementById('a6-bands');
		if (bands) { while (bands.firstChild) bands.removeChild(bands.firstChild); }

		return runAt('AT+AIR6BAND?').then(r => {
			let text = (r?.output || '');
			let m = /TARGET=([A-Z0-9_]+)/.exec(text);
			let l = /LICENSE=([A-Z]+)/.exec(text);
			licState.target = m ? m[1] : '';
			licState.license = l ? l[1] : '';

			let unlocked = licState.target && licState.target !== 'FACTORY';
			if (node) {
				node.textContent = unlocked ? _('全频段（已解锁）') : (licState.target ? _('原厂（未解锁）') : _('未知'));
				node.style.color = unlocked ? '#16a34a' : '';
				node.title = (licState.license ? 'LICENSE=' + licState.license : '') + (licState.target ? ' TARGET=' + licState.target : '');
			}
			renderBands();
		}).catch(() => {
			if (node) node.textContent = _('未知');
			renderBands();
		});
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});



