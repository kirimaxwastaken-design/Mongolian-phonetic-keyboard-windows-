#include "engine.h"
#include "tables.h"
static int state = 0;
void engine_reset(void){ state = 0; }
static void emit(EngineResult*r,const Str*s){ for(int i=0;i<s->n&&r->ntext<8;i++) r->text[r->ntext++]=s->c[i]; }
static const When* find(int action,int st){
    for(int i=0;i<NWHENS;i++) if(WHENS[i].action==action&&WHENS[i].state==st) return &WHENS[i];
    return 0;
}
int engine_key(int sc,int shift,int caps,EngineResult*r){
    r->bs=0; r->ntext=0;
    if(sc<0||sc>=128){ state=0; return 0; }
    const KeyDef*k=&KEYS[shift?1:(caps?2:0)][sc];
    if(k->kind==0){ state=0; return 0; }
    if(k->kind==1){ state=0; emit(r,&k->out); return 1; }
    for(int pass=0;pass<2;pass++){
        const When*w=find(k->action,state);
        if(w){
            if(w->out.n){ if(state) r->bs=1; emit(r,&w->out); state=0; return 1; }
            if(state) r->bs=1;
            emit(r,&TERM[w->next]); state=w->next; return 1;
        }
        if(!state) break;
        state=0; /* previous pending char is already shown */
    }
    state=0; return 0;
}
