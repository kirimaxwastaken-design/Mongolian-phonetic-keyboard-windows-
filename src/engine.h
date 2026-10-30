#ifndef ENGINE_H
#define ENGINE_H
typedef struct { int bs; unsigned short text[8]; int ntext; } EngineResult;
void engine_reset(void);
/* returns 1 if key was handled (swallow it and apply result), 0 to pass through */
int engine_key(int scancode, int shift, int caps, EngineResult *r);
#endif
