/* プリプロセッサ */
#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include "w2xconv.h"
#include "filter.h"

/* using宣言 */
using std::string;
using std::vector;

/* 定数宣言 */
// トラックバー
const int kTracks = 2;
TCHAR *track_name[] = {"noise", "scale"};	//名前
int track_default[] = {1, 0};			//初期値
int track_s[] = {0, 0};			//下限値
int track_e[] = {2, 1};			//上限値
enum kTrackType { kTrackNoise, kTrackScale };
// チェックボックス
const int kChecks = 1;
TCHAR *check_name[] = {"use GPU"};	//名前
int check_default[] = {0};	//初期値(値は0か1)
/* 構造体宣言 */
FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION, 0, 0, "waifu2x-w2xc",
	kTracks, track_name, track_default, track_s, track_e,
	kChecks, check_name, check_default,
	func_proc, func_init, func_exit, NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	"aviutl-waifu2x-w2xc Ver.1.2 by YSR",
	NULL, NULL,
};

/* グローバル変数宣言 */
W2XConv *conv;
bool waifu2x_flg = false;
int use_gpu_flg = 0;

/* プロトタイプ宣言 */
void waifu2x_w2xc(FILTER*, FILTER_PROC_INFO*, const int);

/* AviUtlから呼び出すための関数 */
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void){ return &filter; }

/* 初期化関数 */
bool w2xc_init(W2XConv *conv_, const int use_gpu_flg_){
	conv = w2xconv_init(use_gpu_flg_, 0, 0);
	int r = w2xconv_load_models(conv, ".\\plugins\\models");
	if(r < 0) return false;
	return true;
}

/* 開始時に呼ばれる関数 */
BOOL func_init(FILTER *fp){
	SetWindowText(fp->hwnd, "初期化中...");
	use_gpu_flg = fp->check[0];
	waifu2x_flg = w2xc_init(conv, use_gpu_flg);
	SetWindowText(fp->hwnd, "waifu2x-w2xc");
	if(!waifu2x_flg){
		char *err = w2xconv_strerror(&conv->last_error);
		MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
		return FALSE;
	}
	return waifu2x_flg;
}

/* 終了時に呼ばれる関数 */
BOOL func_exit(FILTER *fp){
	w2xconv_fini(conv);
	return TRUE;
}

/* 処理関数 */
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip){
	// どのモードでもない場合
	if((fp->track[kTrackNoise] == 0) && (fp->track[kTrackScale] == 0)){
		if(!fp->exfunc->is_saving(fpip->editp)) {
			SetWindowText(fp->hwnd, "waifu2x-w2xc");
		}
		return TRUE;
	}
	// チェックボックスが変更されていた場合
	if(use_gpu_flg != fp->check[0]){
		if(!fp->exfunc->is_saving(fpip->editp)) {
			SetWindowText(fp->hwnd, "再初期化中...");
		}
		use_gpu_flg = fp->check[0];
		waifu2x_flg = w2xc_init(conv, use_gpu_flg);
	}
	// 最大サイズを逸脱していた場合
	if(fp->track[kTrackScale] != 0){
		if((fpip->w * 2 > fpip->max_w) || (fpip->h * 2 > fpip->max_h)){
			if(!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "サイズが大きすぎます！", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// 正常にモジュールが読み込まれていない場合
	if(!waifu2x_flg){
		if(!fp->exfunc->is_saving(fpip->editp)) {
			MessageBox(fp->hwnd, "モデルデータが読み込まれていません！", "waifu2x-w2xc", MB_OK);
		}
		return FALSE;
	}
	// 計測準備
	SetWindowText(fp->hwnd, "変換中...");
	auto start = std::chrono::high_resolution_clock::now();
	// デノイズする場合
	if(fp->track[kTrackNoise] > 0){
		try{
			waifu2x_w2xc(fp, fpip, fp->track[kTrackNoise] - 1);
		}
		catch(char *err){
			if(!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
			}
			w2xconv_free(err);
			return FALSE;
		}
		catch(std::bad_alloc){
			if(!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "メモリ不足です！", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// 拡大する場合
	if(fp->track[kTrackScale] > 0){
		try{
			waifu2x_w2xc(fp, fpip, W2XCONV_FILTER_SCALE2x);
		}
		catch(char *err){
			if(!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
			}
			w2xconv_free(err);
			return FALSE;
		}
		catch(std::bad_alloc){
			if(!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "メモリ不足です！", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// 計測終了
	auto end = std::chrono::high_resolution_clock::now();
	auto duaration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::stringstream ss;
	ss << duaration << "[ms]";
	SetWindowText(fp->hwnd, ss.str().c_str());

	return TRUE;
}

/* waifu2xでデノイズ・拡大するための関数 */
void waifu2x_w2xc(FILTER *fp, FILTER_PROC_INFO *fpip, const int mode_){
	if(mode_ == W2XCONV_FILTER_SCALE2x){
		// 拡大しながら読み込み(Yのみ)
		int w = fpip->w, h = fpip->h, w2 = w * 2, h2 = h * 2;
		vector<float> src(w2 * h2);
		for(int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for(int x = 0; x < w; ++x){
				float param = 1.0 * ycp->y / 4096;
				int p = y * 2 * w2 + x * 2;
				src[p] = param;
				src[p + 1] = param;
				src[p + w2] = param;
				src[p + w2 + 1] = param;
				++ycp;
			}
		}
		fpip->w = w2;
		fpip->h = h2;
		// 変換
		vector<float> dst(w2 * h2);
		int r = w2xconv_apply_filter_y(conv, W2XCONV_FILTER_SCALE2x, (unsigned char *)&dst[0], 4 * w2, (unsigned char *)&src[0], 4 * w2, w2, h2, 128);
		if(r < 0) throw w2xconv_strerror(&conv->last_error);
		// 書き込み
		fp->exfunc->resize_yc(fpip->ycp_edit, w2, h2, fpip->ycp_edit, 0, 0, w, h);
		int p = 0;
		for(int y = 0; y < h2; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for(int x = 0; x < w2; ++x){
				ycp->y = round(dst[p] * 4096);
				++ycp;
				++p;
			}
		}
	} else{
		// 読み込み(Yのみ)
		int w = fpip->w, h = fpip->h;
		vector<float> src(h * w), dst(w * h);
		int p = 0;
		for(int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for(int x = 0; x < w; ++x){
				src[p] = 1.0 * ycp->y / 4096;
				++ycp;
				++p;
			}
		}
		// 変換
		int r = w2xconv_apply_filter_y(conv, (W2XConvFilterType)mode_, (unsigned char *)&dst[0], 4 * w, (unsigned char *)&src[0], 4 * w, w, h, 128);
		if(r < 0) throw w2xconv_strerror(&conv->last_error);
		// 書き込み
		p = 0;
		for(int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for(int x = 0; x < w; ++x){
				ycp->y = round(dst[p] * 4096);
				++ycp;
				++p;
			}
		}
	}
}
