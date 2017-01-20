#include <Windows.h>

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>


#include <cassert>
#include <d3d11.h>
#include <minwinbase.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")


class Window;
std::vector<Window*> g_windows;

ID3D11Device* g_device;
ID3D11DeviceContext* g_context;
IDXGIFactory* g_factory;

int g_width = 1024;
int g_height = 768;

IDXGISwapChain* createSwapChain(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferDesc.Width = g_width;
	swap_chain_desc.BufferDesc.Height = g_height;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.Windowed = true;

	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swap_chain_desc.Flags = 0;

	IDXGISwapChain* swap_chain;
	HRESULT result = g_factory->CreateSwapChain(g_device, &swap_chain_desc, &swap_chain);
	assert(SUCCEEDED(result));
	return swap_chain;
}

#define MAKE_BOOL(BOOL) ((BOOL) != 0)

class Window final
{
public:
	Window(HWND hwnd)
		:_hwnd(hwnd)
	{
		createSwapChain();
		createRenderTargetView();
	}

	void setClearColor(float* color)
	{
		memcpy(_clear_color, color, sizeof(_clear_color));
	}

	void clearColor()
	{
		if(!_isClosing)
		{
			g_context->ClearRenderTargetView(_render_target_view, _clear_color);
			HRESULT result = _swap_chain->Present(1, 0);
			result;
		}
	}

	HWND getHWnd() const
	{
		return _hwnd;
	}

	bool isClosing() const
	{
		return _isClosing;
	}

	void closeLater()
	{
		_isClosing = true;
	}

	~Window()
	{
		std::cout << "window dctor" << std::endl;
		std::flush(std::cout);
		if(_swap_chain != nullptr)
		{
			_swap_chain->Release();
		}
		if(_render_target_view != nullptr)
		{
			_render_target_view->Release();
		}
	}
protected:
	void createSwapChain()
	{
		_swap_chain = ::createSwapChain(_hwnd);
	}

	void createRenderTargetView()
	{
		HRESULT result;
		ID3D11Texture2D* backBuffer;
		result = _swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		assert(SUCCEEDED(result));
		result = g_device->CreateRenderTargetView(backBuffer, NULL, &_render_target_view);
		assert(SUCCEEDED(result));

		backBuffer->Release();
	}
private:
	HWND _hwnd;
	IDXGISwapChain* _swap_chain = nullptr;
	ID3D11RenderTargetView* _render_target_view = nullptr;
	float _clear_color[4];
	bool _isClosing = false;
};

void initD3D()
{
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, &feature_level, 1, D3D11_SDK_VERSION, &g_device, NULL, &g_context);
	assert(SUCCEEDED(result));
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&g_factory);
	assert(SUCCEEDED(result));

	std::cout << "d3d device created." << std::endl;
}



HWND createWindow(HINSTANCE hInstance, const std::wstring& windowClassName, const std::wstring& windowTitle);


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	std::wstring displayText = L"Hellow Win32";

	switch(msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		//TextOut(hdc, 5, 5, displayText.c_str(), static_cast<int>(displayText.length()));
		EndPaint(hwnd, &ps);
		break;
	case WM_LBUTTONUP:
		{
			//LPWSTR lpString[128];
			//GetWindowText(hwnd, reinterpret_cast<LPWSTR>(lpString), sizeof(lpString));
			//std::cout << "click from " << lpString << std::endl;
		}
		break;
	case WM_RBUTTONUP:
		{
		}
		break;
	case WM_DESTROY:
		{
			for(auto wnd : g_windows)
			{
				if(wnd->getHWnd() == hwnd)
				{
					wnd->closeLater();
				}
			}
			std::cout << g_windows.size() << std::endl;
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;//message handled and consumed.
}
void registerWindowClass(HINSTANCE hInstance, const std::wstring& windowClassName)
{
#pragma warning(push)
#pragma warning(disable:4302)
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = windowClassName.c_str();
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if(!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Call to RegisterClassEx failed!", L"Win32 Test", NULL);
		std::abort();
	}
#pragma warning(pop)
}

HWND createWindow(HINSTANCE hInstance, const std::wstring& windowClassName, const std::wstring& windowTitle)
{
	HWND hwnd = CreateWindow(
		windowClassName.c_str(),
		windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		g_width, g_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if(!hwnd)
	{
		MessageBox(nullptr, L"Call to CreateWindow failed!",  L"Win32 Test", NULL);
		std::abort();
	}

	return hwnd;
}


int startWindowLoop(std::vector<Window*> windows)
{
	HWND consoleWnd = GetConsoleWindow();
	ShowWindow(consoleWnd, SW_SHOW);

	for(auto wnd: windows)
	{
		ShowWindow(wnd->getHWnd(), SW_SHOW);
		UpdateWindow(wnd->getHWnd());
	}

	MSG msg;
	bool isRunning = true;
	while(isRunning)
	{
		for(auto wnd: windows)
		{
			bool hasMessage = MAKE_BOOL(PeekMessage(&msg, wnd->getHWnd(), 0, 0, PM_REMOVE));
			if (!hasMessage)
			{
				wnd->clearColor();
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		g_windows.erase(std::remove_if(g_windows.begin(), g_windows.end(), [](Window* wnd){
			if(wnd->isClosing())
			{
				delete wnd;
				return true;
			}else
			{
				return false;
			}
		}), g_windows.end());

		if (g_windows.empty()) break;
	}

	assert(g_windows.size() == 0);

	return 0;
}

void f(int* __restrict /*a*/, int* __restrict /*b*/)
{
	;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
	{
		for(auto wnd:g_windows)
		{
			delete wnd;
		}
		g_windows.clear();
		assert(g_windows.size() == 0);
		std::cout << "Closing...";
		MessageBox(NULL, L"", L"closing", MB_OK);

		return true;
	}
	default:
		return false;
	}
}

int main()
{
	initD3D();
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	std::wstring windowClassName = L"Win32 Test";
	std::wstring windowTitle1 = L"Win32 Test 1";
	std::wstring windowTitle2 = L"Win32 Test 2";
	std::wstring windowTitle3 = L"Win32 Test 3";

	registerWindowClass(hInstance, windowClassName);
	HWND hwnd1 = createWindow(hInstance, windowClassName, windowTitle1);
	HWND hwnd2 = createWindow(hInstance, windowClassName, windowTitle2);
	HWND hwnd3 = createWindow(hInstance, windowClassName, windowTitle3);
	Window* wnd1 = new Window(hwnd1);
	Window* wnd2 = new Window(hwnd2);
	Window* wnd3 = new Window(hwnd3);
	float color1[] = { 1.0, 0.0, 0.0, 1.0 };
	float color2[] = { 0.0, 1.0, 0.0, 1.0 };
	float color3[] = { 0.0, 0.0, 1.0, 1.0 };
	wnd1->setClearColor(color1);
	wnd2->setClearColor(color2);
	wnd3->setClearColor(color3);
	g_windows.push_back(wnd1);
	g_windows.push_back(wnd2);
	g_windows.push_back(wnd3);

	SetConsoleCtrlHandler(HandlerRoutine, true);

	int ret = startWindowLoop(g_windows);
	return ret;
}