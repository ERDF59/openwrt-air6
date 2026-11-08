'use strict';
'require view';
'require rpc';
'require ui';
'require air6modem.common_v2 as common';

const runAt = rpc.declare({ object: 'luci.air6modem', method: 'run_at', params: [ 'command' ] });

const QUICK = [
	[ 'AT+CPIN?', _('SIM 状态') ],
	[ 'AT+CSQ', _('信号强度') ],
	[ 'AT+CESQ', _('信号质量') ],
	[ 'AT+CEREG?', _('注册状态') ],
	[ 'AT+COPS?', _('运营商') ],
	[ 'AT+CGSN', _('IMEI') ],
	[ 'AT+CCID', _('ICCID') ],
	[ 'AT+QTEMP', _('温度') ],
	[ 'AT+ECELL', _('小区信息') ]
];

const TERM_CSS = `
.a6-term{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:.6rem;align-items:center}
.a6-term input{padding:.55rem .7rem;border-radius:.6rem;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  background:color-mix(in srgb,currentColor 5%,transparent);color:inherit;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.9rem}
.a6-quick{display:flex;flex-wrap:wrap;gap:.45rem;margin-top:.8rem}
.a6-log{max-height:26rem;overflow:auto;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.8rem;
  padding:.9rem;border-radius:.7rem;background:color-mix(in srgb,currentColor 7%,transparent);white-space:pre-wrap;word-break:break-all;line-height:1.5}
.a6-log .a6-log-cmd{color:#2f7cf6;font-weight:600}
.a6-log .a6-log-time{opacity:.5}
.a6-warn{display:flex;gap:.6rem;padding:.85rem 1rem;border-radius:.8rem;font-size:.85rem;
  background:color-mix(in srgb,#f59e0b 14%,transparent);border:1px solid color-mix(in srgb,#f59e0b 30%,transparent)}
`;

function nowText() {
	let d = new Date();
	let p = n => (n < 10 ? '0' : '') + n;
	return `${p(d.getHours())}:${p(d.getMinutes())}:${p(d.getSeconds())}`;
}

return view.extend({
	log(lines) {
		let box = document.getElementById('a6-at-log');
		if (!box) return;
		let frag = document.createDocumentFragment();
		for (let line of lines) {
			if (!line.text) { frag.appendChild(document.createTextNode('\n')); continue; }
			let span = E('span', { 'class': line.cls || '' }, line.text);
			frag.appendChild(span);
			frag.appendChild(document.createTextNode('\n'));
		}
		box.appendChild(frag);
		box.scrollTop = box.scrollHeight;
	},

	run(command) {
		if (!command) return Promise.resolve();
		this.log([{ text: `[${nowText()}] `, cls: 'a6-log-time' }, { text: `> ${command}`, cls: 'a6-log-cmd' }]);
		return runAt(command).then(r => {
			this.log([{ text: common.resultText(r) }]);
		}).catch(e => {
			this.log([{ text: String(e?.message || e) }]);
			common.callError(_('AT 调试'), e);
		});
	},

	handleRun() {
		let input = document.getElementById('a6-at-command');
		let command = (input?.value || '').trim();
		return this.run(command);
	},

	handleQuick(ev) {
		let command = ev.currentTarget.getAttribute('data-cmd');
		let input = document.getElementById('a6-at-command');
		if (input) input.value = command;
		return this.run(command);
	},

	render() {
		return E('div', { 'class': 'a6-page' }, [
			common.style(),
			E('style', {}, TERM_CSS),

			E('div', { 'class': 'a6-head' }, [
				E('h2', { 'class': 'a6-title' }, [ E('span', { 'class': 'a6-logo' }, '>_'), _('AT 调试') ])
			]),

			E('div', { 'class': 'a6-warn' }, [
				E('span', {}, '⚠'),
				E('span', {}, _('AT 命令会直接改模组状态，可能中断注册、通话或数据业务。仅支持 AT 指令字符，不接受 shell 语法。'))
			]),

			common.card(_('发送命令'), E('div', {}, [
				E('div', { 'class': 'a6-term' }, [
					E('input', {
						id: 'a6-at-command', value: 'AT+CESQ', maxlength: 512,
						placeholder: 'AT+...',
						keydown: ev => { if (ev.key === 'Enter') this.handleRun(); }
					}),
					E('button', { 'class': 'a6-btn a6-btn-primary', click: ui.createHandlerFn(this, 'handleRun') }, _('执行'))
				]),
				E('div', { 'class': 'a6-quick' }, QUICK.map(item => E('button', {
					'class': 'a6-btn', 'data-cmd': item[0],
					click: ui.createHandlerFn(this, 'handleQuick')
				}, item[1])))
			])),

			common.card(_('输出'), E('pre', { id: 'a6-at-log', 'class': 'a6-log' }, _('就绪，输入命令或点上面的快捷按钮。')))
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
