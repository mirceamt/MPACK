#include "Global.hpp"

#include "Context.hpp"

namespace MPACK
{
	namespace Global
	{
	#ifdef ANDROID_PLATFORM
		android_app 					*pAndroidApp=NULL;
		AAssetManager 					*pAAssetManager=NULL;
	#endif

		Core::Context					*pContext=NULL;
		Core::EventLoop					*pEventLoop;

		Graphics::TextureMappedFont		*pFont;

		Graphics::Camera2D				*pActiveCamera;
	}
}
