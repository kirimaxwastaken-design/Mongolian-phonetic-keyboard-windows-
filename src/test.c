#include <stdio.h>
#include <string.h>
#include "engine.h"
static const struct{char ch;int sc;int shift;}M[]={{'a',0x1E,0},{'s',0x1F,0},{'d',0x20,0},{'f',0x21,0},{'g',0x22,0},{'h',0x23,0},{'j',0x24,0},{'k',0x25,0},{'l',0x26,0},{'\'',0x28,0},{'z',0x2C,0},{'x',0x2D,0},{'c',0x2E,0},{'v',0x2F,0},{'b',0x30,0},{'n',0x31,0},{'m',0x32,0},{'q',0x10,0},{'w',0x11,0},{'e',0x12,0},{'r',0x13,0},{'t',0x14,0},{'y',0x15,0},{'u',0x16,0},{'i',0x17,0},{'o',0x18,0},{'p',0x19,0}};
static unsigned short buf[256];static int n;
static void run(const char*s){
  n=0;engine_reset();
  for(;*s;s++){
    int c=*s,sh=0,sc=-1;
    if(c==' '){engine_reset();buf[n++]=' ';continue;}
    if(c>='A'&&c<='Z'){sh=1;c+=32;}
    if(c=='"'){sh=1;c='\'';}
    for(unsigned i=0;i<sizeof M/sizeof*M;i++) if(M[i].ch==c) sc=M[i].sc;
    EngineResult r; if(!engine_key(sc,sh,0,&r)){buf[n++]=c;continue;}
    n-=r.bs; for(int i=0;i<r.ntext;i++) buf[n++]=r.text[i];
  }
  for(int i=0;i<n;i++){unsigned c=buf[i];if(c<128)putchar(c);else printf("%c%c",0xC0|(c>>6),0x80|(c&63));}
  putchar('\n');
}
int main(){
 run("Mongol hel"); run("tsagaan shuw"); run("chi sh yu ya yo ye"); run("ai ui i' y x w q"); run("Ulaanbaatar"); run("Tsetseg Chinggis Shine"); run("''' b'' \"\" \"");
}
