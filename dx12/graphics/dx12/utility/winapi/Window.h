#pragma once

#include <Windows.h>

namespace hinode
{
	namespace winapi
	{
		/// @brief ウィンド関連をまとめたクラス
		///
		/// DX12などを使うときは適当に継承してください。
		/// 描画タイミングはonPaint関数で行うといいと思います。
		class Window
		{
		public:
			/// @brief 初期化用のパラメータ
			struct InitParam
			{
				LONG windowWidth;
				LONG windowHeight;
				const TCHAR* pWindowTitle;
				WNDPROC pWindowProc;

				InitParam()noexcept;
				InitParam(LONG width, LONG height, const TCHAR* pTitle)noexcept;
			};

			/// @brief 入力されたキーの情報
			///
			///	各パラメータの意味はMSDNのWM_KEYDOWN,WM_KEYUPを参考にしてください
			struct KeyInfo
			{
				DWORD repeatCount;
				UINT8 scanCode;
				bool isExtendeKey;
				bool contextCode;
				bool prevKeyState;
				bool transitionState;

				/// @brief LPARAMからデータを設定する
				KeyInfo& set(LPARAM lParam)noexcept;
			};

		public:
			/// @brief プロシージャ関数
			///
			/// create関数に渡すInitParam::pWindowProcに何も指定しなければこの関数が使用されます
			/// @param[in] hWnd
			/// @param[in] message
			/// @param[in] wParam
			/// @param[in] lParam
			/// @retval LRESULT
			static LRESULT CALLBACK sWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			/// @brief コンソールウェインドウの閉じるボタンを無効化する
			///
			/// コンソールの閉じるボタンを押すと強制停止扱いになって正規の終了処理が実行されないので作成
			static void sDisableConsoleCloseButton()noexcept;

		public:
			Window();

			virtual ~Window();

			/// @brief 作成
			/// @param[in] initParam
			void create(const InitParam& initParam);

			/// @brief ウィンドウを表示する
			/// @param[in] showCommand [default=SW_SHOW]
			void show(int showCommand = SW_SHOW);

			/// @brief メインループを実行する
			/// @retval int
			int mainLoop();

			/// @brief ウィンドウサイズが変わったときのイベント
			/// @param[in] wParam 渡される値はWM_SIZEで検索してください
			/// @param[in] width
			/// @param[in] height
			/// @retval LRESULT 特に理由がない場合は0を返してください
			virtual LRESULT onResize(WPARAM wParam, UINT width, UINT height);

			/// @brief キーが押されたときのイベント
			/// @param[in]] key
			/// @param[in] keyInfo
			/// @retval LRESULT 特に理由がない場合は0を返してください
			virtual LRESULT onKeyDown(UINT8 key, const KeyInfo& keyInfo);

			/// @brief キーが離されたときのイベント
			/// @param[in]] key
			/// @param[in] keyInfo
			/// @retval LRESULT 特に理由がない場合は0を返してください
			virtual LRESULT onKeyUp(UINT8 key, const KeyInfo& keyInfo);

			/// @biref 描画イベント
			virtual void onPaint();

			/// @brief ウィンドウハンドルを返す
			/// @retval HWND
			HWND HWnd()noexcept;

		private:
			HWND mHWnd;
		};
	}
}
