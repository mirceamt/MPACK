#ifdef LINUX_PLATFORM

#include "LinuxEventLoop.hpp"

#include "ActivityHandler.hpp"
#include "Context.hpp"
#include "InputService.hpp"
#include "TimeService.hpp"
#include "Global.hpp"
#include "Profiler.hpp"
#include "Render.hpp"
#include "Log.hpp"

using namespace std;

namespace MPACK
{
	namespace Core
	{
		typedef GLXContext ( * PFNGLXCREATECONTEXTATTRIBSARBPROC) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);


		LinuxEventLoop::LinuxEventLoop(void *data)
			: m_isFullscreen(false), m_isRunning(false), m_width(0), m_height(0), m_enabled(false)
		{
		}

		ReturnValue LinuxEventLoop::Run(ActivityHandler* pActivityHandler)
		{
			m_pActivityHandler = pActivityHandler;
			m_isRunning=true;

			if(InitializeDisplay() == RETURN_VALUE_KO)
			{
				LOGE("LinuxEventLoop::Run failed to InitializeDisplay()");
				return RETURN_VALUE_KO;
			}
			m_pActivityHandler->onActivate();

			// Global step loop.
			while(m_isRunning)
			{
				ProcessEvents();
				if(m_pActivityHandler->onStep() != RETURN_VALUE_OK)
				{
					m_isRunning=false;
				}
				SwapBuffers();
			}

			m_pActivityHandler->onDeactivate();
			DestroyDisplay();
			return RETURN_VALUE_OK;
		}

		void LinuxEventLoop::ShowCursor()
		{
			Display* mainDisplay = XOpenDisplay(NULL);

			XUndefineCursor(mainDisplay, m_XWindow);

			XCloseDisplay(mainDisplay);
		}

		void LinuxEventLoop::HideCursor()
		{
			Display* mainDisplay = XOpenDisplay(NULL);

			Pixmap blank;
			XColor dummy;
			char data[1] = {0};

			/* make a blank cursor */
			blank = XCreateBitmapFromData (mainDisplay, m_XWindow, data, 1, 1);
			Cursor m_cursor = XCreatePixmapCursor(mainDisplay, blank, blank, &dummy, &dummy, 0, 0);
			XFreePixmap (mainDisplay, blank);

			XDefineCursor(mainDisplay, m_XWindow, m_cursor);

			XCloseDisplay(mainDisplay);
		}

		void* LinuxEventLoop::GetWindowHandle() const
		{
			return (void *)(&m_XWindow);
		}

		ReturnValue LinuxEventLoop::InitializeDisplay()
		{
			int width=800;
			int height=600;
			int bpp=32;
			bool fullscreen=false;

			m_display = XOpenDisplay(0);

			if (m_display == NULL)
			{
				LOGE("Could not open the display");
				return RETURN_VALUE_KO;
			}

			m_screenID = DefaultScreen(m_display); //Get the default screen id
			Window root = RootWindow(m_display, m_screenID);

			int n = 0, modeNum = 0;
			//Get a framebuffer config using the default attributes
			GLXFBConfig framebufferConfig = (*glXChooseFBConfig(m_display, m_screenID, 0, &n));

			XF86VidModeModeInfo **modes;
			if (!XF86VidModeGetAllModeLines(m_display, m_screenID, &modeNum, &modes))
			{
				LOGE("Could not query the video modes");
				return RETURN_VALUE_KO;
			}

			m_XF86DeskMode = *modes[0];

			int bestMode = -1;
			for (int i = 0; i < modeNum; i++)
			{
				if ((modes[i]->hdisplay == width) &&
					(modes[i]->vdisplay == height))
				{
					bestMode = i;
				}
			}

			if (bestMode == -1)
			{
				LOGE("Could not find a suitable graphics mode");
				return RETURN_VALUE_KO;
			}


			int doubleBufferedAttribList [] = {
				GLX_RGBA, GLX_DOUBLEBUFFER,
				GLX_RED_SIZE, 4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE, 4,
				GLX_ALPHA_SIZE, 4,
				GLX_DEPTH_SIZE, 16,
				None
			};

			XVisualInfo* vi = NULL;
			//Attempt to create a double buffered window
			vi = glXChooseVisual(m_display, m_screenID, doubleBufferedAttribList);

			if (vi == NULL)
			{
				LOGE("Could not create a double buffered window");
				return RETURN_VALUE_KO;
			}

			Colormap cmap = XCreateColormap(m_display, root, vi->visual, AllocNone);

			m_XSetAttr.background_pixel = 0;
			m_XSetAttr.border_pixel = 0;
			m_XSetAttr.colormap = cmap;
			m_XSetAttr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
										StructureNotifyMask;
			m_XSetAttr.override_redirect = False;

			unsigned long windowAttributes = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;

			if (fullscreen)
			{
				windowAttributes = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;

				XF86VidModeSwitchToMode(m_display, m_screenID, modes[bestMode]);
				XF86VidModeSetViewPort(m_display, m_screenID, 0, 0);
				m_XSetAttr.override_redirect = True;
			}

			m_XWindow = XCreateWindow(m_display, root,
										  0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
										  windowAttributes, &m_XSetAttr);

			{
				XSizeHints sizehints;
				sizehints.x = 0;
				sizehints.y = 0;
				sizehints.width  = width;
				sizehints.height = height;
				sizehints.flags = USSize | USPosition;
				XSetNormalHints(m_display, m_XWindow, &sizehints);
			}


			//Create a GL 2.1 context
			GLXContext m_glContext = glXCreateContext(m_display, vi, 0, GL_TRUE);

			if (m_glContext == NULL)
			{
				LOGE("Could not create a GL 2.1 context, please check your graphics drivers");
				return RETURN_VALUE_KO;
			}

			m_GL3Supported = false; //we're not using GL3.0 here!

			string title = "MPACK";

			if (fullscreen)
			{
				XWarpPointer(m_display, None, m_XWindow, 0, 0, 0, 0, 0, 0);
				XMapRaised(m_display, m_XWindow);
				XGrabKeyboard(m_display, m_XWindow, True, GrabModeAsync, GrabModeAsync, CurrentTime);
				XGrabPointer(m_display, m_XWindow, True, ButtonPressMask,
							 GrabModeAsync, GrabModeAsync, m_XWindow, None, CurrentTime);

				m_isFullscreen = true;
			}
			else
			{
				Atom wmDelete = XInternAtom(m_display, "WM_DELETE_WINDOW", True);
				XSetWMProtocols(m_display, m_XWindow, &wmDelete, 1);

				XSetStandardProperties(m_display, m_XWindow, title.c_str(), None, 0, NULL, 0, NULL);
				XMapRaised(m_display, m_XWindow);
			}

			XFree(modes);

			//Make the new context current
			glXMakeCurrent(m_display, m_XWindow, m_glContext);

			typedef int (*PFNGLXSWAPINTERVALMESA)(int interval);
			PFNGLXSWAPINTERVALMESA glXSwapIntervalMESA = NULL;

			glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESA)glXGetProcAddress((unsigned char*)"glXSwapIntervalMESA");
			if( !glXSwapIntervalMESA )
			{
				 return RETURN_VALUE_KO;
			}

			glXSwapIntervalMESA(0);
			return RETURN_VALUE_OK;
		}

		void LinuxEventLoop::DestroyDisplay()
		{
			if(m_glContext)
			{
				glXMakeCurrent(m_display, None, NULL);
				glXDestroyContext(m_display, m_glContext);
				m_glContext = NULL;
			}

			if(m_isFullscreen)
			{
				XF86VidModeSwitchToMode(m_display, m_screenID, &m_XF86DeskMode);
				XF86VidModeSetViewPort(m_display, m_screenID, 0, 0);
			}
		}

		void LinuxEventLoop::ProcessEvents()
		{
			Global::pContext->pTimeService->Update();
			Global::pContext->pInputService->Update();

			XEvent event;

			while (XPending(m_display) > 0)
			{
				XNextEvent(m_display, &event);
				switch (event.type)
				{
				case Expose:
					break;
				case ConfigureNotify:
				{
					int width = event.xconfigure.width;
					int height = event.xconfigure.height;
					if(width!=m_width || height!=m_height)
					{
						m_width=width;
						m_height=height;
						m_pActivityHandler->onDeactivate();
						Graphics::Render::SetScreenSize(m_width,m_height);
						m_pActivityHandler->onActivate();
					}
				}
				break;
				case KeyPress:
				{
					Global::pContext->pInputService->GetKeyboard()->HandleKeyDown(Global::pContext->pInputService->GetKeyboard()->TranslateCode(XLookupKeysym(&event.xkey,0)));
				}
				break;
				case KeyRelease:
				{
					Global::pContext->pInputService->GetKeyboard()->HandleKeyUp(Global::pContext->pInputService->GetKeyboard()->TranslateCode(XLookupKeysym(&event.xkey,0)));
				}
				break;

				case ClientMessage:
					if (string(XGetAtomName(m_display, event.xclient.message_type)) == string("WM_PROTOCOLS"))
					{
						std::cout << "Closing the Window!" << std::endl;
						m_isRunning = false;
						return;
					}
					break;
				default:
					break;
				}
			}
		}

		void LinuxEventLoop::SwapBuffers()
		{
			Profiler::Begin("SwapBuffers");
			glXSwapBuffers(m_display, m_XWindow);
			Profiler::End();
		}
	}
}

#endif


