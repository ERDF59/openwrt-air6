'use strict';
'require baseclass';
'require ui';
'require air6modem.i18n as i18n';

/* Air6 5G 模组页面的公共样式与小工具。
 * 视觉参考 MT5700 一类现代 CPE 面板：大圆环信号表 + 指标磁贴 + 卡片网格。
 * 颜色一律用 currentColor 派生，跟随 LuCI 主题的深浅色自动适配。 */

const CSS_LAYOUT = `
.a6-page{max-width:1180px;margin:0 auto;padding-bottom:4rem}
.a6-page,.a6-page *{box-sizing:border-box}
.a6-head{display:flex;flex-wrap:wrap;gap:.8rem;align-items:center;justify-content:space-between;margin:1.2rem 0 .9rem}
.a6-title{display:flex;align-items:center;gap:.7rem;font-size:1.35rem;font-weight:700;margin:0}
.a6-title .a6-logo{width:2rem;height:2rem;border-radius:.7rem;display:grid;place-items:center;font-size:1rem;
  background:linear-gradient(135deg,#2f7cf6,#41d1a7);color:#fff;box-shadow:0 6px 18px rgba(47,124,246,.35)}
.a6-live{display:inline-flex;align-items:center;gap:.45rem;font-size:.82rem;opacity:.75}
.a6-dot{width:.5rem;height:.5rem;border-radius:50%;background:#34c759;box-shadow:0 0 0 0 rgba(52,199,89,.55);animation:a6-pulse 2s infinite}
@keyframes a6-pulse{0%{box-shadow:0 0 0 0 rgba(52,199,89,.5)}70%{box-shadow:0 0 0 .55rem rgba(52,199,89,0)}100%{box-shadow:0 0 0 0 rgba(52,199,89,0)}}
.a6-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:1rem}
.a6-grid-wide{display:grid;grid-template-columns:minmax(0,1.55fr) minmax(0,1fr);gap:1rem;align-items:stretch}
@media(max-width:860px){.a6-grid-wide{grid-template-columns:1fr}}
.a6-card{position:relative;padding:1.15rem 1.25rem;border-radius:1rem;border:1px solid color-mix(in srgb,currentColor 12%,transparent);
  background:color-mix(in srgb,currentColor 4%,transparent);box-shadow:0 10px 30px rgba(15,23,42,.06);overflow:hidden}
.a6-card>h3{margin:0 0 .9rem;font-size:.95rem;letter-spacing:.02em;opacity:.85;display:flex;align-items:center;gap:.5rem}
.a6-card>h3:after{content:"";flex:1;height:1px;background:color-mix(in srgb,currentColor 12%,transparent)}
.a6-card-hero{padding:1.4rem 1.5rem;background:
  radial-gradient(120% 120% at 0% 0%,color-mix(in srgb,#2f7cf6 22%,transparent) 0%,transparent 55%),
  radial-gradient(120% 120% at 100% 0%,color-mix(in srgb,#41d1a7 20%,transparent) 0%,transparent 50%),
  color-mix(in srgb,currentColor 5%,transparent)}
.a6-badge{display:inline-flex;align-items:center;gap:.35rem;padding:.2rem .6rem;border-radius:999px;font-size:.78rem;font-weight:600;line-height:1.5}
.a6-badge-5g{background:linear-gradient(135deg,#3b6cf6,#7c4dff);color:#fff}
.a6-badge-lte{background:linear-gradient(135deg,#0ea5a4,#22c55e);color:#fff}
.a6-badge-off{background:color-mix(in srgb,#ef4444 18%,transparent);color:#ef4444}
.a6-badge-mut{background:color-mix(in srgb,currentColor 10%,transparent)}
.a6-tiles{display:grid;grid-template-columns:repeat(auto-fit,minmax(9.5rem,1fr));gap:.75rem}
.a6-tile{padding:.85rem .9rem;border-radius:.85rem;background:color-mix(in srgb,currentColor 6%,transparent);
  border:1px solid color-mix(in srgb,currentColor 10%,transparent)}
.a6-tile small{display:block;font-size:.74rem;opacity:.65;margin-bottom:.3rem;letter-spacing:.03em}
.a6-value{display:flex;align-items:baseline;gap:.25rem}
.a6-value b{font-size:1.22rem;font-weight:650;font-variant-numeric:tabular-nums;line-height:1.15}
.a6-tile strong{font-size:1.18rem;font-weight:650;font-variant-numeric:tabular-nums}
.a6-tile .a6-unit{font-size:.76rem;font-weight:500;opacity:.55}
.a6-kv{display:grid;grid-template-columns:auto 1fr;gap:.55rem 1rem;font-size:.9rem}
.a6-kv dt{opacity:.62}
.a6-kv dd{margin:0;font-weight:600;word-break:break-all;text-align:right}
.a6-note{font-size:.78rem;opacity:.6;margin:.6rem 0 0}
.a6-raw{max-height:16rem;overflow:auto;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.78rem;
  padding:.8rem;border-radius:.7rem;background:color-mix(in srgb,currentColor 7%,transparent);white-space:pre-wrap;word-break:break-all}
`;


/* 信号圆环 + 流量条 + 旧类名兼容（network/sms/terminal 三个页面沿用 m5- 前缀） */
const CSS_COMPONENTS = `
.a6-ring{position:relative;width:11.5rem;height:11.5rem;flex:0 0 auto}
.a6-ring svg{width:100%;height:100%;transform:rotate(-90deg)}
.a6-ring .a6-ring-track{fill:none;stroke:color-mix(in srgb,currentColor 12%,transparent);stroke-width:10}
.a6-ring .a6-ring-fill{fill:none;stroke-width:10;stroke-linecap:round;
  transition:stroke-dashoffset .9s cubic-bezier(.4,0,.2,1)}
.a6-ring-face{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:.1rem}
.a6-ring-face .a6-ring-val{font-size:1.85rem;font-weight:700;font-variant-numeric:tabular-nums;line-height:1}
.a6-ring-face .a6-ring-unit{font-size:.75rem;opacity:.6}
.a6-ring-face .a6-ring-cap{font-size:.72rem;margin-top:.35rem;padding:.1rem .5rem;border-radius:999px;
  background:color-mix(in srgb,currentColor 10%,transparent)}
.a6-hero-body{display:flex;gap:1.4rem;align-items:center;flex-wrap:wrap}
.a6-hero-main{flex:1 1 15rem;min-width:13rem;display:flex;flex-direction:column;gap:.55rem}
.a6-hero-op{font-size:1.5rem;font-weight:700;line-height:1.2;display:flex;align-items:center;gap:.6rem;flex-wrap:wrap}
.a6-hero-code{font-size:.85rem;font-weight:500;opacity:.5}
.a6-hero-sub{font-size:.85rem;opacity:.7}
.a6-chips{display:flex;gap:.5rem;flex-wrap:wrap;margin-top:1rem}
.a6-chip{display:inline-flex;align-items:center;gap:.4rem;padding:.32rem .7rem;border-radius:999px;
  background:color-mix(in srgb,currentColor 8%,transparent);border:1px solid color-mix(in srgb,currentColor 10%,transparent);font-size:.78rem}
.a6-chip span{opacity:.6}
.a6-chip b{font-weight:650;font-variant-numeric:tabular-nums}
.a6-tile-void{opacity:.45}
.a6-tile-x{display:inline-block;padding:.3rem .9rem;border-radius:.55rem;border:1px dashed color-mix(in srgb,currentColor 18%,transparent);font-size:.8rem}
.a6-bars{display:grid;gap:.65rem;margin-top:.9rem}
.a6-bar-row{display:grid;grid-template-columns:2.6rem 1fr 6.2rem;align-items:center;gap:.6rem;font-size:.82rem}
.a6-bar-track{height:.55rem;border-radius:999px;background:color-mix(in srgb,currentColor 12%,transparent);overflow:hidden}
.a6-bar-fill{height:100%;width:0;border-radius:999px;transition:width .5s ease}
.a6-bar-fill.a6-down{background:linear-gradient(90deg,#2f7cf6,#4fc3f7)}
.a6-bar-fill.a6-up{background:linear-gradient(90deg,#41d1a7,#a3e635)}
.a6-bar-val{text-align:right;font-variant-numeric:tabular-nums;font-weight:600}
.a6-quality-good{color:#22c55e}
.a6-quality-mid{color:#f59e0b}
.a6-quality-poor{color:#ef4444}
.a6-cell-table{width:100%;border-collapse:collapse;font-size:.85rem}
.a6-cell-table th{font-size:.74rem;text-transform:uppercase;letter-spacing:.04em;opacity:.6;text-align:left;padding:.45rem .5rem;border-bottom:1px solid color-mix(in srgb,currentColor 12%,transparent)}
.a6-cell-table td{padding:.5rem;border-bottom:1px solid color-mix(in srgb,currentColor 8%,transparent);font-variant-numeric:tabular-nums}
.a6-btn{display:inline-flex;align-items:center;gap:.4rem;padding:.45rem .9rem;border-radius:.6rem;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  background:color-mix(in srgb,currentColor 6%,transparent);color:inherit;cursor:pointer;font-size:.85rem;transition:.15s;white-space:nowrap}
.a6-btn:hover{background:color-mix(in srgb,currentColor 12%,transparent)}
.a6-btn-primary{background:linear-gradient(135deg,#2f7cf6,#3b82f6);border-color:transparent;color:#fff}
.a6-btn-primary:hover{filter:brightness(1.06)}
/* 旧页面（network/sms/terminal）沿用 m5- 前缀，这里统一成新外观 */
.m5-page{max-width:1180px;margin:0 auto}
.m5-page h2{font-size:1.35rem;font-weight:700;margin:1.2rem 0 .9rem}
.m5-panel{padding:1.15rem 1.25rem;border-radius:1rem;border:1px solid color-mix(in srgb,currentColor 12%,transparent);
  background:color-mix(in srgb,currentColor 4%,transparent);box-shadow:0 10px 30px rgba(15,23,42,.06);margin:1rem 0}
.m5-panel h3{margin:0 0 .9rem;font-size:.95rem;opacity:.85}
.m5-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(9.5rem,1fr));gap:.75rem}
.m5-metric{padding:.85rem .9rem;border-radius:.85rem;background:color-mix(in srgb,currentColor 6%,transparent);
  border:1px solid color-mix(in srgb,currentColor 10%,transparent)}
.m5-metric small{display:block;font-size:.74rem;opacity:.65;margin-bottom:.25rem}
.m5-metric strong{font-size:1.05rem;font-variant-numeric:tabular-nums;word-break:break-all}
.m5-raw{max-height:18rem;overflow:auto;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:.78rem;
  padding:.8rem;border-radius:.7rem;background:color-mix(in srgb,currentColor 7%,transparent);white-space:pre-wrap;word-break:break-all}
.m5-form{display:grid;grid-template-columns:10rem minmax(0,1fr);gap:.6rem 1rem;align-items:center;max-width:46rem}
.m5-form label{opacity:.7;font-size:.85rem}
.m5-form input,.m5-form textarea{padding:.5rem .65rem;border-radius:.55rem;border:1px solid color-mix(in srgb,currentColor 16%,transparent);
  background:color-mix(in srgb,currentColor 5%,transparent);color:inherit;width:100%}
.m5-actions{display:flex;gap:.5rem;align-items:center;flex-wrap:wrap}
.m5-note{font-size:.78rem;opacity:.6}
.m5-head{display:flex;flex-wrap:wrap;gap:.8rem;align-items:center;justify-content:space-between}
.m5-live{font-size:.82rem;opacity:.7}
`;

function num(value) {
	let n = parseFloat(value);
	return Number.isFinite(n) ? n : null;
}

i18n.install();

return baseclass.extend({
	style() { return E('style', {}, CSS_LAYOUT + CSS_COMPONENTS); },
	/* 旧页面还在调用 extraStyle()，保留空实现避免报错。 */
	extraStyle() { return E('style', {}, ''); },

	callError(title, err) {
		ui.addNotification(title, E('p', {}, err?.message || String(err)), 'error');
	},
	resultText(result) {
		return result?.output || result?.error || _('No response');
	},
	/* 旧页面用的两个构造器，换成新外观 */
	panel(title, body) {
		return E('section', { 'class': 'm5-panel' }, [ E('h3', {}, title), body ]);
	},
	metric(label, value, id) {
		return E('div', { 'class': 'm5-metric' }, [
			E('small', {}, label),
			E('strong', id ? { id } : {}, value || '-')
		]);
	},

	/* 新组件 */
	card(title, body, cls) {
		return E('section', { 'class': 'a6-card' + (cls ? ' ' + cls : '') },
			title ? [ E('h3', {}, title), body ] : [ body ]);
	},
	tile(label, value, unit, id) {
		return E('div', { 'class': 'a6-tile' }, [
			E('small', {}, label),
			E('div', { 'class': 'a6-value' }, [
				E('b', id ? { id } : {}, value == null || value === '' ? '-' : value),
				unit ? E('span', { 'class': 'a6-unit' }, unit) : ''
			])
		]);
	},
	badge(rat) {
		let text = String(rat || '').toUpperCase(), cls = 'a6-badge-mut';
		if (text.indexOf('5G') >= 0 || text.indexOf('NR') >= 0)
			cls = 'a6-badge-5g';
		else if (text.indexOf('LTE') >= 0 || text.indexOf('4G') >= 0)
			cls = 'a6-badge-lte';
		else if (!text || text === 'UNKNOWN')
			cls = 'a6-badge-off';
		let label = cls === 'a6-badge-5g' ? '5G' : cls === 'a6-badge-lte' ? '4G' : _('无服务');
		return E('span', { 'class': 'a6-badge ' + cls }, label);
	},
	kv(pairs) {
		let body = [];
		for (let i = 0; i < pairs.length; i++) {
			body.push(E('dt', {}, pairs[i][0]));
			let value = pairs[i][1];
			body.push(E('dd', {}, (value == null || value === '') ? '-' : value));
		}
		return E('dl', { 'class': 'a6-kv' }, body);
	},

	/* 运营商代码 -> 友好名称（覆盖常见的国内/日本段） */
	operatorName(code) {
		const map = {
			'46000': '中国移动', '46002': '中国移动', '46004': '中国移动', '46007': '中国移动', '46008': '中国移动',
			'46001': '中国联通', '46006': '中国联通', '46009': '中国联通',
			'46003': '中国电信', '46005': '中国电信', '46011': '中国电信',
			'46015': '中国广电', '46020': '中国铁通',
			'44010': 'NTT docomo', '44020': 'SoftBank', '44011': 'Rakuten', '44100': '日本测试网'
		};
		return map[String(code || '')] || '';
	},
	/* 采集器里的英文状态串 -> 中文 */
	registrationText(value) {
		const map = {
			'registered (home)': _('已注册（本地）'),
			'registered (roaming)': _('已注册（漫游）'),
			'searching': _('正在搜网…'),
			'registration denied': _('注册被拒'),
			'not registered': _('未注册')
		};
		return map[String(value || '')] || (value || '-');
	},
	radioText(value) {
		let s = String(value || '');
		if (s.indexOf('STATE_ON') >= 0) return _('射频已开启');
		if (s.indexOf('STATE_OFF') >= 0) return _('射频已关闭');
		return s || '-';
	},
	simText(value) {
		let s = String(value || '');
		if (s.indexOf('READY') >= 0) return _('就绪');
		if (s.indexOf('ABSENT') >= 0) return _('未检测到卡');
		if (s.indexOf('PIN') >= 0) return _('需要 PIN');
		if (s.indexOf('BLOCK') >= 0 || s.indexOf('LOCK') >= 0) return _('已锁定');
		return s || '-';
	},
	ratText(value) {
		let s = String(value || '');
		if (s === 'LTE') return 'LTE（4G）';
		if (s === 'NR5G' || s === 'NR') return '5G NR';
		return s || '-';
	},

	/* RSRP -> 0..100，用于圆环 */
	signalPercent(rsrp) {
		let v = num(rsrp);
		if (v == null) return 0;
		return Math.max(0, Math.min(100, Math.round((v + 140) / 90 * 100)));
	},
	signalQuality(rsrp) {
		let v = num(rsrp);
		if (v == null) return { cls: '', text: _('未知') };
		if (v >= -85) return { cls: 'a6-quality-good', text: _('优') };
		if (v >= -100) return { cls: 'a6-quality-mid', text: _('良') };
		return { cls: 'a6-quality-poor', text: _('弱') };
	},
	valueQuality(value, good, mid) {
		let v = num(value);
		if (v == null) return '';
		return v >= good ? 'a6-quality-good' : v >= mid ? 'a6-quality-mid' : 'a6-quality-poor';
	},
	formatUptime(seconds) {
		let s = Math.max(0, parseInt(seconds, 10) || 0);
		let d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600), m = Math.floor(s % 3600 / 60);
		if (d) return _('%d 天 %d 小时').format(d, h);
		if (h) return _('%d 小时 %d 分').format(h, m);
		return _('%d 分').format(m);
	},
	formatRate(bytesPerSecond) {
		let mbps = Math.max(0, bytesPerSecond || 0) * 8 / 1000000;
		return mbps >= 100 ? `${Math.round(mbps)} Mbps` : `${mbps.toFixed(1)} Mbps`;
	}
});
