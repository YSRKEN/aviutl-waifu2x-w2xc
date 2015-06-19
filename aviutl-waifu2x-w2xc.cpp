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
// �g���b�N�o�[
const int kTracks = 2;
TCHAR *track_name[] = { "noise", "scale" };	//���O
int track_default[] = { 1, 0 };			//�����l
int track_s[] = { 0, 0 };			//�����l
int track_e[] = { 2, 1 };			//����l
enum kTrackType { kTrackNoise, kTrackScale };
// �`�F�b�N�{�b�N�X
/*const int kChecks = 2;
TCHAR *check_name[] = {"reduction1", "reduction2"};	//���O
int check_default[] = {           0,            0};	//�����l(�l��0��1)
enum kCheckType {kReduction1, kReduction2};*/
/* �\���̐錾 */
FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION, 0, 0, "waifu2x-w2xc",
	kTracks, track_name, track_default, track_s, track_e,
	NULL, NULL, NULL,
	func_proc, func_init, func_exit, NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	"aviutl-waifu2x-w2xc Ver.1.1 by YSR",
	NULL, NULL,
};

/* �O���[�o���ϐ��錾 */
W2XConv *conv;
bool waifu2x_flg = false;

/* �v���g�^�C�v�錾 */
void waifu2x_w2xc(FILTER*, FILTER_PROC_INFO*, const int);

/* AviUtl����Ăяo�����߂̊֐� */
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void){ return &filter; }

/* �������֐� */
BOOL func_init(FILTER *fp){
	conv = w2xconv_init(0, 0, 0);
	int r = w2xconv_load_models(conv, ".\\plugins\\models");
	if (r < 0){
		char *err = w2xconv_strerror(&conv->last_error);
		MessageBox(fp->hwnd, err, "waifu2x-w2xc", MB_OK);
		return FALSE;
	}
	waifu2x_flg = true;
	return TRUE;
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
	// �ő�T�C�Y����E���Ă����ꍇ
	if (fp->track[kTrackScale] != 0){
		if ((fpip->w * 2 > fpip->max_w) || (fpip->h * 2 > fpip->max_h)){
			if (!fp->exfunc->is_saving(fpip->editp)) {
				MessageBox(fp->hwnd, "�T�C�Y���傫�����܂��I", "waifu2x-w2xc", MB_OK);
			}
			return FALSE;
		}
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
	if (mode_ == W2XCONV_FILTER_SCALE2x){
		// �g�債�Ȃ���ǂݍ���(Y�̂�)
		int w = fpip->w, h = fpip->h, w2 = w * 2, h2 = h * 2;
		vector<float> src(w2 * h2);
		for (int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for (int x = 0; x < w; ++x){
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
		// �ϊ�
		vector<float> dst(w2 * h2);
		int r = w2xconv_apply_filter_y(conv, W2XCONV_FILTER_SCALE2x, (unsigned char *)&dst[0], 4 * w2, (unsigned char *)&src[0], 4 * w2, w2, h2, 128);
		if (r < 0) throw w2xconv_strerror(&conv->last_error);
		// ��������
		fp->exfunc->resize_yc(fpip->ycp_edit,w2, h2, fpip->ycp_edit, 0, 0, w, h);
		int p = 0;
		for (int y = 0; y < h2; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for (int x = 0; x < w2; ++x){
				ycp->y = round(dst[p] * 4096);
				++ycp;
				++p;
			}
		}
		w2xconv_test(conv, 128);
	}
	else{
		// �ǂݍ���(Y�̂�)
		int w = fpip->w, h = fpip->h;
		vector<float> src(h * w), dst(w * h);
		int p = 0;
		for (int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for (int x = 0; x < w; ++x){
				src[p] = 1.0 * ycp->y / 4096;
				++ycp;
				++p;
			}
		}
		// �ϊ�
		int r = w2xconv_apply_filter_y(conv, (W2XConvFilterType)mode_, (unsigned char *)&dst[0], 4 * w, (unsigned char *)&src[0], 4 * w, w, h, 128);
		if (r < 0) throw w2xconv_strerror(&conv->last_error);
		// ��������
		p = 0;
		for (int y = 0; y < h; ++y){
			PIXEL_YC *ycp = fpip->ycp_edit + y * fpip->max_w;
			for (int x = 0; x < w; ++x){
				ycp->y = round(dst[p] * 4096);
				++ycp;
				++p;
			}
		}
	}
}
