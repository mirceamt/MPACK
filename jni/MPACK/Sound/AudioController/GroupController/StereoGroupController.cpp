#include "StereoGroupController.hpp"

#include "AudioPlayer.hpp"
#include "GroupController.hpp"
#include "VolumeController.hpp"
#include "StereoController.hpp"

using namespace std;
using namespace MPACK::Core;

namespace MPACK
{
	namespace Sound
	{
		StereoGroupController::StereoGroupController(GroupController *pGroupController)
			: m_pGroupController(pGroupController)
		{
		}

		StereoGroupController::~StereoGroupController()
		{
		}

		ReturnValue StereoGroupController::Enable()
		{
			ReturnValue retVal=RETURN_VALUE_OK;
			for(vector<AudioPlayer*>::iterator it=m_pGroupController->m_objects.begin();it!=m_pGroupController->m_objects.end();++it)
			{
				if((*it)->Volume()->Stereo()->Enable()==RETURN_VALUE_KO)
				{
					retVal=RETURN_VALUE_KO;
				}
			}
			return retVal;
		}

		ReturnValue StereoGroupController::Disable()
		{
			ReturnValue retVal=RETURN_VALUE_OK;
			for(vector<AudioPlayer*>::iterator it=m_pGroupController->m_objects.begin();it!=m_pGroupController->m_objects.end();++it)
			{
				if((*it)->Volume()->Stereo()->Disable()==RETURN_VALUE_KO)
				{
					retVal=RETURN_VALUE_KO;
				}
			}
			return retVal;
		}

		ReturnValue StereoGroupController::Toggle()
		{
			ReturnValue retVal=RETURN_VALUE_OK;
			for(vector<AudioPlayer*>::iterator it=m_pGroupController->m_objects.begin();it!=m_pGroupController->m_objects.end();++it)
			{
				if((*it)->Volume()->Stereo()->Toggle()==RETURN_VALUE_KO)
				{
					retVal=RETURN_VALUE_KO;
				}
			}
			return retVal;
		}

		ReturnValue StereoGroupController::SetEnabled(bool enabled)
		{
			ReturnValue retVal=RETURN_VALUE_OK;
			for(vector<AudioPlayer*>::iterator it=m_pGroupController->m_objects.begin();it!=m_pGroupController->m_objects.end();++it)
			{
				if((*it)->Volume()->Stereo()->SetEnabled(enabled)==RETURN_VALUE_KO)
				{
					retVal=RETURN_VALUE_KO;
				}
			}
			return retVal;
		}

		ReturnValue StereoGroupController::Set(SLpermille position)
		{
			ReturnValue retVal=RETURN_VALUE_OK;
			for(vector<AudioPlayer*>::iterator it=m_pGroupController->m_objects.begin();it!=m_pGroupController->m_objects.end();++it)
			{
				if((*it)->Volume()->Stereo()->Set(position)==RETURN_VALUE_KO)
				{
					retVal=RETURN_VALUE_KO;
				}
			}
			return retVal;
		}
	}
}
