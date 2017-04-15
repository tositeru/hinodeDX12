#include "stdafx.h"

#include <iostream>

#include <stdlib.h>

#include <graphics\dx12\common\Exception.h>
#include <graphics\dx12\utility\winapi\Window.h>
#include <graphics\dx12\utility\winapi\CheckMemoryLeak.h>

#include "DXWindow\DXWindow.h"

using namespace std;

int main(int argc, const char** args)
{
	hinode::winapi::StartMemoryLeakCheck(0);
	hinode::winapi::Window::sDisableConsoleCloseButton();

	DXWindow window;
	try {
		DXWindow::InitParam initParam(1280, 720, L"ConquestGame");
		window.create(initParam);

		window.mainLoop();

	} catch (hinode::graphics::Exception& e) {
		e.writeLog();
		return 1;
	} catch (...) {
		return 1;
	}
	return 0;
}