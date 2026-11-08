'use strict';
'require view';
'require rpc';
'require ui';
'require air6modem.common_v2 as common';

const getNetwork = rpc.declare({ object: 'luci.air6modem', method: 'get_network', expect: {} });
const setRadio = rpc.declare({ object: 'luci.air6modem', method: 'set_radio', params: [ 'enabled' ] });
const setRat = rpc.declare({ object: 'luci.air6modem', method: 'set_rat', params: [ 'mode' ] });
const ifup = rpc.declare({ object: 'luci.air6modem', method: 'ifup' });
const ifdown = rpc.declare({ object: 'luci.air6modem', method: 'ifdown' });

function parse(data) {
	let status = {};
	try { status = JSON.parse(data?.status || '{}'); } catch (e) {}
	let config = data?.config || '';
	let value = key => {
		let m = config.match(new RegExp(`network\\.wan\\.${key}='([^']*)'`));
		return m ? m[1] : '-';
	};
	return {
		status,
		config,
		value,
		up: status.up === true,
		iface: status.l3_device || '-',
		address: status['ipv4-address']?.[0]?.address || '-',
		gateway: status.route?.find(r => r.mask == 0)?.nexthop || '-',
		dns: (status['dns-server'] || []).join(' / ') || '-',
		uptime: +status.uptime || 0,
		proto: status.proto || '-',
		apn: status.data?.nw_apn || value('apn')
	};
}

function stateView(data) {
	let n = parse(data);
	return E('div', {}, [
		E('div', { 'class': 'a6-hero-op', 'style': 'font-size:1.25rem' }, [
			E('span', { 'class': 'a6-badge ' + (n.up ? 'a6-badge-lte' : 'a6-badge-off') },
				n.up ? _('已连接') : _('未连接')),
			E('span', { 'class': 'a6-hero-code' }, n.iface === '-' ? '' : n.iface)
		]),
		E('div', { 'class': 'a6-tiles', 'style': 'margin-top:.9rem' }, [
			common.tile('IPv4', n.address),
			common.tile(_('网关'), n.gateway),
			common.tile(_('在线时长'), n.up ? common.formatUptime(n.uptime) : '-'),
			common.tile(_('协议'), n.proto)
		]),
		E('div', { 'style': 'margin-top:.9rem' }, common.kv([
			[ 'APN', n.apn ],
			[ 'DNS', n.dns ],
			[ _('IP 类型'), n.value('iptype') ],
			[ _('认证方式'), n.value('auth') ],
			[ _('MTU'), n.value('mtu') ]
		]))
	]);
}

function engineering(data) {
	let n = parse(data);
	return E('div', {}, [
		E('pre', { 'class': 'm5-raw' }, data?.status || '-'),
		E('p', { 'class': 'm5-note' }, n.config || '-')
	]);
}

return view.extend({
	load() {
		return getNetwork();
	},

	refresh() {
		return getNetwork().then(n => {
			let state = document.getElementById('a6-net-state');
			let eng = document.getElementById('a6-net-eng');
			if (state) state.replaceChildren(stateView(n));
			if (eng) eng.replaceChildren(engineering(n));
		});
	},

	handleCall(fn, value) {
		return fn(value)
			.then(r => {
				let out = document.getElementById('a6-net-result');
				if (out) out.textContent = common.resultText(r);
				return this.refresh();
			})
			.catch(e => common.callError(_('蜂窝拨号'), e));
	},

	render(data) {
		let n = parse(data);

		return E('div', { 'class': 'a6-page' }, [
			common.style(),

			E('div', { 'class': 'a6-head' }, [
				E('h2', { 'class': 'a6-title' }, [ E('span', { 'class': 'a6-logo' }, '⇅'), _('拨号与网络') ]),
				E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleRefresh') }, '↻ ' + _('刷新'))
			]),

			E('div', { 'class': 'a6-grid-wide' }, [
				common.card(_('连接状态'), E('div', { id: 'a6-net-state' }, stateView(data))),

				common.card(_('操作'), E('div', {}, [
					E('div', { 'style': 'display:flex;flex-wrap:wrap;gap:.6rem' }, [
						E('button', { 'class': 'a6-btn a6-btn-primary', click: ui.createHandlerFn(this, 'handleCall', ifup) },
							_('重新拨号')),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleCall', ifdown) },
							_('断开连接')),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleCall', setRadio, true) },
							_('开启射频')),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleCall', setRadio, false) },
							_('关闭射频'))
					]),
					E('p', { 'class': 'a6-note' }, _('关闭射频会立即断网；重新拨号等于 ifdown + ifup。')),
					E('h3', { 'style': 'margin:1.2rem 0 .6rem;font-size:.9rem;opacity:.7' }, _('首选网络制式')),
					E('div', { 'style': 'display:flex;gap:.6rem;flex-wrap:wrap;align-items:center' }, [
						E('select', { id: 'a6-rat-mode', 'class': 'cbi-input-select', 'style': 'padding:.45rem .6rem;border-radius:.6rem' }, [
							E('option', { value: 21 }, _('3G / 4G / 5G 自动')),
							E('option', { value: 19 }, _('4G / 5G 自动')),
							E('option', { value: 15 }, _('仅 5G')),
							E('option', { value: 3 }, _('仅 4G'))
						]),
						E('button', {
							'class': 'a6-btn a6-btn-primary',
							click: ui.createHandlerFn(this, function() {
								return this.handleCall(setRat, +document.getElementById('a6-rat-mode').value);
							})
						}, _('应用制式'))
					]),
					E('p', { 'class': 'a6-note' }, _('当前射频：%s · 当前制式：%s')
						.format(common.radioText(data.radio), common.ratText(data.rat)))
				]))
			]),

			common.card(_('操作结果'), E('pre', { id: 'a6-net-result', 'class': 'm5-raw' }, _('（尚未执行操作）'))),

			E('details', { 'class': 'a6-card' }, [
				E('summary', { 'style': 'cursor:pointer;font-weight:600' }, _('工程信息（接口状态与配置原文）')),
				E('div', { id: 'a6-net-eng', 'style': 'margin-top:.8rem' }, engineering(data))
			]),

			E('p', { 'class': 'a6-note' }, _('APN、IP 类型、认证、MTU、重试次数请到「网络 → 接口 → wan」修改，避免两个页面互相覆盖。'))
		]);
	},

	handleRefresh() {
		return this.refresh().then(() => ui.addNotification(null, E('p', {}, _('状态已刷新'))));
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
