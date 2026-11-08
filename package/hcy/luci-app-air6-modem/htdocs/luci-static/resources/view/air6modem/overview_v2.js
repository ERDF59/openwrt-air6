'use strict';
'require view';
'require rpc';
'require poll';
'require ui';
'require air6modem.common_v2 as common';
'require air6modem.unlock as unlock';

const callStatus = rpc.declare({ object: 'luci.air6modem', method: 'get_status', expect: {} });
const callCells = rpc.declare({ object: 'luci.air6modem', method: 'get_cells', expect: {} });
const runAt = rpc.declare({ object: 'luci.air6modem', method: 'run_at', params: [ 'command' ] });
const callDevices = rpc.declare({ object: 'luci-rpc', method: 'getNetworkDevices', expect: { '': {} } });

const RING_R = 52;
const RING_C = 2 * Math.PI * RING_R;

let lastSample = null;
let trafficDevice = null;

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

function chip(label, id, unit, value) {
	return E('span', { 'class': 'a6-chip' }, [
		E('span', {}, label),
		E('b', { id }, value == null || value === '' ? '-' : value),
		unit ? E('span', {}, unit) : ''
	]);
}

function simLabel(sim) {
	return common.simText(sim);
}

function updateRing(rsrp) {
	let pct = common.signalPercent(rsrp);
	let fill = document.querySelector('#a6-ring .a6-ring-fill');
	if (fill) fill.setAttribute('stroke-dashoffset', `${RING_C * (1 - pct / 100)}`);
	let val = document.getElementById('a6-rsrp');
	if (val) val.textContent = rsrp == null || rsrp === '' ? '--' : rsrp;
	let cap = document.getElementById('a6-signal-cap');
	if (cap) {
		let q = common.signalQuality(rsrp);
		cap.textContent = q.text;
		cap.className = 'a6-ring-cap ' + (q.cls || '');
	}
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

function ratClass(rat) {
	let t = String(rat || '').toUpperCase();
	if (t.indexOf('5G') >= 0 || t.indexOf('NR') >= 0) return 'a6-badge a6-badge-5g';
	if (t.indexOf('LTE') >= 0 || t.indexOf('4G') >= 0) return 'a6-badge a6-badge-lte';
	return 'a6-badge a6-badge-off';
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

function applyStatus(data) {
	if (!data || !data.available) return;

	let m = String(data.signal || '').match(/RSRP=(-?[0-9.]+)/);
	let rsrp = m ? m[1] : signalOf(data, 'RSRP');
	updateRing(rsrp);

	setText('a6-op', common.operatorName(data.operator) || data.operator || _('未注册'));
	setText('a6-op-code', common.operatorName(data.operator) ? data.operator : '');
	setBadge('a6-rat', data.rat);
	setText('a6-reg', common.registrationText(data.registration));
	setText('a6-band', data.band ? `${data.band} · PCI ${data.pci || '-'}` : '');
	setText('a6-live-note', data.initializing ? _('正在等待模组就绪…')
		: _('实时刷新 · 状态 %s 秒前').format(data.cache_age ?? '-'));
	setText('a6-sim-state', simLabel(data.sim));

	setText('a6-t-rsrp', signalOf(data, 'RSRP'));
	setText('a6-t-rsrq', signalOf(data, 'RSRQ'));
	setText('a6-t-sinr', signalOf(data, 'SINR'));
	setText('a6-t-rssi', signalOf(data, 'RSSI'));

	let color = [
		[ 'a6-t-rsrp', common.valueQuality(rsrp, -85, -100) ],
		[ 'a6-t-rsrq', common.valueQuality(signalOf(data, 'RSRQ'), -10, -15) ],
		[ 'a6-t-sinr', common.valueQuality(signalOf(data, 'SINR'), 15, 0) ]
	];
	for (let [ id, cls ] of color) {
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

	setText('a6-band2', data.band);
	setText('a6-pci', data.pci);
	setText('a6-arfcn', data.arfcn);
	setText('a6-tac', data.tac);
	setText('a6-cellid', data.cellid);

	setText('a6-imei', data.imei);
	setText('a6-imsi', data.imsi);
	setText('a6-iccid', data.iccid);
	setText('a6-temp', data.temperature ? `${data.temperature} °C` : '');

	setText('a6-chip-uptime', n.up ? common.formatUptime(n.uptime) : '-');
	setText('a6-chip-temp', data.temperature || '-');
	setText('a6-chip-sim', String(data.sim || '').indexOf('READY') >= 0 ? _('就绪') : (data.sim || '-'));
	setText('a6-chip-rsrp', rsrp);

	let next = n.l3_device || null;
	if (next !== trafficDevice) {
		trafficDevice = next;
		lastSample = null;
	}
}


return view.extend({
	load() { return Promise.all([ callStatus(), callCells() ]); },

	update(data) {
		applyStatus(data);
	},

	render(result) {
		let data = result[0] || {}, cells = result[1] || {};
		let rsrp = signalOf(data, 'RSRP');
		let n = data.network || {};

		let page = E('div', { 'class': 'a6-page' }, [
			common.style(),

			E('div', { 'class': 'a6-head' }, [
				E('h2', { 'class': 'a6-title' }, [ E('span', { 'class': 'a6-logo' }, '5G'), _('模组状态') ]),
				E('div', { 'style': 'display:flex;gap:.8rem;align-items:center' }, [
					E('span', { 'class': 'a6-live' }, [
						E('span', { 'class': 'a6-dot' }),
						E('span', { id: 'a6-live-note' }, _('实时刷新'))
					]),
					E('span', { 'class': 'a6-live', id: 'a6-band-set' }, _('频段集合') + ': …'),
					E('button', {
						'class': 'a6-btn',
						click: ui.createHandlerFn(this, 'handleUnlock')
					}, '🔓 ' + _('解锁频段')),
					E('button', {
						'class': 'a6-btn a6-btn-primary',
						click: ui.createHandlerFn(this, 'handleRefresh')
					}, '↻ ' + _('刷新'))
				])
			]),

			E('div', { 'class': 'a6-grid-wide' }, [
				common.card(null, E('div', {}, [
					E('div', { 'class': 'a6-hero-body' }, [
						ring(rsrp),
						E('div', { 'class': 'a6-hero-main' }, [
							E('div', { 'class': 'a6-hero-op' }, [
								E('span', { id: 'a6-op' }, common.operatorName(data.operator) || data.operator || _('未注册')),
								E('span', { 'class': 'a6-hero-code', id: 'a6-op-code' },
									common.operatorName(data.operator) ? data.operator : ''),
								E('span', { 'class': ratClass(data.rat), id: 'a6-rat' }, shortRat(data.rat))
							]),
							E('div', { 'class': 'a6-hero-sub', id: 'a6-reg' },
								common.registrationText(data.registration)),
							E('div', { 'class': 'a6-hero-sub', id: 'a6-band' },
								data.band ? `${data.band} · PCI ${data.pci || '-'}` : ''),
							E('div', { 'class': 'a6-bars' }, [
								bar('a6-down', _('下行'), 'a6-down'),
								bar('a6-up', _('上行'), 'a6-up')
							])
						])
					]),
					E('div', { 'class': 'a6-chips' }, [
						chip(_('在线时长'), 'a6-chip-uptime', '', n.up ? common.formatUptime(n.uptime) : '-'),
						chip(_('模组温度'), 'a6-chip-temp', '°C', data.temperature || '-'),
						chip('SIM', 'a6-chip-sim', '', simLabel(data.sim)),
						chip(_('信号'), 'a6-chip-rsrp', 'dBm', rsrp || '-')
					])
				]), 'a6-card-hero'),

				common.card(_('信号质量'), E('div', {}, [
					E('div', { 'class': 'a6-tiles' }, [
						common.tile('RSRP', signalOf(data, 'RSRP'), 'dBm', 'a6-t-rsrp'),
						common.tile('RSRQ', signalOf(data, 'RSRQ'), 'dB', 'a6-t-rsrq'),
						common.tile('SINR', signalOf(data, 'SINR'), 'dB', 'a6-t-sinr'),
						common.tile('RSSI', signalOf(data, 'RSSI'), 'dBm', 'a6-t-rssi')
					]),
					E('p', { 'class': 'a6-note' }, _('取 AT+CESQ 的标准字段；LTE 下无 SINR 时显示 “-”。'))
				]))
			]),

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
		requestAnimationFrame(() => this.refreshBandSet());
		return page;
	},

	handleRefresh() {
		return Promise.all([ callStatus(), callCells() ]).then(r => {
			applyStatus(r[0]);
			let raw = document.querySelector('.a6-cell-raw');
			if (raw)
				raw.textContent = common.resultText(r[1]?.serving) + '\n\n' + common.resultText(r[1]?.engineering);
			ui.addNotification(null, E('p', {}, _('状态已刷新')));
		});
	},

	handleUnlock() {
		unlock.open(() => this.refreshBandSet());
	},

	/* 未授权 = 原厂集合；已授权 = 全频段（EU25） */
	refreshBandSet() {
		let node = document.getElementById('a6-band-set');
		if (!node) return Promise.resolve();
		return runAt('AT+AIR6BAND?').then(r => {
			let text = (r?.output || '');
			let m = /TARGET=([A-Z0-9_]+)/.exec(text);
			let lic = /LICENSE=([A-Z]+)/.exec(text);
			if (!m) { node.textContent = _('频段集合') + ': ' + _('未知'); return; }
			let full = m[1] !== 'FACTORY';
			node.textContent = _('频段集合') + ': ' + (full ? _('全频段（已解锁）') : _('原厂（未解锁）'));
			node.style.opacity = full ? '1' : '.7';
			node.style.fontWeight = full ? '600' : '';
			node.title = (lic ? 'LICENSE=' + lic[1] + ', ' : '') + 'TARGET=' + m[1];
		}).catch(() => { node.textContent = _('频段集合') + ': ' + _('未知'); });
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});

