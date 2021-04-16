# blankcut Tool

【 ファイル名 】blankcut.exe  
【 バージョン 】1.20  

## 概要
指定サイズから余白を切り取ったデータを作成します。  
256色と32bitカラーに対応しています。
(パレットは別途出力)  

※余談  
PS2/PSP/DS時代のツールなので、今の時代には使いみちがないかも。  
余白をカットしてメモリやVRAMサイズを節約するのに使用しました。  
ファイルサイズを効率の良い単位で分割して転送や解凍することも。

※2021/3/7  
C11の勉強として、C11とLinux環境への対応を開始。  
VSCodeでのC開発とC11の経験を積むことが目的であり、機能・利便性は二の次。

## 仕様
- 対応画像はTim2とBMP。 ※2021/4/2 BMPサポート、TGA追加予定
- 画像は256色か32bitカラー。
- 元画像サイズは指定サイズで割り切れる。
- 256色の場合は8の倍数、32bitカラーの場合は2の倍数でサイズが補整される。
- 左上、または右下は透過色。(背景色)
- 元画像が指定画像より大きい場合、並びは下記の順で縦に並べられる。  
```
 ┌─┬─┬─┬─┐　　　┌─┐  
 │ 0│ 1│ 2│ 3│　　　│ 0│  
 ├─┼─┼─┼─┤　　　├─┤  
 │ 4│ 5│ 6│ 7│　→　│ 1│  
 ├─┼─┼─┼─┤　　　├─┤  
 │ 8│ 9│10│11│　　　│ 2│  
 └─┴─┴─┴─┘　　　├─┤  
```
- 追加機能として、バイナリを指定サイズ毎に分割する。

## ファイル構造
|ファイル構造|
|:--|
|ファイルヘッダ|
|Pixcelヘッダ|
|Pixcelデータ|


### ファイルヘッダ(ファイル全体に関する情報)
~~~C
typedef struct tagBlankCutHeader {  
    char      id[4];            // [ 4] ID('B', 'L', 'C', '\0')  
    uint32    PixcelNum;        // [ 4] ピクセルデータ数  
    uint32    PixcelHeadPos;    // [ 4] ピクセルヘッダの位置  
    uint32    PixcelDataPos;    // [ 4] ピクセルデータ位置  
    uint32    WidthMax;         // [ 4] 最大幅  
    uint32    HeightMax;        // [ 4] 最大高さ  
    uint32    SizeMax;          // [ 4] 最大サイズ  
    uint32    padding;          // [ 4]  
} BlankCutHeader;               // [32byte]  
~~~

id[4]           ：blankcutで出力されたデータであることを識別する為の文字列。  
PixcelNum       ：画像の数。Pixcelヘッダの数にもなる。  
PixcelHeadPos   ：ファイル先頭からのPixcelヘッダの位置。  
PixcelDataPos   ：ファイル先頭からのPixcelデータの位置。  
WidthMax        ：画像の最大幅。  
HeightMax       ：画像の最大高さ。  
SizeMax         ：画像の最大サイズ。  
padding         ：ファイルヘッダを32バイトに揃える為のパディング。  

### Pixcelヘッダ(対応するPixcelデータに関する情報)
~~~C
typedef struct tagBlankCutPixcelHeader {
    uint32    size;              // [ 4] データサイズ
    uint32    pixpos;            // [ 4] pixcelデータの位置(pixcelデータ位置から)
    MPOINT    wh;                // [ 8] 画像サイズ
    MPOINT    fwh;               // [ 8] 元画像サイズ
    MPOINT    ofs[2];            // [16] オフセット(0:左上 1:右下)
    uint32    pad[2];            // [ 8]
} BlankCutPixcelHeader;          // [48byte]
~~~

size            ：対応するPixcelデータのサイズ。  
pixpos          ：対応するPixcelデータ先頭からの位置。  
wh              ：対応する画像のサイズ。  
fwh             ：対応する画像の元サイズ。(余白を削る前=指定サイズ)  
ofs[2]          ：元サイズに対する座標位置。  
pad[2]          ：Pixcelヘッダを48バイトに揃える為のパディング。  

## 機能
```
blankcut [option] [in file] [out file]
    [option]
       -?       : ヘルプ出力
       -p [pos] : 基準位置
                  ０：左上(default)
                  １：右下
       -w       : チェックする幅　(省略時は画像幅)
       -h       : チェックする高さ(省略時は画像高さ)
       -r       : 幅補正の時、右辺を優先的に変更する※
       -b       : 高さ補正の時、上辺を優先的に変更する※
       -t       : テーブル出力
       -g       : 余白を削ったtim2/bmp出力
       -c [cut] : バイナリ分割
                  cut: 分割サイズ
       -q       : 標準出力への出力制御

[out file]は省略可能。
その場合は[in file]と同じ位置に出力される。

※省略時は中央揃え。
```

例)
- 幅が同じで縦長な画像の場合  
$> blankcut -p 1 -h 160 -g -t s01_kur.tm2 .\output\s01_kur.blc

- 大きな画像の場合  
$> blankcut -p 0 -w 160 -h 160 -g -t s07_kae.tm2 .\outkae\s07_kae.blc

- <-p [pos]>  
基準となるIndex番号 or カラーを取得する位置を指定。

```
(-p0)→┌───┐
       │      │
       │  絵  │
       │      │
       └───┘←(-p 1)
```

- <-w xxx>
- <-h xxx>  
チェックする画像サイズを指定する。  
指定しない場合、画像サイズの幅 or 高さになる。  
この画像サイズを[in file]画像から抜き出し、余白を削除出力する。  

- <-r>  
幅補正の時、右辺を優先的に変更する。  
これが指定されていると、補正の値がそのまま右辺に加算される。  

- <-b>  
高さ補正の時、上辺を優先的に変更する。  
これが指定されていると、補正の値が上辺のオフセットを上回る場合のみ変更される。  

- <-t>
テーブルを出力する。  
内容はPixcelヘッダーをテーブル化したものです。  
ファイル名は「出力ファイル名.tbl」。  

- <-g>
余白を削った画像を保存します。  
保存する画像形式は入力ファイルと同じ(Tim2/BMP)になります。  
保存先は出力ファイルと同じ位置です。  
ファイル名は「出力ファイル名_xxx.tm2」。  
※2021/4/2 BMPサポート、TGA追加予定

- <-c xxx>
指定したサイズでバイナリを分割（カット）する。
元ファイル名.3桁の数字 の形式で出力されます。
"-q" 以外のオプションと併用でいません。
このオプションが指定されると、blankcutの処理はキャンセルされます。

- <-q>
エラーメッセージ以外は表示されなくなります。  
経過が出力されなくなるのでお薦めできない。  

## サンプルの実行方法(blankcut)
- 256色
$ ./blankcut -w 128 -h 128 -g -t ./pict/sample_256.bmp ./output/sample_256.blc
- 32bitカラー
$ ./blankcut -w 128 -h 128 -g -t ./pict/sample_32.bmp ./output/sample_32.blc

256x384サイズの画像を128x128サイズ(-w/-h)で切り出し、緑色の不要な余白を削除します。  
その際に切り出した画像を保存し(-g)、Pixcelヘッダの内容をテーブルとして出力(-t)します。

## サンプルの実行方法(binarycut)
$ ./blankcut -c 0x10000 pict/sample_32.bmp ./output/

65536Byte単位で指定のバイナリを分割（カット）します。  
出力先にはディレクトリが指定でき、必ず最後に "/"(Linux) or "\\"(Windows) を入力してください。


## 履歴
- 2004/11/12 (ver.1.00)
  - 初版。  
- 2004/11/15 (ver.1.02)
  - オプション「-b」を追加。  
  - オプションのエラーチェック強化。  
- 2006/09/29 (ver.1.03)
  - オプション「-r」を追加。  
  - 補正処理で不具合があった点を修正。  
- 2021/04/02 (ver.1.10)
  - BMPファイルをサポート。  
  - Linux環境でのビルドに対応。
- 2021/04/03 (ver.1.11)
  - 出力先が指定されていないとエラーになる不具合を修正。  
- 2021/04/16 (ver.1.20)
  - Binary cutツールの機能を追加。
