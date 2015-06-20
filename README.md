#aviutl-waifu2x-w2xc
waifu2xの処理を行うためのDLLである「w2xc.dll」を利用した、AviUtlのフィルタプラグインです。
当該DLLは、tanakmura([@tanakmura](https://twitter.com/tanakmura))氏によって作成されました。

##インストール
 * pluginsフォルダ以下を、aviutl.exeと同じフォルダにあるpluginsフォルダにコピーしてください。また、w2xc.dllは、AviUtlと同じフォルダに置いてください。
 * aufファイル、およびmodelsフォルダをaviutl.exeと同フォルダに置くと正常に動作しません。
 * 動作には[Visual Studio 2013 の Visual C++ 再頒布可能パッケージ](https://www.microsoft.com/ja-jp/download/details.aspx?id=40784)が必要です。

##使用方法
設定画面は次の通り。デフォルトでは0・0・128・チェックありにセットされています。
 * noiseトラックバー……デノイズレベル(0でデノイズしない)
 * scaleトラックバー……拡大するか否か(0だと拡大しない)
 * blockトラックバー……ブロックサイズ(縦横のサイズの公約数にすれば効率がいいです)
 * use GPUチェックボックス……未チェックでCPU・チェックでGPU計算です。
なお、設定画面のタイトルバーに演算時間がミリ秒単位で表示されます。
noiseとscaleを0にすれば、フィルタ処理が無効化され最初の表示に戻ります。

##注意事項
use GPUにチェックを入れない場合、CPUのみで計算されます。
しかしチェックを入れた場合、次の順番で使用可能かチェックされ、最初にOKだったもので計算されます。
 * CUDAが利用できる環境ではCUDAを使用する
 * OpenCLが利用できる環境ではOpenCLを使用する。ただしIntel GPUはFMAを使用した方が速いので無視される
 * FMAが利用できる環境ではFMAを使用する
 * AVXが利用できる環境ではAVXを使用する
 * どれもない環境ではOpenCVのfilter2Dを使用する
この仕様についての詳細は、次のtanakmura氏のブログの1ページを参照してください。
[waifu2x 高速化版更新 03:27](http://d.hatena.ne.jp/w_o/20150616#1434392833)

##コンパイル
Microsoft Visual Studio 2013でコンパイルしました。
AviUtlなので32bitバイナリを作成してください。

##更新履歴
Ver.1.3
ブロックサイズを設定できるようになった。

Ver.1.2
GPU計算も使えるようになった。

Ver.1.1
拡大した際正常に拡大されない不具合を修正。
サイズチェックを強化、AviUtlの最大サイズをはみ出た拡大はできないようにした。
ライセンス条項を修正。

Ver.1.0
初版。

##著作権表記
このソフトウェアはMITライセンスを適用しています。
また、MITライセンスに則り、w2xc.dllおよびw2xc.libの作者であるtanakmura氏の著作権表記、およびMITライセンスの全文を記したURLを記します。

[w2xc.dll](http://d.hatena.ne.jp/w_o/20150619#1434643288)
Copyright (c) 2015 tanakmura
Released under the MIT license
http://opensource.org/licenses/mit-license.php
