#include "common.hpp"
#include "d3d_app.hpp"

const int width = 1280;
const int height = 768;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShow)
{
	D3DApp *app = D3DApp::GetApp();
	if (app->Initialize(hInstance, width, height)) {
		app->Run();
	}
	app->Destroy();
}
