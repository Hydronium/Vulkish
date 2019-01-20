// #define checking
#define str(s) #s
#define xstr(s) str(s)


#define __STDC_WANT_LIB_EXT1__ 1 //C compliance. To be expanded upon later...

#pragma message("__STDC_WANT_LIB_EXT1__ = " xstr(__STDC_WANT_LIB_EXT1__))

//#define __GL_H

#include <windows.h>
#include <sys/stat.h>

//#include "glcorearb.h"
//#include "fnld.h"
 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------*/
/*Variables*/
/*----------------------------------------------------*/
ATOM resultRegisterClass;

HDC         hDC         = NULL;
HGLRC       hRC         = NULL;

bool FULLSCREEN = FALSE;
bool INFOCUS = TRUE;

int globCmdShow;
int width, height;

short int unconsumedEvents = 0;

int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine,
                        int nCmdShow);

LRESULT CALLBACK WndProc(     HWND hWnd,
                              UINT uMSG,
                              WPARAM wParam,
                              LPARAM lParam);

int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine,
                        int nCmdShow)
{
      bool done = FALSE;
      globCmdShow = nCmdShow;

      WNDCLASSEX wc;
      HWND hWnd;
      MSG Msg;
      DWORD dwExStyle;
      DWORD dwStyle;
      RECT WindowRect;

      DWORD LastErrorCode;
      width = 640;
      height = 480;

      wc.cbSize        = sizeof(WNDCLASSEX);
      wc.style         = CS_HREDRAW |CS_VREDRAW | CS_OWNDC;
      wc.lpfnWndProc   = WndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = 0;
      wc.hInstance     = hInstance;
      wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = NULL;//(HBRUSH)(COLOR_WINDOW+1);
      wc.lpszMenuName  = NULL;
      wc.lpszClassName = "Vulkish";
      wc.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);

      resultRegisterClass = RegisterClassEx(&wc);
      if (!resultRegisterClass) // Attempt To Register The Window Class
      {
            return 1;
      }

      //Do we want fullscreen? Not needed currently but keeping it so I don't forget.
      if (FULLSCREEN)
      {
            DISPLAY_DEVICE dispDevice;
            DEVMODE dmScreenSettings;
            
            memset(&dispDevice, 0, sizeof(dispDevice));
            memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));

            dispDevice.cb = sizeof(dispDevice);

            if (!(EnumDisplayDevices(NULL, 0, &dispDevice, 0)))
            {
                  LastErrorCode = GetLastError();
                  return 1;
            }
             
            dmScreenSettings.dmSize = sizeof(dmScreenSettings);

            if (!(EnumDisplaySettings(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dmScreenSettings)))
            {
                  LastErrorCode = GetLastError();
                  return 1;
            }

            dmScreenSettings.dmPelsWidth = width;
            dmScreenSettings.dmPelsHeight = height; 

            dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                  if (MessageBox(NULL, "Use window mode?", "GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                  {
                        FULLSCREEN = FALSE;
                  }
                  else
                  {
                        return FALSE;
                  }
            }

            //Fullscreen settings
            dwExStyle = WS_EX_APPWINDOW;
            dwStyle = WS_POPUP;
            ShowCursor(FALSE);
      }
      else
      {
            //Windowed settings
            dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
            dwStyle = WS_OVERLAPPEDWINDOW;
      }

      WindowRect.left = (long)0;
      WindowRect.right = (long)width;
      WindowRect.top = (long)0;
      WindowRect.bottom = (long)height;
      AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

      //Creating the window
      hWnd = CreateWindowEx(   0,//dwExStyle, //Extended settings
                              "Vulkish", //Class title
                              "Test", //Window title
                              WS_OVERLAPPEDWINDOW, //Regular settings
                              0,//CW_USEDEFAULT, 
                              0,//CW_USEDEFAULT, 
                              WindowRect.right - WindowRect.left,
                              WindowRect.bottom - WindowRect.top,
                              NULL, 
                              NULL, 
                              hInstance, 
                              NULL);
      
      if(hWnd == NULL)
      {
            LastErrorCode = GetLastError();
            return 1;
      }

      while (!done)
      {
            if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) //Does not block.
            {
                  if (Msg.message == WM_QUIT)
                  {
                        done = TRUE;
                  }
                  else
                  {
                        TranslateMessage(&Msg);
                        DispatchMessage(&Msg);
                  }
            }

      }

      return (Msg.wParam);
}

LRESULT CALLBACK WndProc(     HWND hWnd,
                              UINT uMSG,
                              WPARAM wParam,
                              LPARAM lParam)
{
      switch(uMSG)
      {
            case WM_ACTIVATE:
            {
                  if (LOWORD(wParam))
                  {
                        INFOCUS = TRUE;
                  }
                  else
                  {
                        INFOCUS = FALSE;
                  }
                  break;
            }
            //CreateWindowEx causes this to fire.
            case WM_CREATE:
            {
                  //If GLSetup fails, window creation is interrupted but CreateWindowEx WILL NOT NOTICE IT AS AN ERROR.
                  //In the future, consider changing this to include an option to "return 0;". This will let windows handle window creation, says teh MSDN.
                  //Another method involving a windows-defined function call was also mentioned...
                  /*if (!(GLSetup(hWnd, lParam)))
                  {
                        return -1;
                  }*/
                  break; //outside the switch is a "return 0;"
            }
            case WM_CLOSE:
            {
                  DestroyWindow(hWnd);
                  break;
            }
            case WM_DESTROY:
            {
                  PostQuitMessage(0);
                  break;
            }
            case WM_SIZE:
            {
                  break; //There was no break before; does it need to fall through to the default case? 20190119, TBD
            }
            default:
            {
                  return DefWindowProc(hWnd, uMSG, wParam, lParam);
            }
      }

      return 0;
}
