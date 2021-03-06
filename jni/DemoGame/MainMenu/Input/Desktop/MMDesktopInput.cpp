#if defined(WINDOWS_PLATFORM) || defined(LINUX_PLATFORM)

#include "MMDesktopInput.hpp"

#include "Global.hpp"
#include "Context.hpp"
#include "InputService.hpp"

using namespace std;
using namespace MPACK;
using namespace MPACK::Core;

MMDesktopInput::MMDesktopInput()
	: m_pFinger(NULL)
{
}

MMDesktopInput::~MMDesktopInput()
{
}

void MMDesktopInput::Update(GLfloat delta)
{
	if(m_pFinger)
	{
		m_pFinger->m_pos=Global::pContext->pInputService->GetMouse()->GetPosition();
	}

	if(Global::pContext->pInputService->GetMouse()->ButtonDown(MPACK::Input::MBC_LEFT))
	{
			m_pFinger=new MPACK::Input::Finger();
			m_pFinger->m_flag=MPACK::Input::Finger::FREE;
			m_pFinger->m_pos=Global::pContext->pInputService->GetMouse()->GetPosition();

			for(vector<Param2PtrCallbackStruct>::iterator it=m_callbackFunc_FDOWN.begin();it!=m_callbackFunc_FDOWN.end();++it)
			{
				it->function(it->param1,m_pFinger);
			}
	}
	else if(Global::pContext->pInputService->GetMouse()->ButtonUp(MPACK::Input::MBC_LEFT))
	{
		for(vector<Param2PtrCallbackStruct>::iterator it=m_callbackFunc_FUP.begin();it!=m_callbackFunc_FUP.end();++it)
		{
			it->function(it->param1,m_pFinger);
		}

		delete m_pFinger;
		m_pFinger=NULL;
	}
}

#endif
