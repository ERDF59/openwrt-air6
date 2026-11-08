'use strict';
'require view';
'require rpc';
'require ui';
'require air6modem.common_v2 as common';
'require air6modem.unlock_v3 as unlock';

/* 频段与解锁：网络模式 + 频段集合 + 授权入口 */

const callStatus = rpc.declare({ object: 'luci.air6modem', method: 'get_status', expect: {} });
const runAt = rpc.declare({ object: 'luci.air6modem', method: 'run_at', params: [ 'command' ] });
const setRat = rpc.declare({ object: 'luci.air6modem', method: 'set_rat', params: [ 'mode' ] });

const LTE_FACTORY = [ 1, 8, 41, 42 ];
const LTE_FULL = [ 1, 3, 5, 7, 8, 20, 28, 38, 40, 41, 42, 43 ];
const NR_FACTORY = [ 3, 28, 77, 79 ];
const NR_FULL = [ 1, 3, 5, 7, 8, 20, 28, 38, 40, 41, 77, 78, 79 ];

const RAT_MODES = [ [ 21, '自动' ], [ 19, '4G + 5G' ], [ 15, '仅 5G' ] ];

let licState = { target: '', license: '' };
let currentBand = '';

function pill(kind, band, on, extended, cur) {
	let cls = 'a6-pill' + (on ? ' a6-pill-on' : '') + (extended ? ' a6-pill-ext' : '') + (cur ? ' a6-pill-cur' : '');
	let node = E('span', {
		'class': cls,
		title: (kind == 'LTE' ? 'B' : 'n') + band + (extended ? _('（扩展频段）') : _('（原厂频段）'))
	}, (kind == 'LTE' ? 'B' : 'n') + band);
	if (!on) node.addEventListener('click', () => unlock.open(() => this && this.refresh()));
	return node;
}

function bandGroup(kind, full, factory, unlocked, cur) {
	let curNum = String(cur || '').toUpperCase().replace(/^B|^N/, '');
	return E('div', { 'class': 'a6-pill-group' }, [
		E('span', { 'class': 'a6-pill-label' }, kind),
		E('div', { 'class': 'a6-pills' }, full.map(b => pill(kind, b, !factory.includes(b) ? unlocked : true,
			factory.indexOf(b) < 0, String(b) == curNum)))
	]);
}

return view.extend({
	load() { return callStatus(); },

	render(data) {
		let page = E('div', { 'class': 'a6-page' }, [
			common.style(),
			E('style', {}, `
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
.a6-mode-row{display:flex;gap:.5rem;flex-wrap:wrap}
`),
			E('div', { 'class': 'a6-head' }, [
				E('h2', { 'class': 'a6-title' }, [ E('span', { 'class': 'a6-logo' }, '📶'), _('频段与解锁') ])
			]),

			common.card(_('网络模式'), E('div', {}, [
				E('div', { 'class': 'a6-mode-row' }, RAT_MODES.map(m => E('button', {
					'class': 'a6-btn', 'data-mode': m[0],
					click: ui.createHandlerFn(this, 'handleRat', m[0])
				}, m[1]))),
				E('p', { 'class': 'a6-note' }, _('切换后模组会重新搜网，数据会短暂中断。'))
			])),

			common.card(_('频段集合'), E('div', {}, [
				E('div', { 'class': 'a6-band-state' }, [
					E('b', { id: 'a6-band-set' }, '…'),
					E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleUnlock') }, '🔓 ' + _('解锁频段'))
				]),
				E('div', { id: 'a6-bands' }),
				E('p', { 'class': 'a6-note', id: 'a6-band-note' }, _('读取中…'))
			])),

			common.card(_('当前服务小区'), common.kv([
				[ _('频段'), E('span', { id: 'a6-band' }, data.band || '-') ],
				[ 'PCI', E('span', { id: 'a6-pci' }, data.pci || '-') ],
				[ 'EARFCN', E('span', { id: 'a6-arfcn' }, data.arfcn || '-') ],
				[ 'TAC', E('span', { id: 'a6-tac' }, data.tac || '-') ]
			]))
		]);

		currentBand = data.band || '';
		window.setTimeout(() => this.refresh(), 0);
		return page;
	},

	handleRat(mode) {
		return setRat(mode).then(r => {
			let ok = r && (r.code === 0 || /success/i.test(r.output || ''));
			ui.addNotification(null, E('p', {}, ok ? _('网络模式已切换，正在重新搜网…') : _('切换失败：') + common.resultText(r)),
				ok ? 'info' : 'danger');
			return this.refresh();
		}).catch(e => common.callError(_('网络模式'), e));
	},

	handleUnlock() { unlock.open(() => this.refresh()); },

	refresh() {
		let node = document.getElementById('a6-band-set');
		return Promise.all([ callStatus(), runAt('AT+AIR6BAND?') ]).then(([ st, r ]) => {
			let text = r?.output || '';
			let tm = /TARGET=([A-Z0-9_]+)/.exec(text);
			let lm = /LICENSE=([A-Z]+)/.exec(text);
			licState.target = tm ? tm[1] : '';
			licState.license = lm ? lm[1] : '';
			currentBand = st?.band || currentBand;
			this.paintBands();
		}).catch(() => this.paintBands());
	},

	paintBands() {
		let unlocked = licState.target && licState.target !== 'FACTORY';
		let node = document.getElementById('a6-band-set');
		if (node) {
			node.textContent = unlocked ? _('全频段（已解锁）') : (licState.target ? _('原厂（未解锁）') : _('未知'));
			node.style.color = unlocked ? '#16a34a' : '';
		}
		let box = document.getElementById('a6-bands');
		if (!box) return;
		while (box.firstChild) box.removeChild(box.firstChild);
		box.appendChild(bandGroup('LTE', LTE_FULL, LTE_FACTORY, unlocked, currentBand));
		box.appendChild(bandGroup('NR', NR_FULL, NR_FACTORY, unlocked, currentBand));
		let note = document.getElementById('a6-band-note');
		if (note) note.textContent = unlocked
			? _('已解锁：全频段集合（EU25）可用。')
			: _('未解锁：只有原厂集合可用，灰色虚线胶囊为扩展频段，点击可去解锁。');
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
