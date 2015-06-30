/* �v���v���Z�b�T */
#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include "w2xconv.h"
#include "filter.h"

/* using�錾 */
using std::string;
using std::vector;

/* �萔�錾 */
const int kColors = 3;
// �g���b�N�o�[
const int kTracks = 3;
TCHAR *track_name[] = { "noise", "scale", "block" };	//���O
int track_default[] = {       1,       0,     128 };	//�����l
int track_s[] = { 0, 0,   1 };	//�����l
int track_e[] = { 2, 1, 2048 };	//����l
enum kTrackType { kTrackNoise, kTrackScale, kTrackBlock };
//�`�F�b�N�{�b�N�X(���E���O�E�����l��ݒ肷��)
const int kChecks = 2;
TCHAR *check_name[] = { "use GPU", "photography" };
int	  check_default[] = { 1, 0 };
enum kCheckBox { kCheckGPU, kCheckPhoto };
/* �\���̐錾 */
FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION, 0, 0, "waifu2x-w2xc",
	kTracks, track_name, track_default, track_s, track_e,
	kChecks, check_name, check_default,
	func_proc, func_init, func_exit, NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	"aviutl-waifu2x-w2xc Ver.1.4 by YSR",
	NULL, NULL,
};

/* �O���[�o���ϐ��錾 */
W2XConv *conv;
bool waifu2x_flg = false;
int use_gpu_flg = 0;
int photo_flg = 0;

/* �v���g�^�C�v�錾 */
void waifu2x_w2xc(FILTER*, FILTER_PROC_INFO*, const int);

/* AviUtl����Ăяo�����߂̊֐� */
//EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void){ return &filter; }
//���W���[����`�t�@�C���F$(ProjectDir)$(ProjectName).def
EXTERN_C __declspec(dllexport) FILTER_DLL* GetFilterTable(void){ return &filter; };

/* �������֐� */
bool w2xc_init(W2XConv *conv_, const int use_gpu_flg_, const int photo_flg){
	conv = w2xconv_init(use_gpu_flg_, 0, 0);
	int r;
	if (photo_flg){
		r = w2xconv_load_models(conv, ".\\plugins\\models_rgb_3d");
	}
	else{
		r = w2xconv_load_models(conv, ".\\plugins\\models_rgb");
	}
	if(r < 0) return false;
	return true;
}

/* �J�n���ɌĂ΂��֐� */
BOOL func_init(FILTER *fp){
	SetWindowText(fp->hwnd, "��������...");
	use_gpu_flg = fp->check[kCheckGPU];
	photo_flg = fp->check[kCheckPhoto];
	waifu2x_flg = w2xc_init(conv, use_gpu_flg, photo_flg);
	SetWindowText(fp->hwnd, "waifu2x-w2xc");
	if(!waifu2x_flg){
		char *err = w2xconv_strerror(&conv->last_error);
		MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
		return FALSE;
	}
	return waifu2x_flg;
}

/* �I�����ɌĂ΂��֐� */
BOOL func_exit(FILTER *fp){
	w2xconv_fini(conv);
	return TRUE;
}

/* �����֐� */
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip){
	// �ǂ̃��[�h�ł��Ȃ��ꍇ
	if ((fp->track[kTrackNoise] == 0) && (fp->track[kTrackScale] == 0)){
		if (!fp->exfunc->is_saving(fpip->editp)) {
			SetWindowText(fp->hwnd, "waifu2x-w2xc");
		}
		return TRUE;
	}
	// �`�F�b�N�{�b�N�X���ύX����Ă����ꍇ
	if (use_gpu_flg != fp->check[kCheckGPU]){
		if(!fp->exfunc->is_saving(fpip->editp)) {
			SetWindowText(fp->hwnd, "�ď�������...");
		}
		use_gpu_flg = fp->check[kCheckGPU];
		waifu2x_flg = w2xc_init(conv, use_gpu_flg, photo_flg);
	}
	if (photo_flg != fp->check[kCheckPhoto]){
		if (!fp->exfunc->is_saving(fpip->editp)) {
			SetWindowText(fp->hwnd, "�ď�������...");
		}
		photo_flg = fp->check[kCheckPhoto];
		waifu2x_flg = w2xc_init(conv, use_gpu_flg, photo_flg);
	}
	// �ő�T�C�Y����E���Ă����ꍇ
	if (fp->track[kTrackScale] != 0){
		if ((fpip->w * 2 > fpip->max_w) || (fpip->h * 2 > fpip->max_h)){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "�T�C�Y���傫�����܂��I", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// ����Ƀ��W���[�����ǂݍ��܂�Ă��Ȃ��ꍇ
	if(!waifu2x_flg){
		if(!fp->exfunc->is_saving(fpip->editp)) {
			MessageBox(fp->hwnd, "���f���f�[�^���ǂݍ��܂�Ă��܂���I", "waifu2x-w2xc", MB_OK);
		}
		return FALSE;
	}
	// �v������
	SetWindowText(fp->hwnd, "�ϊ���...");
	auto start = std::chrono::high_resolution_clock::now();
	// �f�m�C�Y����ꍇ
	if (fp->track[kTrackNoise] > 0){
		try{
			waifu2x_w2xc(fp, fpip, fp->track[kTrackNoise] - 1);
		}
		catch (char *err){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
			}
			w2xconv_free(err);
			return FALSE;
		}
		catch (std::bad_alloc){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "�������s���ł��I", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// �g�傷��ꍇ
	if (fp->track[kTrackScale] > 0){
		try{
			waifu2x_w2xc(fp, fpip, W2XCONV_FILTER_SCALE2x);
		}
		catch (char *err){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
			}
			w2xconv_free(err);
			return FALSE;
		}
		catch (std::bad_alloc){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "�������s���ł��I", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
	}
	// �v���I��
	auto end = std::chrono::high_resolution_clock::now();
	auto duaration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::stringstream ss;
	ss << duaration << "[ms]";
	SetWindowText(fp->hwnd, ss.str().c_str());

	return TRUE;
}

/* waifu2x�Ńf�m�C�Y�E�g�傷�邽�߂̊֐� */
void waifu2x_w2xc(FILTER *fp, FILTER_PROC_INFO *fpip, const int mode_){
	int denoise_level;
	double scale;
	switch (mode_){
		case W2XCONV_FILTER_DENOISE1:
			denoise_level = 1;
			scale = 1.0;
			break;
		case W2XCONV_FILTER_DENOISE2:
			denoise_level = 2;
			scale = 1.0;
			break;
		case W2XCONV_FILTER_SCALE2x:
			denoise_level = 0;
			scale = 2.0;
			break;
	}
	// �g�債�Ȃ���ǂݍ���
	int w = fpip->w, h = fpip->h, w2 = w * scale, h2 = h * scale;
	vector<unsigned char> src(w * h * kColors);
	for (int y = 0; y < h; ++y){
		PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
		for (int x = 0; x < w; ++x){
			short r = (255 * ycp->y + (((22881 * ycp->cr >> 16) + 3) << 10)) >> 12;
			short g = (255 * ycp->y + (((-5616 * ycp->cb >> 16) + (-11655 * ycp->cr >> 16) + 3) << 10)) >> 12;
			short b = (255 * ycp->y + (((28919 * ycp->cb >> 16) + 3) << 10)) >> 12;
			if (r < 0){ r = 0; } if (r > 255){ r = 255; }
			if (g < 0){ g = 0; } if (g > 255){ g = 255; }
			if (b < 0){ b = 0; } if (b > 255){ b = 255; }
			int p = y * w + x;
			src[p * kColors] = r;
			src[p * kColors + 1] = g;
			src[p * kColors + 2] = b;
			++ycp;
		}
	}
	fpip->w = w2;
	fpip->h = h2;
	// �ϊ�
	vector<unsigned char> dst(w2 * h2 * kColors);
	int r = w2xconv_convert_rgb(conv, (unsigned char *)&dst[0], kColors * w2, (unsigned char *)&src[0], kColors * w, w, h, denoise_level, scale, fp->track[kTrackBlock]);
	if (r < 0) throw w2xconv_strerror(&conv->last_error);
	// ��������
	fp->exfunc->resize_yc(fpip->ycp_edit,w2, h2, fpip->ycp_edit, 0, 0, w, h);
	for (int y = 0; y < h2; ++y){
		PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
		for (int x = 0; x < w2; ++x){
			int p = y * w2 + x;
			int r = dst[p * kColors];
			int g = dst[p * kColors + 1];
			int b = dst[p * kColors + 2];
			ycp->y = ((4918 * r + 354) >> 10) + ((9655 * g + 585) >> 10) + ((1875 * b + 523) >> 10);
			ycp->cb = ((-2775 * r + 240) >> 10) + ((-5449 * g + 515) >> 10) + ((8224 * b + 256) >> 10);
			ycp->cr = ((8224 * r + 256) >> 10) + ((-6887 * g + 110) >> 10) + ((-1337 * b + 646) >> 10);
			++ycp;
		}
	}
}
