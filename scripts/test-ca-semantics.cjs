const fs=require('fs'),vm=require('vm'),assert=require('assert/strict');
const path=require('path');
const source=fs.readFileSync(path.join(__dirname,'../package/hcy/air6-cpe-ui/files/www/cpe/app.js'),'utf8');
const part=source.slice(source.indexOf('function caAllCells()'),source.indexOf('function caAgeText()'));
const store={clock:1000000,ca:null};const ctx=vm.createContext({store});vm.runInContext(part,ctx);
function check(cells,expected,active){store.ca={ts:1000,ca:{valid:true},cells:cells.map(c=>({dir:'dl',state:2,bw:'100 MHz',...c}))};assert.equal(ctx.caIsActive(),active);assert.match(ctx.caSummaryText(),expected);}
check([{rat:'nr',band:78}],/单载波/,false);
check([{rat:'lte',band:3},{rat:'nr',band:78}],/双载波，未确认同制式 CA/,false);
check([{rat:'lte',band:3},{rat:'nr',band:78},{rat:'nr',band:78}],/NR 2CC 聚合激活/,true);
check([{rat:'nr',band:78},{rat:'nr',band:78,state:1}],/未确认聚合激活/,false);
console.log('PASS single carrier, EN-DC-only, NR 2CC with LTE anchor, deactivated SCC');
