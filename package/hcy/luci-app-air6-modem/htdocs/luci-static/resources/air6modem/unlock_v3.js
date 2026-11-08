'use strict';
'require baseclass';
'require rpc';
'require ui';
'require air6modem.common_v2 as common';

/* 解锁频段：首页按钮弹出的授权面板
 *
 *   AT+AIR6BANDID?  -> +AIR6BANDID: <64 hex>
 *   AT+AIR6BAND?    -> +AIR6BAND: LICENSE=…,RF=…,TARGET=…
 *   AT+AIR6BAND="A1.…" / AT+AIR6BAND=0
 */

const runAt = rpc.declare({ object: 'luci.air6modem', method: 'run_at', params: [ 'command' ] });
const TOKEN_RE = /^A1\.[A-Za-z0-9_-]{86}$/;

const CSS = `
.a6-unlock{min-width:min(46rem,72vw)}
.a6-unlock .a6-code{font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.86rem;line-height:1.55;
  word-break:break-all;padding:.6rem .75rem;border-radius:.6rem;
  background:color-mix(in srgb,currentColor 7%,transparent);border:1px solid color-mix(in srgb,currentColor 12%,transparent)}
.a6-unlock .a6-badges{display:flex;flex-wrap:wrap;gap:.45rem;margin:.6rem 0}
.a6-unlock .a6-badge{padding:.24rem .65rem;border-radius:999px;font-size:.78rem;font-weight:600;
  background:color-mix(in srgb,currentColor 10%,transparent)}
.a6-unlock .a6-badge-ok{background:color-mix(in srgb,#22c55e 18%,transparent);color:#16a34a}
.a6-unlock .a6-badge-off{background:color-mix(in srgb,#ef4444 16%,transparent);color:#dc2626}
.a6-unlock .a6-row{display:grid;grid-template-columns:minmax(0,1fr);gap:.5rem;margin-top:.5rem}
.a6-unlock .a6-row input{padding:.55rem .7rem;border-radius:.55rem;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  background:color-mix(in srgb,currentColor 5%,transparent);color:inherit;
  font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.82rem;width:100%}
.a6-unlock .a6-actions{display:flex;gap:.45rem;flex-wrap:wrap;margin-top:.55rem}
.a6-unlock .a6-out{margin-top:.55rem;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.76rem;
  padding:.55rem .7rem;border-radius:.5rem;background:color-mix(in srgb,currentColor 7%,transparent);white-space:pre-wrap}
.a6-unlock .a6-hint{font-size:.76rem;opacity:.65;margin:.45rem 0 0}
`;

function idOf(text) {
	let m = /\+AIR6BANDID:\s*([0-9a-fA-F]{64})/.exec(text || '');
	return m ? m[1] : '';
}

function stateOf(text) {
	let m = /\+AIR6BAND:\s*LICENSE=([A-Z]+)\s*,\s*RF=([A-Z]+)\s*,\s*TARGET=([A-Z0-9_]+)/.exec(text || '');
	return m ? { license: m[1], rf: m[2], target: m[3] } : null;
}

function query() {
	return Promise.all([ runAt('AT+AIR6BANDID?'), runAt('AT+AIR6BAND?') ]).then(([ id, st ]) => ({
		id: idOf(id?.output || ''),
		rawId: id?.output || '',
		state: stateOf(st?.output || ''),
		rawState: st?.output || ''
	}));
}

function normalize(value) {
	let token = (value || '').trim();
	return token.replace(/^["']|["']$/g, '');
}

function open(onChange) {
	let body = E('div', { 'class': 'a6-unlock' }, _('正在读取模组…'));
	ui.showModal(_('解锁频段'), [
		E('style', {}, CSS),
		body,
		E('div', { 'class': 'right' }, [
			E('button', { 'class': 'btn', click: ui.hideModal }, _('关闭'))
		])
	]);

	function paint(info, out) {
		while (body.firstChild) body.removeChild(body.firstChild);
		let st = info.state;
		let licensed = st && st.license === 'VALID';
		let supported = st != null;

		let box = E('div', {}, [
			E('p', {}, supported
				? _('本机指纹（交给发码端换授权码）：')
				: _('模组不支持 AIR6BAND 命令，请先刷入带授权功能的固件。')),
			E('div', { 'class': 'a6-code' }, info.id || info.rawId || '—'),
			E('div', { 'class': 'a6-badges' }, [
				E('span', { 'class': 'a6-badge ' + (licensed ? 'a6-badge-ok' : 'a6-badge-off') },
					'LICENSE=' + (st?.license || 'NONE')),
				E('span', { 'class': 'a6-badge ' + (st?.rf === 'READY' ? 'a6-badge-ok' : '') },
					'RF=' + (st?.rf || '—')),
				E('span', { 'class': 'a6-badge ' + (st?.target === 'EU25' ? 'a6-badge-ok' : '') },
					'TARGET=' + (st?.target || 'FACTORY'))
			]),
			E('div', { 'class': 'a6-row' }, [
				E('input', {
					id: 'a6-unlock-token', type: 'text', maxlength: 89,
					placeholder: _('A1.xxxxxxxx…（89 位授权码）'),
					keydown: ev => { if (ev.key === 'Enter') doApply(); }
				})
			]),
			E('div', { 'class': 'a6-actions' }, [
				E('button', { 'class': 'cbi-button cbi-button-apply', click: doApply }, _('确认解锁')),
				E('button', { 'class': 'btn', click: doRefresh }, _('刷新状态')),
				E('button', { 'class': 'btn', click: doClear }, _('清除授权'))
			]),
			E('div', { 'class': 'a6-out' }, out || (info.rawState || _('就绪'))),
			E('p', { 'class': 'a6-hint' },
				_('授权绑定本机硬件指纹；换机、改字符或改范围都无法通过验签。TARGET 是允许集合，不代表每个频段都做过驻网验收。'))
		]);
		body.appendChild(box);
	}

	function reload(out) {
		return query().then(info => paint(info, out)).catch(e => {
			body.textContent = String(e?.message || e);
		});
	}

	function doRefresh() {
		let out = body.querySelector('.a6-out');
		if (out) out.textContent = _('刷新中…');
		return reload();
	}

	function doApply() {
		let input = body.querySelector('#a6-unlock-token');
		let token = normalize(input?.value);
		let out = body.querySelector('.a6-out');
		if (!TOKEN_RE.test(token)) {
			if (out) out.textContent = _('授权码格式不对：应为 A1. 开头、共 89 个字符。');
			return;
		}
		if (out) out.textContent = _('正在写入并校验…');
		runAt('AT+AIR6BAND="' + token + '"').then(r => {
			let text = common.resultText(r);
			return query().then(info => {
				paint(info, text);
				if (info.state?.license === 'VALID') {
					ui.addNotification(null, E('p', {}, _('授权已生效，频段集合已切换。')), 'info');
					if (typeof onChange === 'function') onChange();
				}
			});
		}).catch(e => {
			if (out) out.textContent = String(e?.message || e);
		});
	}

	function doClear() {
		let out = body.querySelector('.a6-out');
		if (out) out.textContent = _('清除中…');
		runAt('AT+AIR6BAND=0').then(r => {
			reload(common.resultText(r));
			if (typeof onChange === 'function') onChange();
		})
			.catch(e => { if (out) out.textContent = String(e?.message || e); });
	}

	reload();
}

return baseclass.extend({ open: open });
