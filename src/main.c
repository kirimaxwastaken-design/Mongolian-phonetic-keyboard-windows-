#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <shellapi.h>
#include "engine.h"

#define WM_TRAY (WM_USER+1)
#define ID_TOGGLE 1
#define ID_EXIT 2
#define HOTKEY_ID 1
#define MAGIC 0x4D4E5048

static HHOOK hKb, hMs;
static int enabled = 1;
static HWND hwnd, lastFg;
static NOTIFYICONDATAW nid;
static HICON icOn, icOff;
static UINT wmTaskbar;
static unsigned char swallowed[128];

static void send_result(const EngineResult *r){
    INPUT in[64]; int n = 0;
    for(int i=0;i<r->bs;i++){
        ZeroMemory(&in[n],sizeof(INPUT)); in[n].type=INPUT_KEYBOARD; in[n].ki.wVk=VK_BACK; in[n].ki.dwExtraInfo=MAGIC; n++;
        ZeroMemory(&in[n],sizeof(INPUT)); in[n].type=INPUT_KEYBOARD; in[n].ki.wVk=VK_BACK; in[n].ki.dwFlags=KEYEVENTF_KEYUP; in[n].ki.dwExtraInfo=MAGIC; n++;
    }
    for(int i=0;i<r->ntext;i++){
        ZeroMemory(&in[n],sizeof(INPUT)); in[n].type=INPUT_KEYBOARD; in[n].ki.wScan=r->text[i]; in[n].ki.dwFlags=KEYEVENTF_UNICODE; in[n].ki.dwExtraInfo=MAGIC; n++;
        ZeroMemory(&in[n],sizeof(INPUT)); in[n].type=INPUT_KEYBOARD; in[n].ki.wScan=r->text[i]; in[n].ki.dwFlags=KEYEVENTF_UNICODE|KEYEVENTF_KEYUP; in[n].ki.dwExtraInfo=MAGIC; n++;
    }
    if(n) SendInput(n,in,sizeof(INPUT));
}

static LRESULT CALLBACK KbProc(int code, WPARAM wp, LPARAM lp){
    if(code!=HC_ACTION || !enabled) return CallNextHookEx(0,code,wp,lp);
    KBDLLHOOKSTRUCT *p=(KBDLLHOOKSTRUCT*)lp;
    if((p->flags & LLKHF_INJECTED) || p->dwExtraInfo==MAGIC) return CallNextHookEx(0,code,wp,lp);
    int ext = (p->flags & LLKHF_EXTENDED)!=0;
    int sc = (int)(p->scanCode & 0xFF);
    if(wp==WM_KEYUP || wp==WM_SYSKEYUP){
        if(!ext && sc<128 && swallowed[sc]){ swallowed[sc]=0; return 1; }
        return CallNextHookEx(0,code,wp,lp);
    }
    switch(p->vkCode){
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
        case VK_MENU: case VK_LMENU: case VK_RMENU: case VK_LWIN: case VK_RWIN: case VK_CAPITAL:
            return CallNextHookEx(0,code,wp,lp);
    }
    HWND fg=GetForegroundWindow();
    if(fg!=lastFg){ engine_reset(); lastFg=fg; }
    int mod = (GetAsyncKeyState(VK_CONTROL)|GetAsyncKeyState(VK_MENU)|GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN)) & 0x8000;
    if(mod || ext){ engine_reset(); return CallNextHookEx(0,code,wp,lp); }
    int shift=(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0;
    int caps=(GetKeyState(VK_CAPITAL)&1)!=0;
    EngineResult r;
    if(engine_key(sc,shift,caps,&r)){
        if(sc<128) swallowed[sc]=1;
        send_result(&r);
        return 1;
    }
    return CallNextHookEx(0,code,wp,lp);
}

static LRESULT CALLBACK MsProc(int code, WPARAM wp, LPARAM lp){
    if(code==HC_ACTION && (wp==WM_LBUTTONDOWN||wp==WM_RBUTTONDOWN||wp==WM_MBUTTONDOWN)) engine_reset();
    return CallNextHookEx(0,code,wp,lp);
}

static void update_tray(int add){
    nid.hIcon = enabled ? icOn : icOff;
    lstrcpyW(nid.szTip, enabled ? L"Mongolian phonetic: ON (Ctrl+Alt+M)" : L"Mongolian phonetic: OFF (Ctrl+Alt+M)");
    Shell_NotifyIconW(add?NIM_ADD:NIM_MODIFY,&nid);
}
static void toggle(void){ enabled=!enabled; engine_reset(); update_tray(0); }

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l){
    if(m==wmTaskbar){ update_tray(1); return 0; }
    switch(m){
    case WM_TRAY:
        if(l==WM_LBUTTONUP) toggle();
        else if(l==WM_RBUTTONUP){
            HMENU menu=CreatePopupMenu();
            AppendMenuW(menu, MF_STRING|(enabled?MF_CHECKED:0), ID_TOGGLE, L"Mongolian phonetic (Ctrl+Alt+M)");
            AppendMenuW(menu, MF_SEPARATOR,0,NULL);
            AppendMenuW(menu, MF_STRING, ID_EXIT, L"Exit");
            POINT pt; GetCursorPos(&pt); SetForegroundWindow(h);
            TrackPopupMenu(menu,TPM_RIGHTBUTTON,pt.x,pt.y,0,h,NULL);
            PostMessageW(h,WM_NULL,0,0); DestroyMenu(menu);
        }
        return 0;
    case WM_COMMAND:
        if(LOWORD(w)==ID_TOGGLE) toggle();
        else if(LOWORD(w)==ID_EXIT) DestroyWindow(h);
        return 0;
    case WM_HOTKEY: toggle(); return 0;
    case WM_DESTROY:
        Shell_NotifyIconW(NIM_DELETE,&nid);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR cmd, int show){
    HANDLE mtx=CreateMutexW(NULL,TRUE,L"Local\\MongolianPhoneticWin");
    if(GetLastError()==ERROR_ALREADY_EXISTS) return 0;
    int s=GetSystemMetrics(SM_CXSMICON);
    icOn =(HICON)LoadImageW(hi,MAKEINTRESOURCEW(1),IMAGE_ICON,s,s,0);
    icOff=(HICON)LoadImageW(hi,MAKEINTRESOURCEW(2),IMAGE_ICON,s,s,0);
    wmTaskbar=RegisterWindowMessageW(L"TaskbarCreated");
    WNDCLASSW wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hi; wc.lpszClassName=L"MongolianPhoneticWnd";
    RegisterClassW(&wc);
    hwnd=CreateWindowExW(0,wc.lpszClassName,L"Mongolian phonetic",0,0,0,0,0,NULL,NULL,hi,NULL);
    ZeroMemory(&nid,sizeof nid); nid.cbSize=sizeof nid; nid.hWnd=hwnd; nid.uID=1;
    nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP; nid.uCallbackMessage=WM_TRAY;
    update_tray(1);
    RegisterHotKey(hwnd,HOTKEY_ID,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,'M');
    hKb=SetWindowsHookExW(WH_KEYBOARD_LL,KbProc,hi,0);
    hMs=SetWindowsHookExW(WH_MOUSE_LL,MsProc,hi,0);
    MSG msg; while(GetMessageW(&msg,NULL,0,0)>0){ TranslateMessage(&msg); DispatchMessageW(&msg); }
    UnhookWindowsHookEx(hKb); UnhookWindowsHookEx(hMs);
    ReleaseMutex(mtx); return 0;
}
