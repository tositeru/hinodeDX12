@echo off
rem -------------------------------------------------------------------------
rem 
rem *.hファイルと*.cppファイルの文字コードをUTF-8(BOM付)に変換するバッチファイル
rem  VisualStudioのために作った
rem  使う際はnkf32.exeを参照できる状態にしてください
rem  こちらの環境ではWindows/System32/の中にnkf32.exeを置いている
rem  nkf32.exeが何かわからなければ、"nkf"で検索してください
rem 
rem -------------------------------------------------------------------------
dir /s /b *.h > LLLLLLLLLLLLLL.txt
for /f %%a in (LLLLLLLLLLLLLL.txt) do (
    echo %%a
    nkf32 -w8 %%a > hoge.txt
    copy hoge.txt /b %%a
    del hoge.txt
)

dir /s /b *.cpp > LLLLLLLLLLLLLL.txt
for /f %%a in (LLLLLLLLLLLLLL.txt) do (
    echo %%a
    nkf32 -w8 %%a > hoge.txt
    copy hoge.txt /b %%a
    del hoge.txt
)

del LLLLLLLLLLLLLL.txt

