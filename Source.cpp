#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"gdiplus.lib")

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

TCHAR szClassName[] = TEXT("Window");

WNDPROC DefaultEditBoxWndProc;

void GetRoundRectPath(GraphicsPath* pPath, Rect r, int dia)
{
	// diameter can't exceed width or height
	if (dia > r.Width)    dia = r.Width;
	if (dia > r.Height)    dia = r.Height;

	// define a corner 
	Rect Corner(r.X, r.Y, dia, dia);

	// begin path
	pPath->Reset();

	// top left
	pPath->AddArc(Corner, 180, 90);

	// tweak needed for radius of 10 (dia of 20)
	if (dia == 20)
	{
		Corner.Width += 1;
		Corner.Height += 1;
		r.Width -= 1; r.Height -= 1;
	}

	// top right
	Corner.X += (r.Width - dia - 1);
	pPath->AddArc(Corner, 270, 90);

	// bottom right
	Corner.Y += (r.Height - dia - 1);
	pPath->AddArc(Corner, 0, 90);

	// bottom left
	Corner.X -= (r.Width - dia - 1);
	pPath->AddArc(Corner, 90, 90);

	// end path
	pPath->CloseFigure();
}

void DrawRoundRect(Graphics* pGraphics, Rect r, Color color, int radius, float width)
{
	// set to pixel mode
	int oldPageUnit = pGraphics->SetPageUnit(UnitPixel);

	// define the pen
	Pen pen(color, width);
	pen.SetAlignment(PenAlignmentCenter);

	// get the corner path
	GraphicsPath path;

	// get path
	GetRoundRectPath(&path, r, radius);

	// draw the round rect
	pGraphics->DrawPath(&pen, &path);

	// restore page unit
	pGraphics->SetPageUnit((Unit)oldPageUnit);
}

LRESULT CALLBACK MyEditBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int border_thickness = 6;
	switch (msg)
	{
		case WM_NCPAINT:
			{
				HDC hdc = GetWindowDC(hWnd);
				RECT rc;
				GetClientRect(hWnd, &rc);
				rc.right += 2 * border_thickness;
				rc.bottom += 2 * border_thickness;
				{
					Graphics g(hdc);
					g.Clear(Color::White);
					g.SetSmoothingMode(SmoothingModeAntiAlias);
					Rect rect = { rc.left, rc.left, rc.right - rc.left, rc.bottom - rc.top };
					if (GetFocus() == hWnd) {
						DrawRoundRect(&g, rect, Color(0, 97, 224), border_thickness, 2.0f);
					}
					else {
						DrawRoundRect(&g, rect, Color(159, 158, 161), border_thickness, 1.0f);
					}
				}
				ReleaseDC(hWnd, hdc);
			}
			return 0;
		case WM_NCCALCSIZE:
			if (lParam) {
				NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
				InflateRect(&sz->rgrc[0], -border_thickness, -border_thickness);
				return 0;
			}
			break;
		case WM_NCHITTEST:
			return HTCLIENT;
		default:
			break;
	}
	return CallWindowProc(DefaultEditBoxWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hEdit2;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindowEx(0, L"EDIT", 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(0, L"EDIT", 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		DefaultEditBoxWndProc = (WNDPROC)GetWindowLongPtr(hEdit1, GWLP_WNDPROC);
		SetWindowLongPtr(hEdit1, GWLP_WNDPROC, (LONG_PTR)MyEditBoxProc);
		SetWindowLongPtr(hEdit2, GWLP_WNDPROC, (LONG_PTR)MyEditBoxProc);
		break;
	case WM_SIZE:
		MoveWindow(hEdit1, 10, 10, 256, 32, TRUE);
		MoveWindow(hEdit2, 10, 50, 256, 32, TRUE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	ULONG_PTR gdiToken;
	GdiplusStartupInput gdiSI;
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindowEx(0,
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GdiplusShutdown(gdiToken);
	return (int)msg.wParam;
}
