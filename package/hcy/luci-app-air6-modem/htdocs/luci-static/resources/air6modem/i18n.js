'use strict';
'require baseclass';

/* 运行时英文词典：跟随 LuCI 语言，仅在非中文界面替换页面文案。 */

const EN = {
	"%d 分": "%d min",
	"%d 天 %d 小时": "%d d %d h",
	"%d 小时 %d 分": "%d h %d min",
	"3G / 4G / 5G 自动": "3G / 4G / 5G auto",
	"4G / 5G 自动": "4G / 5G auto",
	"5G 模组": "5G Modem",
	"A1.xxxxxxxx…（89 位授权码）": "A1.xxxxxxxx… (89-character license)",
	"APN、IP 类型、认证、MTU、重试次数请到「网络 → 接口 → wan」修改，避免两个页面互相覆盖。": "Edit APN, IP type, auth, MTU and retries under Network → Interfaces → wan to avoid the two pages overwriting each other.",
	"APN、IP 类型、认证、MTU、重试次数请到「网络 → 接口」修改，避免两个页面互相覆盖。": "Edit APN, IP type, auth, MTU and retries under Network → Interfaces to avoid the two pages overwriting each other.",
	"AT 命令会直接改模组状态，可能中断注册、通话或数据业务。仅支持 AT 指令字符，不接受 shell 语法。": "AT commands change modem state and may interrupt registration, calls or data. Only AT characters are accepted; shell syntax is not.",
	"AT 终端": "AT Terminal",
	"AT 调试": "AT Debug",
	"CPU→Wi-Fi 注入": "CPU→Wi-Fi injections",
	"IP 类型": "IP type",
	"PPE0 绑定条目": "PPE0 bind entries",
	"PPE1 绑定条目": "PPE1 bind entries",
	"SIM 状态": "SIM status",
	"上行": "Upload",
	"下行": "Download",
	"不完整 %s": "Incomplete %s",
	"中国广电": "China Broadnet",
	"中国电信": "China Telecom",
	"中国移动": "China Mobile",
	"中国联通": "China Unicom",
	"中国铁通": "China Tietong",
	"仅 4G": "4G only",
	"仅 5G": "5G only",
	"优": "Excellent",
	"信号": "Signal",
	"信号与速率": "Signal & rate",
	"信号强度": "Signal strength",
	"信号质量": "Signal quality",
	"全部": "All",
	"全频段（已解锁）": "Full bands (unlocked)",
	"关闭": "Off",
	"关闭射频": "Turn radio off",
	"关闭射频会立即断网；重新拨号等于 ifdown + ifup。": "Turning the radio off disconnects immediately; redial equals ifdown + ifup.",
	"内容": "Content",
	"切换后模组会重新搜网，数据会短暂中断。": "The modem will rescan and data will pause briefly.",
	"切换后模组会重新搜网，期间数据会短暂中断。": "The modem will rescan and data will pause briefly.",
	"切换失败：": "Switch failed:",
	"删除": "Delete",
	"删除与短信中心": "Delete from SMSC",
	"删除短信": "Delete message",
	"刷新": "Refresh",
	"刷新中…": "Refreshing…",
	"刷新状态": "Refresh status",
	"加速状态": "Acceleration status",
	"加速统计": "Acceleration",
	"协议": "Protocol",
	"厂商 HNAT 独占 PPE；nft flowtable 在 HNAT 加载时由 fw4 自动禁用。": "Vendor HNAT owns the PPE; fw4 disables the nft flowtable while HNAT is loaded.",
	"原厂（未解锁）": "Factory (locked)",
	"发送": "Send",
	"发送命令": "Send command",
	"发送总量": "TX total",
	"发送短信": "Send SMS",
	"发送速率": "TX rate",
	"取 AT+CESQ 的标准字段；LTE 下无 SINR 时显示 “-”。": "Standard AT+CESQ fields; SINR shows “-” when unavailable on LTE.",
	"在线时长": "Uptime",
	"实时刷新": "Live",
	"实时刷新 · %s 秒前": "Live · %s s ago",
	"实时刷新 · 状态 %s 秒前": "Live · status %s s ago",
	"射频已关闭": "Radio off",
	"射频已开启": "Radio on",
	"小区 ID": "Cell ID",
	"小区信息": "Cell info",
	"就绪": "Ready",
	"就绪，输入命令或点上面的快捷按钮。": "Ready. Type a command or use a shortcut above.",
	"工程信息（接口状态与配置原文）": "Engineering info (interface state and raw config)",
	"工程诊断（AT+ECELL 原始返回）": "Engineering (raw AT+ECELL)",
	"已加载": "Loaded",
	"已发送": "Sent",
	"已注册（本地）": "Registered (home)",
	"已注册（漫游）": "Registered (roaming)",
	"已解锁：全频段集合（EU25）可用。": "Unlocked: full band set (EU25) available.",
	"已读": "Read",
	"已连接": "Connected",
	"已重新拨号": "Redialed",
	"已锁定": "Locked",
	"序号 %s": "No. %s",
	"应用制式": "Apply mode",
	"开启射频": "Turn radio on",
	"弱": "Weak",
	"当前射频：%s · 当前制式：%s": "Radio: %s · Mode: %s",
	"当前服务小区": "Serving cell",
	"待发送": "Queued",
	"执行": "Run",
	"拨号与网络": "Dial & network",
	"授权已生效，频段集合已切换。": "License applied; band set switched.",
	"授权码格式不对：应为 A1. 开头、共 89 个字符。": "Invalid license: must start with A1. and be 89 characters.",
	"授权绑定本机硬件指纹；换机、改字符或改范围都无法通过验签。TARGET 是允许集合，不代表每个频段都做过驻网验收。": "The license is bound to this hardware fingerprint; changing device, characters or range fails verification. TARGET is an allowed set, not a guarantee that every band has been field-tested.",
	"接口": "Interface",
	"接口流量": "Interface traffic",
	"接收号码": "Recipient",
	"接收总量": "RX total",
	"接收速率": "RX rate",
	"操作": "Action",
	"操作结果": "Result",
	"收件箱": "Inbox",
	"数据接口": "Data interface",
	"断开连接": "Disconnect",
	"无": "None",
	"无服务": "No service",
	"日本测试网": "Japan test network",
	"暂无短信（或当前 SIM 没有存储的消息）": "No messages (or none stored on the SIM)",
	"暂无邻区测量数据": "No neighbour measurements",
	"最多 160 个 GSM/ASCII 字符": "Up to 160 GSM/ASCII characters",
	"未加载": "Not loaded",
	"未检测到卡": "No SIM detected",
	"未注册": "Not registered",
	"未知": "Unknown",
	"未知号码": "Unknown number",
	"未解锁：只有原厂集合可用，灰色胶囊为扩展频段，点击“解锁频段”后开放。": "Locked: only factory bands are available; grey pills are extended bands, unlocked via “Unlock bands”.",
	"未解锁：只有原厂集合可用，灰色虚线胶囊为扩展频段，点击可去解锁。": "Locked: only factory bands are available; dashed grey pills are extended bands, click to unlock.",
	"未读": "Unread",
	"未连接": "Disconnected",
	"本机指纹（交给发码端换授权码）：": "Device fingerprint (give to the issuer for a license):",
	"来自 AT+ECELL 的邻区列表；服务小区见概览。": "Neighbour list from AT+ECELL; see Overview for the serving cell.",
	"概览": "Overview",
	"模组不支持 AIR6BAND 命令，请先刷入带授权功能的固件。": "The modem does not support AIR6BAND; flash firmware with licensing support first.",
	"模组与 SIM": "Modem & SIM",
	"模组温度": "Modem temperature",
	"模组状态": "Modem status",
	"正在写入并校验…": "Writing and verifying…",
	"正在搜网…": "Searching for network…",
	"正在等待模组就绪…": "Waiting for the modem…",
	"正在读取模组…": "Reading modem…",
	"注入回退": "Injection fallbacks",
	"注册状态": "Registration",
	"注册被拒": "Registration rejected",
	"消息序号": "Message ID",
	"清除中…": "Clearing…",
	"清除授权": "Clear license",
	"温度": "Temperature",
	"状态已刷新": "Status refreshed",
	"短信": "SMS",
	"短信中心": "SMSC",
	"短信已刷新": "Messages refreshed",
	"短信已提交给模组": "SMS submitted to modem",
	"确认解锁": "Confirm unlock",
	"等待模组就绪…": "Waiting for the modem…",
	"网关": "Gateway",
	"网络": "Network",
	"网络模式": "Network mode",
	"网络模式已切换，正在重新搜网…": "Network mode switched; rescanning…",
	"自动": "Auto",
	"良": "Good",
	"蜂窝拨号": "Cellular dial",
	"解锁频段": "Unlock bands",
	"认证方式": "Auth",
	"设置": "Settings",
	"设置短信中心": "Set SMSC",
	"读取中…": "Reading…",
	"调试": "Debug",
	"输出": "Output",
	"运营商": "Operator",
	"连接操作": "Connection",
	"连接状态": "Connection state",
	"通过厂商 libqlril 的短信接口提交；中文短信暂不支持，请用 ASCII。": "Submitted via the vendor libqlril SMS API; non-ASCII SMS is not supported, use ASCII.",
	"邻区 %s": "Neighbour %s",
	"邻区测量": "Neighbours",
	"邻区读取失败": "Failed to read neighbours",
	"重新拨号": "Redial",
	"错误 / 丢包": "Errors / drops",
	"需要 PIN": "PIN required",
	"频段": "Bands",
	"频段与解锁": "Bands & unlock",
	"频段集合": "Band set",
	"首选网络制式": "Preferred mode",
	"（原厂频段）": "(factory band)",
	"（尚未执行操作）": "(no action yet)",
	"（扩展频段）": "(extended band)"
};

const PATTERNS = [
	[ "^实时刷新 · (.+) 秒前$", "Live · $1 s ago" ],
	[ "^实时刷新 · 状态 (.+) 秒前$", "Live · status $1 s ago" ],
	[ "^邻区 (.+)$", "Neighbour $1" ],
	[ "^序号 (.+)$", "No. $1" ],
	[ "^不完整 (.+)$", "Incomplete $1" ],
	[ "^(\\d+) 天 (\\d+) 小时$", "$1 d $2 h" ],
	[ "^(\\d+) 小时 (\\d+) 分$", "$1 h $2 min" ],
	[ "^(\\d+) 分$", "$1 min" ],
	[ "^当前射频：(.+) · 当前制式：(.+)$", "Radio: $1 · Mode: $2" ]
];

function translateText(text) {
	let lead = text.match(/^\s*/)[0];
	let trail = text.match(/\s*$/)[0];
	let core = text.trim();
	if (!core)
		return text;

	let out = EN[core];
	if (out == null) {
		for (let [ re, rep ] of PATTERNS) {
			let m = core.match(re);
			if (m) {
				out = rep.replace(/\$(\d)/g, (_, i) => m[+i] ?? '');
				break;
			}
		}
	}
	return out == null ? text : lead + out + trail;
}

function walk(node) {
	if (!node)
		return;
	if (node.nodeType === 3) {
		node.nodeValue = translateText(node.nodeValue);
		return;
	}
	if (node.nodeType !== 1 || node.tagName === 'SCRIPT' || node.tagName === 'STYLE')
		return;
	for (let attr of [ 'placeholder', 'title', 'aria-label' ]) {
		if (node.hasAttribute && node.hasAttribute(attr))
			node.setAttribute(attr, translateText(node.getAttribute(attr)));
	}
	for (let child of node.childNodes)
		walk(child);
}

let installed = false;

function install() {
	if (installed)
		return;
	installed = true;

	let lang = (typeof L !== 'undefined' && L.env && L.env.lang) || document.documentElement.lang || '';
	if (/^zh/i.test(lang))
		return;

	let start = function() {
		walk(document.body);
		new MutationObserver(function(mutations) {
			for (let m of mutations) {
				if (m.type === 'childList') {
					for (let n of m.addedNodes)
						walk(n);
				}
				else if (m.type === 'characterData') {
					walk(m.target);
				}
			}
		}).observe(document.body, { childList: true, subtree: true, characterData: true });
	};

	if (document.body)
		start();
	else
		document.addEventListener('DOMContentLoaded', start);
}

return baseclass.extend({ install: install });
