'use strict';
'require view';
'require rpc';
'require ui';
'require air6modem.common_v2 as common';

const listSms = rpc.declare({ object: 'luci.air6modem', method: 'list_sms', expect: {} });
const getSmsc = rpc.declare({ object: 'luci.air6modem', method: 'get_smsc', expect: {} });
const sendSms = rpc.declare({ object: 'luci.air6modem', method: 'send_sms', params: [ 'number', 'text' ] });
const deleteSms = rpc.declare({ object: 'luci.air6modem', method: 'delete_sms', params: [ 'index' ] });
const setSmsc = rpc.declare({ object: 'luci.air6modem', method: 'set_smsc', params: [ 'number' ] });

/* atcid 用 UCS2 返回：每 4 个十六进制字符一个码位 */
function decodeUcs2(hex) {
	if (!hex) return '';
	if (!/^[0-9A-Fa-f]+$/.test(hex) || hex.length < 4) return hex;
	let out = '';
	for (let i = 0; i + 4 <= hex.length; i += 4)
		out += String.fromCharCode(parseInt(hex.slice(i, i + 4), 16));
	return out;
}

function stateText(state) {
	const map = {
		'REC UNREAD': _('未读'), 'REC READ': _('已读'),
		'STO SENT': _('已发送'), 'STO UNSENT': _('待发送'),
		'ALL': _('全部')
	};
	return map[String(state || '').toUpperCase()] || state || '-';
}

/* +CMGL: <idx>,"<stat>","<sender>","<alpha>","<time>" \r\n <UCS2 hex> */
function parseMessages(raw) {
	const re = /\+CMGL:\s*(\d+),"([^"]*)","([^"]*)","[^"]*","([^"]*)"\s*\r?\n([0-9A-Fa-f]+)/g;
	const records = [];
	let m;
	while ((m = re.exec(raw || '')) !== null) {
		let hex = m[5];
		let part8 = hex.match(/^050003([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})/i);
		let part16 = hex.match(/^060804([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})/i);
		let part = part16 || part8;
		let rec = {
			index: +m[1], state: m[2], sender: m[3], time: m[4],
			text: '', ref: '', total: 1, part: 1
		};
		if (part) {
			rec.ref = part[1].toUpperCase();
			rec.total = parseInt(part[2], 16);
			rec.part = parseInt(part[3], 16);
			hex = hex.slice(part16 ? 14 : 12);
		}
		rec.text = decodeUcs2(hex);
		records.push(rec);
	}
	return records;
}

/* 把同一条长短信的多个分片拼起来 */
function groupMessages(records) {
	const groups = {};
	for (let rec of records) {
		let key = rec.ref ? `${rec.sender}|${rec.time}|${rec.ref}` : `idx:${rec.index}`;
		(groups[key] = groups[key] || []).push(rec);
	}
	return Object.values(groups).map(parts => {
		parts.sort((a, b) => a.part - b.part);
		let first = parts[0];
		return {
			indexes: parts.map(p => p.index).join(', '),
			state: first.state,
			sender: first.sender,
			time: first.time,
			text: parts.map(p => p.text).join(''),
			incomplete: first.total > parts.length ? `${parts.length}/${first.total}` : ''
		};
	}).sort((a, b) => +b.indexes.split(',')[0] - +a.indexes.split(',')[0]);
}

const SMS_CSS = `
.a6-msg{padding:.8rem .9rem;border-radius:.8rem;border:1px solid color-mix(in srgb,currentColor 10%,transparent);
  background:color-mix(in srgb,currentColor 5%,transparent);margin-bottom:.6rem}
.a6-msg-head{display:flex;flex-wrap:wrap;gap:.5rem;align-items:center;font-size:.82rem}
.a6-msg-head b{font-size:.95rem}
.a6-msg-badge{padding:.05rem .45rem;border-radius:999px;background:color-mix(in srgb,#2f7cf6 18%,transparent);color:#2f7cf6;font-size:.72rem}
.a6-msg-time,.a6-msg-idx{opacity:.55;font-size:.75rem}
.a6-msg-idx{margin-left:auto}
.a6-msg-body{margin-top:.45rem;font-size:.9rem;line-height:1.5;white-space:pre-wrap;word-break:break-word}
.a6-empty{padding:1.4rem;text-align:center;opacity:.55;font-size:.88rem}
.a6-form-row{display:grid;grid-template-columns:6rem minmax(0,1fr);gap:.55rem 1rem;align-items:center;max-width:40rem}
.a6-form-row label{opacity:.7;font-size:.85rem}
.a6-form-row input,.a6-form-row textarea{padding:.5rem .65rem;border-radius:.55rem;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  background:color-mix(in srgb,currentColor 5%,transparent);color:inherit;width:100%}
`;

function messageList(data) {
	let messages = groupMessages(parseMessages(data?.output || ''));
	if (!messages.length)
		return E('div', { 'class': 'a6-empty' }, _('暂无短信（或当前 SIM 没有存储的消息）'));

	return E('div', {}, messages.map(msg => E('div', { 'class': 'a6-msg' }, [
		E('div', { 'class': 'a6-msg-head' }, [
			E('b', {}, msg.sender || _('未知号码')),
			E('span', { 'class': 'a6-msg-badge' }, stateText(msg.state)),
			E('span', { 'class': 'a6-msg-time' }, msg.time || ''),
			msg.incomplete ? E('span', { 'class': 'a6-msg-time' }, _('不完整 %s').format(msg.incomplete)) : '',
			E('span', { 'class': 'a6-msg-idx' }, _('序号 %s').format(msg.indexes))
		]),
		E('div', { 'class': 'a6-msg-body' }, msg.text || '')
	])));
}

function smscOf(result) {
	let m = common.resultText(result).match(/\+CSCA:\s*"([^"]+)"/);
	return m ? m[1] : '';
}

return view.extend({
	load() {
		return listSms().then(list => getSmsc().then(smsc => [ list, smsc ]));
	},

	refresh() {
		return listSms().then(list => {
			let box = document.getElementById('a6-sms-list');
			if (box) box.replaceChildren(messageList(list));
		});
	},

	showResult(r) {
		let out = document.getElementById('a6-sms-result');
		if (out) out.textContent = common.resultText(r);
	},

	handleRefresh() {
		return this.refresh().then(() => ui.addNotification(null, E('p', {}, _('短信已刷新'))));
	},

	handleSend() {
		let number = document.getElementById('a6-sms-number').value.trim();
		let text = document.getElementById('a6-sms-text').value;
		return sendSms(number, text).then(r => {
			this.showResult(r);
			if (r.code) throw new Error(common.resultText(r));
			ui.addNotification(null, E('p', {}, _('短信已提交给模组')));
		}).catch(e => common.callError(_('发送短信'), e));
	},

	handleDelete() {
		let index = +document.getElementById('a6-sms-index').value;
		return deleteSms(index).then(r => { this.showResult(r); return this.refresh(); })
			.catch(e => common.callError(_('删除短信'), e));
	},

	handleSetSmsc() {
		let number = document.getElementById('a6-smsc').value.trim();
		return setSmsc(number).then(r => this.showResult(r))
			.catch(e => common.callError(_('设置短信中心'), e));
	},

	render(result) {
		let list = result[0] || {}, smsc = result[1] || {};

		return E('div', { 'class': 'a6-page' }, [
			common.style(),
			E('style', {}, SMS_CSS),

			E('div', { 'class': 'a6-head' }, [
				E('h2', { 'class': 'a6-title' }, [ E('span', { 'class': 'a6-logo' }, '✉'), _('短信') ]),
				E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleRefresh') }, '↻ ' + _('刷新'))
			]),
			/* 应用内导航：让五个页面看起来是同一个应用 */
			E('style', {}, '.a6-tabs{display:flex;gap:.3rem;flex-wrap:wrap;margin:.1rem 0 1rem;padding:.3rem;border-radius:.85rem;' +
				'background:color-mix(in srgb,currentColor 5%,transparent);border:1px solid color-mix(in srgb,currentColor 10%,transparent);width:fit-content}' +
				'.a6-tab{padding:.38rem .85rem;border-radius:.6rem;font-size:.84rem;font-weight:600;color:inherit;opacity:.62;text-decoration:none}' +
				'.a6-tab:hover{opacity:1;background:color-mix(in srgb,currentColor 9%,transparent)}' +
				'.a6-tab-on{opacity:1;color:#fff;background:linear-gradient(135deg,#2f7cf6,#3b82f6)}'),
			(() => {
				let items = [ [ 'overview', _('概览') ], [ 'bands', _('频段') ], [ 'network', _('网络') ],
					[ 'sms', _('短信') ], [ 'terminal', _('调试') ] ];
				let base = location.pathname.replace(/\/+$/, '').split('/');
				return E('div', { 'class': 'a6-tabs' }, items.map(function(it) {
					let path = base.slice();
					path[path.length - 1] = it[0];
					return E('a', { 'class': 'a6-tab' + (it[0] == 'sms' ? ' a6-tab-on' : ''), href: path.join('/') }, it[1]);
				}));
			})(),

			common.card(_('收件箱'), E('div', { id: 'a6-sms-list' }, messageList(list))),

			common.card(_('发送短信'), E('div', {}, [
				E('div', { 'class': 'a6-form-row' }, [
					E('label', {}, _('接收号码')),
					E('input', { id: 'a6-sms-number', type: 'tel', placeholder: '+8613800138000' }),
					E('label', {}, _('内容')),
					E('textarea', { id: 'a6-sms-text', rows: 4, maxlength: 160, placeholder: _('最多 160 个 GSM/ASCII 字符') })
				]),
				E('div', { 'style': 'margin-top:.8rem' }, [
					E('button', { 'class': 'a6-btn a6-btn-primary', click: ui.createHandlerFn(this, 'handleSend') }, _('发送'))
				]),
				E('p', { 'class': 'a6-note' }, _('通过厂商 libqlril 的短信接口提交；中文短信暂不支持，请用 ASCII。'))
			])),

			common.card(_('删除与短信中心'), E('div', {}, [
				E('div', { 'class': 'a6-form-row' }, [
					E('label', {}, _('消息序号')),
					E('div', { 'style': 'display:flex;gap:.5rem' }, [
						E('input', { id: 'a6-sms-index', type: 'number', min: 0, 'style': 'max-width:8rem' }),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleDelete') }, _('删除'))
					])
				]),
				E('div', { 'class': 'a6-form-row', 'style': 'margin-top:.6rem' }, [
					E('label', {}, _('短信中心')),
					E('div', { 'style': 'display:flex;gap:.5rem' }, [
						E('input', { id: 'a6-smsc', value: smscOf(smsc) }),
						E('button', { 'class': 'a6-btn', click: ui.createHandlerFn(this, 'handleSetSmsc') }, _('设置'))
					])
				])
			])),

			common.card(_('操作结果'), E('pre', { id: 'a6-sms-result', 'class': 'a6-raw' }, _('（尚未执行操作）')))
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});

