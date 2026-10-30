import re, xml.etree.ElementTree as ET
s=open('Mongolian-phonetic.keylayout',encoding='utf-8').read()
s=s.replace('<?xml version="1.1"','<?xml version="1.0"')
s=re.sub(r'<!DOCTYPE[^>]*>','',s)
s=re.sub(r'&#x(0[0-9A-Fa-f]{3}|[0-9A-Fa-f]{1,2});',lambda m: m.group(0) if int(m.group(1),16)>=0x20 else 'CTL%04X'%int(m.group(1),16),s)
r=ET.fromstring(s.encode())
ms={m.get('id'):m for m in r.iter('keyMapSet')}
acts={a.get('id'):a for a in r.iter('action')}
terms={w.get('state'):w.get('output') for w in r.find('terminators')}
sc2mac={0x1E:0,0x1F:1,0x20:2,0x21:3,0x22:5,0x23:4,0x24:38,0x25:40,0x26:37,0x27:41,0x28:39,0x29:50,0x2B:42,
0x2C:6,0x2D:7,0x2E:8,0x2F:9,0x30:11,0x31:45,0x32:46,0x33:43,0x34:47,0x35:44,
0x10:12,0x11:13,0x12:14,0x13:15,0x14:17,0x15:16,0x16:32,0x17:34,0x18:31,0x19:35,0x1A:33,0x1B:30,
0x02:18,0x03:19,0x04:20,0x05:21,0x06:23,0x07:22,0x08:26,0x09:28,0x0A:25,0x0B:29,0x0C:27,0x0D:24}
states={'':0}
def sid(n):
    if n not in states: states[n]=len(states)
    return states[n]
aid={a:i for i,a in enumerate(acts)}
def S(t):
    t=t or ''
    assert len(t)<=3
    return '{%d,{%s}}'%(len(t),','.join(str(ord(c)) for c in t) or '0')
whens=[]
for a,el in acts.items():
    for w in el.findall('when'):
        st=w.get('state'); st=0 if st=='none' else sid(st)
        nx=w.get('next'); nx=0 if nx is None else (0 if nx=='none' else sid(nx))
        whens.append((aid[a],st,nx,w.get('output')))
# register all terminator states
for t in terms: sid(t)
out=['/* generated from ../tools/Mongolian-phonetic.keylayout by gen_tables.py */',
 'typedef struct{unsigned char n;unsigned short c[3];}Str;',
 'typedef struct{short action,state,next;Str out;}When;',
 'typedef struct{char kind;short action;Str out;}KeyDef; /* kind 0 pass,1 output,2 action */']
out.append('static const When WHENS[]={'+','.join('{%d,%d,%d,%s}'%(a,st,nx,S(o)) for a,st,nx,o in whens)+'};')
out.append('#define NWHENS %d'%len(whens))
tl=['{0,{0}}']*len(states)
for t,o in terms.items(): tl[states[t]]=S(o)
out.append('static const Str TERM[]={'+','.join(tl)+'};')
keys=[[ '{0,0,{0,{0}}}' for _ in range(128)] for _ in range(3)]
for mi in range(3):
    km=[k for k in ms['16c'] if int(k.get('index'))==mi][0]
    bycode={int(k.get('code')):k for k in km}
    for sc,mc in sc2mac.items():
        k=bycode[mc]
        if k.get('action') is not None:
            keys[mi][sc]='{2,%d,{0,{0}}}'%aid[k.get('action')]
        else:
            o=k.get('output')
            if o is None or o.startswith('CTL') or o==' ' or len(o)==0: continue
            keys[mi][sc]='{1,0,%s}'%S(o)
out.append('static const KeyDef KEYS[3][128]={'+','.join('{'+','.join(r_)+'}' for r_ in keys)+'};')
open('tables.h','w').write('\n'.join(out)+'\n')
print(len(whens),'whens',len(states),'states')
