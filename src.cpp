#include <windows.h>
#include <aviutl.hpp>
#include <exedit.hpp>


inline static char name[] = "音声クロスフェード";

inline static short* (__cdecl* create_audio_cache)(ExEdit::ObjectFilterIndex ofi, int size, int func_idx, int fillzero);
inline static BOOL(__cdecl* exedit_audio_func_main)(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip, int end_layer, int add_frame, int audio_speed, int milliframe, int scene_idx, ExEdit::ObjectFilterIndex ofi);

AviUtl::FilterPlugin* exedit_audio_fp;
AviUtl::FilterPlugin* get_exedit_audio_fp(ExEdit::Filter* efp) {
    AviUtl::SysInfo si;
    efp->aviutl_exfunc->get_sys_info(NULL, &si);

    for (int i = 0; i < si.filter_n; i++) {
        auto tfp = efp->aviutl_exfunc->get_filterp(i);
        if (tfp->information != NULL) {
            if (!strcmp(tfp->information, "拡張編集(音声) version 0.92 by ＫＥＮくん")) return tfp;
        }
    }
    return NULL;
}
BOOL func_init(ExEdit::Filter* efp) {
    exedit_audio_fp = get_exedit_audio_fp(efp);
    if (exedit_audio_fp != NULL) {
        create_audio_cache = reinterpret_cast<decltype(create_audio_cache)>((int)exedit_audio_fp->dll_hinst + 0x2a8f0);
        exedit_audio_func_main = reinterpret_cast<decltype(exedit_audio_func_main)>((int)exedit_audio_fp->dll_hinst + 0x49ca0);
    }
    return TRUE;
}

BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
    if (exedit_audio_fp == NULL) return TRUE;

    int sc_frame = max(0, efp->frame_start_chain - 1);

    AviUtl::FilterProcInfo fpi = *(AviUtl::FilterProcInfo*)efpip;
    fpi.flag = (AviUtl::FilterProcInfo::Flag)ExEdit::FilterProcInfo::Flag::nesting;
    fpi.frame = sc_frame;
    short* sub = create_audio_cache(efp->processing, efpip->audio_ch * efpip->audio_n * sizeof(short), efpip->v_func_idx + 1, 1);

    if (sub == nullptr) {
        return FALSE;
    }
    efp->aviutl_exfunc->get_audio_filtering(exedit_audio_fp, *reinterpret_cast<AviUtl::EditHandle**>((int)exedit_audio_fp->dll_hinst + 0x1a532c), efpip->frame_num, sub);
    fpi.audiop = sub;
    int audio_speed = efpip->audio_speed, audio_milliframe = efpip->audio_milliframe;
    if (audio_speed == 0) {
        audio_speed = 1000000;
        audio_milliframe = efpip->frame_num * 1000;
    }
    if (!exedit_audio_func_main(NULL, &fpi, efp->layer_set, efpip->frame_num - sc_frame, audio_speed, audio_milliframe, efp->scene_set, efp->processing)) {
        return FALSE;
    }

    double step_d = 1073741824.0 / (double)(efp->frame_end_chain - efp->frame_start_chain + 1);
    int step = (int)round(step_d * (double)audio_speed * 0.000001 / (double)efpip->audio_n);
    int rate = 1073741824 - (int)round((double)efpip->frame * step_d);
    short* rate_hi = (short*)&rate + 1;

    short* main = efpip->audio_p;
    for (int i = efpip->audio_n; 0 < i; i--) {
        for (int ch = efpip->audio_ch; 0 < ch; ch--) {
            *main += (*sub - *main) * *rate_hi >> 14;
            main++; sub++;
        }
        rate -= step;
    }
    return TRUE;
}

ExEdit::Filter ef = {
    .flag = ExEdit::Filter::Flag::Audio,
    .name = name,
    .track_n = 0,
    .check_n = 0,
    .func_proc = func_proc,
    .func_init = func_init,
};
ExEdit::Filter* filter_list[] = {
    &ef,
    NULL
};

EXTERN_C __declspec(dllexport)ExEdit::Filter** __stdcall GetFilterTableList() {
    return filter_list;
}
