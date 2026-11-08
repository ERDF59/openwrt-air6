#!/usr/bin/ucode
'use strict';
import {readfile,writefile,stat,open,rename,popen as __popen} from 'fs';
const base='/tmp/air6-update';
const mode=ARGV[0], id=ARGV[1];
function run(args) {
  let p=__popen(join(' ',map(args,v=>"'"+replace(''+v,"'","'\\''")+"'"))+' 2>&1','r');
  let out=p.read('all');let code=p.close();if(code)die((out || 'Command failed')+' ['+join(' ',args)+']');return trim(out);
}

function digest(path){return split(run(['/usr/bin/sha256sum',path]),' ')[0];}
function save(v){writefile(base+'/status.json.new',sprintf('%J',v));rename(base+'/status.json.new',base+'/status.json');}
function part(label){
  for(let n=1;n<=50;n++)if(index(readfile('/sys/class/block/mmcblk0p'+n+'/uevent')||'','PARTNAME='+label+'\n')>=0)return '/dev/mmcblk0p'+n;
  die('Missing partition '+label);
}
function size(path){return +(readfile('/sys/class/block/'+substr(path,5)+'/size')||0)*512;}
function active(){let m=match(readfile('/proc/cmdline')||'',/(^|[ \t])bootslot=([ab])([ \t]|$)/);if(!m)die('Unknown active slot');return m[2];}
function verify(meta){
  if(!meta || meta.id!=id)die('Upload session expired');
  if(stat(base+'/upload.tar')?.size!=meta.size)die('Incomplete upload');
  if(!stat('/etc/air6-cpe/update.pub'))die('No trusted release public key installed');
  let names=split(run(['/bin/tar','-tf',base+'/upload.tar']),'\n');
  if(length(names)>8 || length(names)<3)die('Invalid package entries');
  let verbose=split(run(['/bin/tar','-tvf',base+'/upload.tar']),'\n');
  for(let line in verbose)if(substr(line,0,1)!='-')die('Only regular files are allowed');
  for(let name in names)if(!match(name,/^(manifest\.json|manifest\.sig|boot\.img|rootfs\.img|connsys_gnss\.img|modem\.img)$/))die('Unexpected package member');
  for(let name in names)if(length(filter(names,n=>n==name))!=1)die('Duplicate package member');
  for(let name in ['manifest.json','manifest.sig']){
    let content=run(['/bin/sh','-c','tar -xOf /tmp/air6-update/upload.tar '+name+' | head -c 16385']);
    if(length(content)>16384)die('Manifest too large');writefile(base+'/'+name,content+'\n');
  }
  // usign authenticates exact bytes, so re-extract rather than normalize JSON.
  run(['/bin/sh','-c','tar -xOf /tmp/air6-update/upload.tar manifest.json > /tmp/air6-update/manifest.json; tar -xOf /tmp/air6-update/upload.tar manifest.sig > /tmp/air6-update/manifest.sig']);
  run(['/usr/bin/usign','-V','-m',base+'/manifest.json','-p','/etc/air6-cpe/update.pub','-x',base+'/manifest.sig']);
  let m=json(readfile(base+'/manifest.json'));
  if((m.format!=1 && m.format!=2 && m.format!=3) || m.board!='AIR6-MT6990' || m.kind!=meta.kind || !match(m.slot||'',/^(a|b|any)$/) || type(m.version)!='string' || length(m.version)>80)die('Package model/type/version mismatch');
  if(index(readfile('/tmp/sysinfo/model')||'','Air 6')<0 && index(readfile('/tmp/sysinfo/model')||'','AIR6')<0)die('This device is not an AIR6');
  let slot=active();
  if(m.slot=='any' && m.format!=3)die('Invalid universal package');
  let allowed=m.slot=='any'?(m.kind=='modem'?[slot]:['a','b']):[m.slot];
  let target=meta.target_slot || (m.slot=='any'?(m.kind=='modem'?slot:(slot=='a'?'b':'a')):m.slot);
  if(index(allowed,target)<0 || (m.kind=='modem' && target!=slot))die('Package slot does not match update target');
  m.package_slot=m.slot;m.slot=target;m.allowed_slots=allowed;m.current_slot=slot;
  let expected=m.kind=='firmware'?(m.format>=2?['boot','rootfs']:['boot','rootfs','connsys_gnss']):['modem'];
  if(type(m.images)!='array' || length(m.images)!=length(expected) || length(names)!=length(expected)+2)die('Wrong image set');
  let total=0;
  for(let role in expected){
    let entries=filter(m.images,x=>x.role==role);if(length(entries)!=1)die('Missing/duplicate role '+role);let image=entries[0];
    if(image.file!=role+'.img' || !match(image.sha256||'',/^[a-f0-9]{64}$/))die('Invalid image descriptor');
    let target=part((role=='modem'?'md1img':role)+'_'+m.slot);
    if(index(names,image.file)<0)die('Missing image member');
    if(image.size!=size(target) || image.size<=0 || image.size>134217728)die('Image must match full target partition size');
    total+=image.size;
    if(total>201326592)die('Image set too large');
    run(['/bin/sh','-c','tar -xOf /tmp/air6-update/upload.tar '+image.file+' > /tmp/air6-update/'+image.file]);
    if(stat(base+'/'+image.file)?.size!=image.size || digest(base+'/'+image.file)!=image.sha256)die('Image digest mismatch: '+role);
    image.target=target;
  }
  return {...m,digest:digest(base+'/upload.tar')};
}
try {
  if(!match(id||'',/^[a-f0-9-]{36}$/))die('Invalid upload id');
  let meta=json(readfile(base+'/upload.json')||'null');
  if(mode=='check'){
    // procd validates synchronously; keep its watchdog event loop unblocked.
    // Full archive and image hashes are checked before handoff and again in RAM.
    if(!meta || meta.id!=id || meta.kind!='firmware' || !match(meta.confirmed_digest||'',/^[a-f0-9]{64}$/) || !match(meta.target_slot||'',/^[ab]$/) || stat(base+'/upload.tar')?.size!=meta.size)die('Not authorized');
    run(['/usr/bin/usign','-V','-m',base+'/manifest.json','-p','/etc/air6-cpe/update.pub','-x',base+'/manifest.sig']);
    let m=json(readfile(base+'/manifest.json'));
    if(m.board!='AIR6-MT6990' || m.kind!='firmware' || (m.slot!=meta.target_slot && !(m.slot=='any' && m.format==3)))die('Target not authorized');
    exit(0);
  }
  save({running:true,output:mode=='flash'?'重新校验签名与镜像，准备刷写…':'正在校验签名、机型、槽位和全部镜像…'});
  let m=verify(meta);
  if(mode=='verify')save({running:false,verified:m,output:'签名、机型、槽位及所有镜像哈希校验通过'});
  else if(mode=='flash' || mode=='ramflash'){
    if(meta.confirmed_digest!=m.digest)die('Confirmation digest changed');
    if(m.kind=='firmware' && mode=='flash'){
      save({running:true,output:'正在切换到内存升级环境，随后刷入槽 '+uc(m.slot)+'…'});
      run(['/bin/sh','-c','. /lib/functions.sh; . /lib/upgrade/common.sh; install_bin /sbin/upgraded; test -x /tmp/root/sbin/upgraded']);
      run(['/bin/ubus','-t','300','call','system','sysupgrade',sprintf('%J',{prefix:'/tmp/root',path:base+'/upload.tar',force:true,defer_service_stop:true,command:'/usr/bin/ucode /usr/share/air6-cpe/update.uc ramflash '+id})]);
      die('RAM upgrade handoff returned unexpectedly');
    }
    if(m.kind=='firmware'){
      let mounts=readfile('/proc/mounts')||'';
      if(index(mounts,' /rom ')>=0 || index(mounts,' /overlay ')>=0 || index(mounts,'overlayfs:')>=0)die('Firmware flashing requires detached RAM root');
    }
    let p=__popen('mkdir /tmp/air6-flash.lock 2>&1','r');p.read('all');if(p.close())die('Another flash is running');
    // Firmware images are written only after pivoting to RAM and detaching overlay.
    if(m.kind=='modem'){
      run(['/bin/sh','-c','ifdown modem5g; for svc in air6-ca-daemon air6-modem-status ql_ril_service mipc_wan.init mipc_submonitor.init; do /etc/init.d/$svc stop >/dev/null 2>&1 || true; done']);
    }
    if(m.kind=='firmware')run(['/bin/dd','if='+part('misc'),'of='+base+'/misc-before.bin','bs=512']);
    for(let image in m.images){
      save({running:true,output:'正在刷写 '+image.role+' 到槽 '+uc(m.slot)+'，请勿断电…'});
      run(['/bin/dd','if='+base+'/'+image.file,'of='+image.target,'bs=1048576','conv=fsync']);
      if(digest(image.target)!=image.sha256)die('Readback verification failed: '+image.role);
    }
    if(m.kind=='firmware')run(['/usr/sbin/air6-ui-slotctl',m.slot,'--i-have-backed-up-misc']);
    run(['/bin/sync']);save({running:false,done:true,output:'全部镜像读回校验成功，正在重启…'});
    run(['/bin/sh','-c','(sleep 3; reboot) >/tmp/air6-update-reboot.log 2>&1 </dev/null &']);
  }else die('Unknown operation');
}catch(e){save({running:false,error:''+(e.message || e),output:'更新已停止：'+e});exit(1);}
