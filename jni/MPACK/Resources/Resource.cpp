#include "Resource.hpp"

#include "Asset.hpp"
#include "SDInputFile.hpp"
#include "MVFS.hpp"
#include "MVFSInputFile.hpp"
#include "MVFSDataBase.hpp"
#include "StringEx.hpp"

using namespace std;

namespace MPACK
{
	namespace Core
	{
		const int PATH_BUFFER_SIZE=256;

		Resource::Resource(const char* pPath)
		{
			mPath = new char[strlen(pPath)+2];
			strcpy(mPath,pPath);
		}

		const char* Resource::GetPath()
		{
			return mPath;
		}

		Resource::~Resource()
		{
			delete[] mPath;
		}

		Resource* LoadResource(const char* pPath)
		{
			const char* temp = pPath;
			if(pPath[0]=='[')
			{
				int id=0;
				++pPath;
				while('0'<=(*pPath) && (*pPath)<='9')
				{
					id*=10;
					id+=(*pPath)-'0';
					++pPath;
				}
				if((*pPath)!=']')
				{
					LOGE("LoadResource: invalid path %s",temp);
					return NULL;
				}
				++pPath;
				MVFS::Reader *pReader = MVFSDB::Get(id);
				if(pReader == NULL)
				{
					LOGE("LoadResource: invalid path %s MVFS database with id=%d does not exists",temp,id);
					return NULL;
				}
				return (Resource*)(new MVFSInputFile(MVFS::Open(pReader, pPath),true));
			}
	#ifdef ANDROID_PLATFORM
			if(pPath[0]=='@')
			{
				return (Resource*)(new Asset(pPath+1));
			}
			if(pPath[0]=='&')
			{
				return (Resource*)(new SDInputFile(pPath+1));
			}
	#elif	defined(WINDOWS_PLATFORM) || defined(LINUX_PLATFORM)
			char pathBuffer[PATH_BUFFER_SIZE];
			if(pPath[0]=='@')
			{
				strcpy(pathBuffer,"assets/");
				strcat(pathBuffer,pPath+1);
			}
			if(pPath[0]=='&')
			{
				strcpy(pathBuffer,pPath+1);
			}
			return (Resource*)(new SDInputFile(pathBuffer));
	#endif
			LOGE("LoadResource: invalid path %s",pPath);
			return NULL;
		}

		string GetResourcePath(string path)
		{
	#ifdef ANDROID_PLATFORM
			if(path[0]=='@')
			{
				return StringEx::Substring(path,1);
			}
			if(path[0]=='&')
			{
				return StringEx::Substring(path,1);
			}
	#elif	defined(WINDOWS_PLATFORM) || defined(LINUX_PLATFORM)
			if(path[0]=='@')
			{
				return string("assets/")+StringEx::Substring(path,1);
			}
			if(path[0]=='&')
			{
				return StringEx::Substring(path,1);
			}
	#endif
			return path;
		}
	}
}
