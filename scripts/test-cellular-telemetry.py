#!/usr/bin/env python3
"""Read-only hardware smoke test; uses the caller's normal SSH authentication.

Usage: python3 scripts/test-cellular-telemetry.py root@ROUTER
No device identifiers or complete snapshots are printed.
"""
import json
import subprocess
import sys
import time


def main():
    if len(sys.argv) != 2 or sys.argv[1].startswith('-'):
        raise SystemExit('usage: test-cellular-telemetry.py root@ROUTER')
    host = sys.argv[1]

    def rpc(obj, method):
        response = subprocess.run(
            ['ssh', '-o', 'ConnectTimeout=8', '--', host,
             f'ubus call {obj} {method}'],
            check=True, capture_output=True, text=True, timeout=20)
        return json.loads(response.stdout)

    status = rpc('luci.air6modem', 'get_status')
    cells = rpc('luci.air6modem', 'get_cells')
    assert status.get('source') == cells.get('source') == 'qlril-ubus'
    assert not status.get('initializing') and not cells.get('stale')
    assert 0 <= cells['cache_age'] <= 30
    assert cells['api_errors']['registration'] == 0
    for section in ['cells', 'carriers', 'measurements']:
        assert isinstance(cells[section], list)
    for c in cells['carriers']:
        assert c['rat'] in ('LTE', 'NR') and c['role'] in ('PCC', 'SCC')
        assert 1 <= c['band'] <= 1024
        if c['role'] == 'SCC':
            assert c['active'] == (c['state'] == 2)
    for m in cells['measurements']:
        assert 'rsrp_index' in m and 'snr_index' in m
    ca = rpc('air6.cpe', 'get_ca')
    assert ca.get('ok') and not ca.get('stale')
    compat = json.loads(ca['raw'])
    assert compat['source'] == 'qlril-ubus'
    bands = rpc('air6.cpe', 'get_band_lock')
    if cells['api_errors'].get('band_lock') == 0:
        assert bands.get('code') == 0 and len(bands['lte_mask']) == 64
        assert len(bands['nr_mask']) == 256
    print('PASS: fresh API status/cells, carrier semantics, measurements and UI compatibility')
    print('Individual API results:', json.dumps(cells['api_errors'], sort_keys=True))
    print('CA/NSA/n41 radio operation requires separate network testing.')


if __name__ == '__main__':
    main()
