// #define checking
#define str(s) #s
#define xstr(s) str(s)

//PellesC compiler defaults to most recent. V10 uses C17.
//According to https://stackoverflow.com/questions/47867130/stdc-lib-ext1-availability-in-gcc-and-clang this may be pointless?
//#define __STDC_WANT_LIB_EXT1__ 1

//#pragma message("__STDC_WANT_LIB_EXT1__ = " xstr(__STDC_WANT_LIB_EXT1__))

//#define __GL_H

#include "C:\VulkanSDK\1.2.141.2\Include\vulkan\vulkan.h"

#include <windows.h>
#include <sys/stat.h>

//#include "glcorearb.h"
//#include "fnld.h" 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
/*----------------------------------------------------*/
/*Variables*/
/*----------------------------------------------------*/
ATOM resultRegisterClass;

HANDLE eventFPSTimer;
HANDLE hFPSTimer = NULL;
HANDLE hTimerQueue = NULL; //By setting to null instead of storing the result of CreateTimerQueue, CreateTimerQueueTimer will use the default timer queue when the timer is created; we create a queue though, see below

bool FULLSCREEN = FALSE;
bool INFOCUS = TRUE;

int globCmdShow;
int width, height;

short int unconsumedEvents = 0;

char * logfilename;
FILE * logfile;

/*----------------------------------------------------*/
/*Function Definitions*/
/*----------------------------------------------------*/
int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine,
                        int nCmdShow);

LRESULT CALLBACK WndProc(     HWND hWnd,
                              UINT uMSG,
                              WPARAM wParam,
                              LPARAM lParam);

VOID CALLBACK FPSTimer(       PVOID lpParam,          //Optional data from CreateTimerQueueTimer
                              BOOLEAN hasFired);      //True for timers, false for wait events

int logCreateFile(void);
int logMsg(char * msgtext);
int logCloseFile(void);

/*----------------------------------------------------*/
/*Function Implementations*/
/*----------------------------------------------------*/
int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine,
                        int nCmdShow)
{
      bool done = FALSE;
      globCmdShow = nCmdShow;

      //+Window Variables
      WNDCLASSEX wc;
      HWND hWnd;
      MSG Msg;
      DWORD dwExStyle;
      DWORD dwStyle;
      RECT WindowRect;
      //-Window Variables

      //+Timer Var
      DWORD resultWaitResTimer = 0;
      //-Timer Var

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

      //test only
	logCreateFile();
      logMsg("Test message");

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

      eventFPSTimer = CreateEvent(  NULL,       //No SECURITY_ATTRIBUTES struct
                                    FALSE,      //Auto-reset
                                    FALSE,      //Initially off
                                    NULL);      //No name

      if(GetLastError())
      {
            return FALSE;
      }

      hTimerQueue = CreateTimerQueue();

      if(GetLastError())
      {
            return FALSE;
      }
      //20190127 - i don't know why the timer period is 8ms here vs 17ms in the WaitForSingleObject; maybe old-me wanted a 120fps screen with a 60fps fallback or something? TBD.
      if (!CreateTimerQueueTimer(   &hFPSTimer,                         //Address of timer handle
                                    hTimerQueue,                        //Which queue to put it in
                                    (WAITORTIMERCALLBACK)FPSTimer,      //What callback function to use
                                    NULL,                               //No optional parameter
                                    20,                                 //20Ms till first fire
                                    8,                                  //8Ms period between fires
                                    WT_EXECUTEINTIMERTHREAD))           //Execute in this thread
      {
        GetLastError();
        return FALSE;
      }

      ShowWindow(hWnd, globCmdShow);
      SetForegroundWindow(hWnd);
      UpdateWindow(hWnd);

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

            //60 frames per second is roughly 16.667~ milliseconds per frame
            //17 is close enough for our needs
            resultWaitResTimer = WaitForSingleObject(eventFPSTimer, 17);
            if (resultWaitResTimer == WAIT_FAILED)
            {
                  LastErrorCode = GetLastError();
                  return -1;
            }
            else if (resultWaitResTimer == WAIT_OBJECT_0)
            {
                  unconsumedEvents--;
            }

      }

      return (Msg.wParam);
}

//2020-08-16 TODO: Add case for WM_GETMINMAXINFO for when screen size changes. Not critical.
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
                  //break; //There was no break before; does it need to fall through to the default case? 20190119, TBD
                  //2020-08-16 Removed break since DefWindowProc needs to be called if we are not handling the param/case.
            }
            default:
            {
                  //When we don't know what to do, this default case ensures that all messages are handled by sending them to the default window handler code in winapi.
                  return DefWindowProc(hWnd, uMSG, wParam, lParam);
            }
      }

      return 0;
}
      
VOID CALLBACK FPSTimer(       PVOID lpParam,
                              BOOLEAN hasFired)
{
      if (hasFired)
      {
            if (unconsumedEvents < 15) //If we've missed 15 let's stop firing until it drops. Wouldn't want to overflow.
            {      
                  SetEvent(eventFPSTimer); 
                  unconsumedEvents++;
            }
      }
}

int logCreateFile(void)
{
      logfilename = malloc(sizeof logfile * L_tmpnam);

      if (tmpnam(logfilename))
      {
            logfile = fopen(logfilename, "w+");
            return 0;
      }
      return 1;
}

/*
Simple log function. Get the current time, convert it to UTC, and then print it to the logfile with the message text.
*/
int logMsg(char * msgtext)
{
      time_t currenttime;
      struct tm *currenttime_utc;

      if (time(&currenttime) == -1)
      {
            return 1;//todo?
      }
      else
      {
            currenttime_utc = gmtime(&currenttime);
      }
      if(fprintf(logfile, "%d-%.2d-%.2d-%.2d%.2d%.2d:%s", currenttime_utc->tm_year+1900, currenttime_utc->tm_mon+1, currenttime_utc->tm_mday, currenttime_utc->tm_hour, currenttime_utc->tm_min, currenttime_utc->tm_sec, msgtext) < 0)
      {
            return 2;//todo?
      }

      fflush(logfile);
      return 0;
}

int logCloseFile(void)
{
      if (fclose(logfile)) //fclose returns 0 for success
      {
            return 1;
      }
      return 0;
}