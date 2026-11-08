'use strict';
'require view';
'require rpc';
'require poll';

/* 加速统计：HNAT PPE 绑定计数 + 各接口流量。 */

const callStats = rpc.declare({
	object: 'luci.air6modem',
	method: 'get_accel_stats',
	expect: { '': {} }
});

let lastSample = null;

function fmtBytes(value) {
	let n = +value || 0;
	let units = [ 'B', 'KiB', 'MiB', 'GiB', 'TiB' ];
	let i = 0;
	while (n >= 1024 && i < units.length - 1) {
		n /= 1024;
		i++;
	}
	return (i ? n.toFixed(1) : n) + ' ' + units[i];
}

function fmtCount(value) {
	let n = +value || 0;
	return n.toLocaleString('en-US');
}

function card(label, id, cls) {
	return E('div', { 'class': 'cbi-value' }, [
		E('div', { 'class': 'cbi-value-title' }, label),
		E('div', { 'class': 'cbi-value-field' },
			E('strong', { id: id, 'class': cls || '' }, '-'))
	]);
}

function rates(interfaces) {
	let now = Date.now() / 1000;
	let out = {};
	for (let it of interfaces)
		out[it.name] = { rx: +it.rx_bytes || 0, tx: +it.tx_bytes || 0 };

	let delta = {};
	if (lastSample && now > lastSample.time) {
		let dt = now - lastSample.time;
		for (let name in out) {
			let prev = lastSample.map[name];
			delta[name] = prev ? {
				rx: Math.max(0, (out[name].rx - prev.rx) / dt),
				tx: Math.max(0, (out[name].tx - prev.tx) / dt)
			} : { rx: 0, tx: 0 };
		}
	}
	lastSample = { time: now, map: out };
	return delta;
}

function update(root, data) {
	let hnat = data?.hnat || {};
	let interfaces = data?.interfaces || [];
	let delta = rates(interfaces);

	let set = (id, text, cls) => {
		let el = root.querySelector('#' + id);
		if (el) {
			el.textContent = text;
			if (cls != null)
				el.className = cls;
		}
	};

	if (hnat.available) {
		set('a6-hnat-state', '已加载', 'a6-ok');
		set('a6-ppe0', fmtCount(hnat.ppe0_bind));
		set('a6-ppe1', fmtCount(hnat.ppe1_bind));
		set('a6-injected', fmtCount(hnat.injected));
		set('a6-fallback', fmtCount(hnat.fallback));
	}
	else {
		set('a6-hnat-state', '未加载', 'a6-bad');
		set('a6-ppe0', '-');
		set('a6-ppe1', '-');
		set('a6-injected', '-');
		set('a6-fallback', '-');
	}

	let body = root.querySelector('#a6-iface-body');
	if (!body)
		return;
	body.replaceChildren();

	for (let it of interfaces.sort((a, b) => a.name.localeCompare(b.name))) {
		let d = delta[it.name] || { rx: 0, tx: 0 };
		body.appendChild(E('tr', {}, [
			E('td', {}, E('strong', {}, it.name)),
			E('td', { 'class': 'a6-rx' }, fmtBytes(d.rx) + '/s'),
			E('td', { 'class': 'a6-tx' }, fmtBytes(d.tx) + '/s'),
			E('td', {}, fmtBytes(it.rx_bytes)),
			E('td', {}, fmtBytes(it.tx_bytes)),
			E('td', {}, fmtCount((+it.rx_errors || 0) + (+it.tx_errors || 0)) +
				' / ' + fmtCount((+it.rx_dropped || 0) + (+it.tx_dropped || 0)))
		]));
	}
}

return view.extend({
	load() {
		return callStats();
	},

	render(data) {
		let root = E('div', { 'class': 'cbi-section', id: 'a6-accel' }, [
			E('h3', {}, '加速状态'),
			E('div', { 'class': 'cbi-section-node' }, [
				card('HNAT', 'a6-hnat-state'),
				card('PPE0 绑定条目', 'a6-ppe0'),
				card('PPE1 绑定条目', 'a6-ppe1'),
				card('CPU→Wi-Fi 注入', 'a6-injected'),
				card('注入回退', 'a6-fallback')
			]),
			E('p', { 'class': 'cbi-section-descr' },
				'厂商 HNAT 独占 PPE；nft flowtable 在 HNAT 加载时由 fw4 自动禁用。'),
			E('h3', {}, '接口流量'),
			E('table', { 'class': 'table' }, [
				E('thead', {}, E('tr', {}, [
					E('th', {}, '接口'),
					E('th', {}, '接收速率'),
					E('th', {}, '发送速率'),
					E('th', {}, '接收总量'),
					E('th', {}, '发送总量'),
					E('th', {}, '错误 / 丢包')
				])),
				E('tbody', { id: 'a6-iface-body' })
			])
		]);

		update(root, data);

		poll.add(() => callStats().then(d => update(root, d)), 3);

		return root;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
