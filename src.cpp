#include <windows.h>
#include <aviutl.hpp>
#include <exedit.hpp>


ExEdit::Filter ef;
inline static char name[] = "音声クロスフェード";

inline static short* (__cdecl* create_audio_cache)(ExEdit::ObjectFilterIndex ofi, int size, int func_idx, int fillzero);
inline static BOOL(__cdecl* exedit_audio_func_main)(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip, int end_layer, int add_frame, int audio_speed, int milliframe, int scene_idx, ExEdit::ObjectFilterIndex ofi);

AviUtl::FilterPlugin* exeditfp;
AviUtl::FilterPlugin* get_exeditfp(ExEdit::Filter* efp) {
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
    exeditfp = get_exeditfp(efp);
    if (exeditfp != NULL) {
        create_audio_cache = reinterpret_cast<decltype(create_audio_cache)>((int)exeditfp->dll_hinst + 0x2a8f0);
        exedit_audio_func_main = reinterpret_cast<decltype(exedit_audio_func_main)>((int)exeditfp->dll_hinst + 0x49ca0);
    }
    return TRUE;
}

BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
    int sc_frame = max(0, efp->frame_start_chain - 1);

    AviUtl::FilterProcInfo fpi = *(AviUtl::FilterProcInfo*)efpip;
    fpi.flag = (AviUtl::FilterProcInfo::Flag)ExEdit::FilterProcInfo::Flag::nesting;
    fpi.frame = sc_frame;
    short* sub = create_audio_cache(efp->processing, efpip->audio_ch * efpip->audio_n * 2, efpip->v_func_idx + 1, 1);

    if (sub == nullptr) {
        return FALSE;
    }
    efp->aviutl_exfunc->get_audio_filtering(exeditfp, *reinterpret_cast<AviUtl::EditHandle**>((int)exeditfp->dll_hinst + 0x1a532c), efpip->frame_num, sub);
    fpi.audiop = sub;
    int audio_speed = efpip->audio_speed;
    if (audio_speed == 0) {
        audio_speed = 1000000;
    }
    if (!exedit_audio_func_main(NULL, &fpi, efp->layer_set, efpip->frame_num - sc_frame, audio_speed, efpip->frame_num * 1000, efp->scene_set, efp->processing)) {
        return FALSE;
    }

    int rate = 32768 - (int)round((double)(efpip->frame + 1) * 32768.0 / (double)(efp->frame_end_chain - efp->frame_start_chain + 2));

    short* main = efpip->audio_p;
    for (int i = efpip->audio_ch * efpip->audio_n; 0 < i; i--) {
        *main += (*sub - *main) * rate >> 15;
        main++; sub++;
    }
    return TRUE;
}

ExEdit::Filter* filter_list[] = {
    &ef,
    NULL
};

EXTERN_C __declspec(dllexport)ExEdit::Filter** __stdcall GetFilterTableList() {
    ef.flag = ExEdit::Filter::Flag::Audio;
    ef.name = name;
    ef.track_n = 0;
    ef.check_n = 0;
    ef.func_proc = (func_proc);
    ef.func_init = (func_init);

    return filter_list;
}
