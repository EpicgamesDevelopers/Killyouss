// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "CorePrivatePCH.h"
#include "HTML5Application.h"
#include "HTML5Cursor.h"
#include "HTML5InputInterface.h"


#if PLATFORM_HTML5_BROWSER
#include "emscripten.h"
#include "html5.h"

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
	return 1;
}

#endif 

static const uint32 MaxWarmUpTicks = 10; 

FHTML5Application* FHTML5Application::CreateHTML5Application()
{
	return new FHTML5Application();
}

FHTML5Application::FHTML5Application()
	: GenericApplication( MakeShareable( new FHTML5Cursor() ) )
	, ApplicationWindow(FHTML5Window::Make())
	, InputInterface( FHTML5InputInterface::Create(MessageHandler, Cursor ) )
	, WarmUpTicks(-1)
{

#if PLATFORM_HTML5_BROWSER
    // full screen will only be requested after the first click after the window gains focus. 
    // the problem is that because of security/UX reasons browsers don't allow pointer lock in main loop
    // but only through callbacks generated by browser. 

 	// work around emscripten bug where deffered browser requests are not called if there are no callbacks.
	emscripten_set_mousedown_callback("#canvas",0,1,mouse_callback);
#endif 

#if PLATFORM_HTML5_WIN32
	// these events don't happen on startup. 
	// To warm up the app correctly push these events on startup. 

	SDL_ActiveEvent Event2; 
	Event2.gain = 1; 
	Event2.state = SDL_APPMOUSEFOCUS;
	Event2.type = SDL_ACTIVEEVENT; 
	SDL_PushEvent((SDL_Event*)&Event2);

	SDL_ActiveEvent Event; 
	Event.gain = 1; 
	Event.state = SDL_APPINPUTFOCUS;
	Event.type = SDL_ACTIVEEVENT; 
	SDL_PushEvent((SDL_Event*)&Event);

#endif 

#if PLATFORM_HTML5_BROWSER

	SDL_WindowEvent Event ;
	Event.type = SDL_WINDOWEVENT; 
	Event.event = SDL_WINDOWEVENT_FOCUS_GAINED;
    SDL_PushEvent((SDL_Event*)&Event);

#endif 


}


void FHTML5Application::SetMessageHandler( const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler )
{
	GenericApplication::SetMessageHandler(InMessageHandler);
	InputInterface->SetMessageHandler( MessageHandler );

}

void FHTML5Application::PollGameDeviceState( const float TimeDelta )
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event)) 
	{
		// Tick Input Interface. 
		switch (Event.type)
		{
			case SDL_VIDEORESIZE:
				{
					SDL_ResizeEvent *r = (SDL_ResizeEvent*)&Event;

					if ( r->h > 0 )
					{
						MessageHandler->OnSizeChanged(  ApplicationWindow, r->w,r->h); 
					} 
					else 
					{
						// Ask the game to reshape the window. 
						int w = r->w;  
						int h = -r->h;
						ANSICHAR command[1048]; 
						FCStringAnsi::Sprintf(command, "r.setRes %dx%d ", w, h);

						IConsoleManager::Get().ProcessUserConsoleInput(ANSI_TO_TCHAR(command), *GWarn, NULL );
					}

					// Slate needs to know when desktop size changes.
					FDisplayMetrics DisplayMetrics;
					FDisplayMetrics::GetDisplayMetrics(DisplayMetrics);
					BroadcastDisplayMetricsChanged(DisplayMetrics);
				}
				break;
#if PLATFORM_HTML5_WIN32
			case SDL_ACTIVEEVENT:
				{
					EWindowActivation::Type ActivationType;
					static bool DontSkip = true; 
				
					if (Event.active.gain && Event.active.state & (SDL_APPINPUTFOCUS)) {

						ActivationType = EWindowActivation::Activate;
						MessageHandler->OnWindowActivationChanged( ApplicationWindow, ActivationType );
						WarmUpTicks = 0; 

					} 
					else if (Event.active.gain && Event.active.state & (SDL_APPMOUSEFOCUS)) 
					{
						ActivationType = EWindowActivation::ActivateByMouse;
						MessageHandler->OnWindowActivationChanged( ApplicationWindow, ActivationType );
					}
					else if ( !Event.active.gain && Event.active.state & (SDL_APPINPUTFOCUS ) )
					{
							ActivationType = EWindowActivation::Deactivate;
							MessageHandler->OnWindowActivationChanged( ApplicationWindow, ActivationType );
							WarmUpTicks = 0; 
					}
				}

			break;
#endif 

#if PLATFORM_HTML5_BROWSER

			case SDL_WINDOWEVENT: 
				{
					SDL_WindowEvent windowEvent = Event.window;

					switch (windowEvent.event)
					{
                         // first two events don't exist in the SDK yet, see open pull request https://github.com/kripken/emscripten/pull/2704
						case SDL_WINDOWEVENT_ENTER:
							{
									MessageHandler->OnCursorSet();
									MessageHandler->OnWindowActivationChanged(ApplicationWindow, EWindowActivation::ActivateByMouse);
							}
							break;

						case SDL_WINDOWEVENT_LEAVE:
							{
									MessageHandler->OnWindowActivationChanged(ApplicationWindow, EWindowActivation::Deactivate);
							}
							break;

						case SDL_WINDOWEVENT_FOCUS_GAINED:
							{
									MessageHandler->OnWindowActivationChanged(ApplicationWindow, EWindowActivation::Activate);
									WarmUpTicks = 0;
							}
							break;

						case SDL_WINDOWEVENT_FOCUS_LOST:
							{
									MessageHandler->OnWindowActivationChanged(ApplicationWindow, EWindowActivation::Deactivate);
									WarmUpTicks = 0;
							}
							break;
						default:
							break;
					}
				}
#endif
			default:
			{
				InputInterface->Tick( TimeDelta,Event, ApplicationWindow);
			}
		}
	}
	InputInterface->SendControllerEvents();


	if ( WarmUpTicks >= 0)
		WarmUpTicks ++; 


	if ( WarmUpTicks == MaxWarmUpTicks  )
	{
        // browsers don't allow locking and hiding to work independently. use warmup ticks after the application has settled
        // on its mouse lock/visibility status.  This is necessary even in cases where the game doesn't want to locking because 
        // the lock status oscillates for few ticks before settling down. This causes a Browser UI pop even when we don't intend to lock.
        // see http://www.w3.org/TR/pointerlock more for information. 
#if PLATFORM_HTML5_WIN32
		if (((FHTML5Cursor*)Cursor.Get())->LockStatus && !((FHTML5Cursor*)Cursor.Get())->CursorStatus)
		{
			SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(SDL_DISABLE);
		}
		else
		{
			SDL_ShowCursor(SDL_ENABLE);
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		}
#endif 

#if PLATFORM_HTML5_BROWSER
		if (((FHTML5Cursor*)Cursor.Get())->LockStatus && !((FHTML5Cursor*)Cursor.Get())->CursorStatus)
		{
			emscripten_request_pointerlock ( "#canvas" , true);
		}
		else
		{
			emscripten_exit_pointerlock(); 
		}
#endif 

		WarmUpTicks = -1; 
	}
}

FPlatformRect FHTML5Application::GetWorkArea( const FPlatformRect& CurrentWindow ) const
{
	return  FHTML5Window::GetScreenRect();
}

void FDisplayMetrics::GetDisplayMetrics(FDisplayMetrics& OutDisplayMetrics)
{
	OutDisplayMetrics.PrimaryDisplayWorkAreaRect = FHTML5Window::GetScreenRect();
	OutDisplayMetrics.VirtualDisplayRect    =	OutDisplayMetrics.PrimaryDisplayWorkAreaRect;
	OutDisplayMetrics.PrimaryDisplayWidth   =	OutDisplayMetrics.PrimaryDisplayWorkAreaRect.Right;
	OutDisplayMetrics.PrimaryDisplayHeight  =	OutDisplayMetrics.PrimaryDisplayWorkAreaRect.Bottom; 
}

TSharedRef< FGenericWindow > FHTML5Application::MakeWindow()
{
	return ApplicationWindow;
}
