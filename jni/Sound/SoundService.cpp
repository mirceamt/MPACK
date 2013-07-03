#include "SoundService.hpp"
#include "Resource.hpp"
#include "Log.hpp"

namespace Core
{
    SoundService::SoundService() :
        mEngineObj(NULL), mEngine(NULL),
        mOutputMixObj(NULL),
        mBGMPlayerObj(NULL), mBGMPlayer(NULL),
        mPlayerObj(), mPlayer(), mPlayerQueue(),
        mSounds(), mPlaylist(NULL)
    {
        LOGI("Creating SoundService.");
    }


    SoundService::~SoundService()
    {
        LOGI("Destroying SoundService.");

        for (map<string, Sound*>::iterator it = mSounds.begin(); it != mSounds.end(); ++it)
        {
        	it->second->Unload();
        	delete it->second;
        }

        mSounds.clear();
    }

    Status SoundService::Start()
    {
        LOGI("Starting SoundService.");
        SLresult lRes;
        const SLuint32      lEngineMixIIDCount = 0;
        const SLInterfaceID lEngineMixIIDs[]   = {};
        const SLboolean     lEngineMixReqs[]   = {};
        const SLuint32      lOutputMixIIDCount = 0;
        const SLInterfaceID lOutputMixIIDs[]   = {};
        const SLboolean     lOutputMixReqs[]   = {};

        // Creates OpenSL ES engine and dumps its capabilities.
        lRes = slCreateEngine(&mEngineObj, 0, NULL, lEngineMixIIDCount, lEngineMixIIDs, lEngineMixReqs);
        if (lRes != SL_RESULT_SUCCESS)
        {
        	goto ERROR;
        }
        lRes = (*mEngineObj)->Realize(mEngineObj,SL_BOOLEAN_FALSE);
        if (lRes != SL_RESULT_SUCCESS)
        {
        	goto ERROR;
        }
        lRes = (*mEngineObj)->GetInterface(mEngineObj, SL_IID_ENGINE, &mEngine);
        if (lRes != SL_RESULT_SUCCESS)
        {
        	goto ERROR;
        }

        // Creates audio output.
        lRes = (*mEngine)->CreateOutputMix(mEngine, &mOutputMixObj, lOutputMixIIDCount, lOutputMixIIDs, lOutputMixReqs);
        lRes = (*mOutputMixObj)->Realize(mOutputMixObj, SL_BOOLEAN_FALSE);

        // Set-up sound player.
        if (StartSoundPlayers() != STATUS_OK)
        {
        	goto ERROR;
        }

        if (StartBGMPlayer() != STATUS_OK)
        {
			goto ERROR;
		}


        return STATUS_OK;

    ERROR:
        LOGE("Error while starting SoundService");
        Stop();
        return STATUS_KO;
    }

    void SoundService::Stop()
    {
    	LOGI("Stopping SoundService.");

        // Stops and destroys BGM player.
        //StopBGM();
    	if (mBGMPlayerObj != NULL)
		{
			(*mBGMPlayerObj)->Destroy(mBGMPlayerObj);
			mBGMPlayerObj = NULL; mBGMPlayer = NULL; mBGMPlayerQueue = NULL;
		}

        // Destroys sound player.
        for(int i = 0; i < MAX_SOUNDS; i++)
        {
			if (mPlayerObj[i] != NULL)
			{
				(*mPlayerObj[i])->Destroy(mPlayerObj[i]);
				mPlayerObj[i] = NULL; mPlayer[i] = NULL; mPlayerQueue[i] = NULL;
			}
    	}

        // Destroys audio output and engine.
        if (mOutputMixObj != NULL)
        {
            (*mOutputMixObj)->Destroy(mOutputMixObj);
            mOutputMixObj = NULL;
        }
        if (mEngineObj != NULL)
        {
            (*mEngineObj)->Destroy(mEngineObj);
            mEngineObj = NULL; mEngine = NULL;
        }

        for (map<string, Sound*>::iterator it = mSounds.begin(); it != mSounds.end(); ++it)
		{
        	it->second->Unload();
        	delete it->second;
		}

		mSounds.clear();
    }

    Status SoundService::StartSoundPlayers()
    {
    	LOGI("Starting sound player.");
        SLresult lRes;

        // Set-up sound audio source.
        SLDataLocator_AndroidSimpleBufferQueue lDataLocatorIn;
        lDataLocatorIn.locatorType =
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
        // At most one buffer in the queue.
        lDataLocatorIn.numBuffers = 1;

        SLDataFormat_PCM lDataFormat;
        lDataFormat.formatType = SL_DATAFORMAT_PCM;
        lDataFormat.numChannels = 1; // Mono sound.
        lDataFormat.samplesPerSec = SL_SAMPLINGRATE_22_05;
        lDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
        lDataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
        lDataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
        lDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

        SLDataSource lDataSource;
        lDataSource.pLocator = &lDataLocatorIn;
        lDataSource.pFormat = &lDataFormat;

        SLDataLocator_OutputMix lDataLocatorOut;
        lDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        lDataLocatorOut.outputMix = mOutputMixObj;

        SLDataSink lDataSink;
        lDataSink.pLocator = &lDataLocatorOut;
        lDataSink.pFormat = NULL;

        // Creates the sounds player and retrieves its interfaces.
        const SLuint32 lSoundPlayerIIDCount = 1;
        const SLInterfaceID lSoundPlayerIIDs[] =
            { SL_IID_BUFFERQUEUE };
        const SLboolean lSoundPlayerReqs[] =
            { SL_BOOLEAN_TRUE };

        for(int i = 0; i < MAX_SOUNDS; i++){
			lRes = (*mEngine)->CreateAudioPlayer(mEngine, &mPlayerObj[i],
				&lDataSource, &lDataSink, lSoundPlayerIIDCount,
				lSoundPlayerIIDs, lSoundPlayerReqs);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;
			lRes = (*mPlayerObj[i])->Realize(mPlayerObj[i], SL_BOOLEAN_FALSE);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;

			lRes = (*mPlayerObj[i])->GetInterface(mPlayerObj[i], SL_IID_PLAY,
				&mPlayer[i]);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;
			lRes = (*mPlayerObj[i])->GetInterface(mPlayerObj[i],
				SL_IID_BUFFERQUEUE, &mPlayerQueue[i]);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;

			lRes = (*mPlayerQueue[i])->RegisterCallback(mPlayerQueue[i], bqSFXPlayerCallback, &tempSounds[i]);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;


			tempSounds[i] = NULL;
			// Starts the sound player. Nothing can be heard while the
			// sound queue remains empty.
			lRes = (*mPlayer[i])->SetPlayState(mPlayer[i],
				SL_PLAYSTATE_PLAYING);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;
        }

        return STATUS_OK;

    ERROR:
        LOGE("Error while starting sound players");
        return STATUS_KO;
    }

    Status SoundService::StartBGMPlayer()
    {
    	LOGI("Starting bgm player.");
		SLresult lRes;


    	SLDataLocator_AndroidSimpleBufferQueue lDataLocatorIn;
		lDataLocatorIn.locatorType =
			SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
		// At most one buffer in the queue.
		lDataLocatorIn.numBuffers = 1;


		SLDataFormat_PCM lDataFormat;
		lDataFormat.formatType = SL_DATAFORMAT_PCM;
		lDataFormat.numChannels = 1; // Mono sound.
		lDataFormat.samplesPerSec = SL_SAMPLINGRATE_22_05;
		lDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		lDataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
		lDataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
		lDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;



		// Here you can set USB settings.
		SLDataSource lDataSource;
		lDataSource.pLocator = &lDataLocatorIn;
		lDataSource.pFormat  = &lDataFormat;

		SLDataLocator_OutputMix lDataLocatorOut;
		lDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		lDataLocatorOut.outputMix   = mOutputMixObj;

		SLDataSink lDataSink;
		lDataSink.pLocator = &lDataLocatorOut;
		lDataSink.pFormat  = NULL;

		// Creates BGM player and retrieves its interfaces.
		const SLuint32 lBGMPlayerIIDCount = 2;
		const SLInterfaceID lBGMPlayerIIDs[] =
			{ SL_IID_BUFFERQUEUE, SL_IID_PLAY};
		const SLboolean lBGMPlayerReqs[] =
			{ SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

		lRes = (*mEngine)->CreateAudioPlayer(mEngine,
			&mBGMPlayerObj, &lDataSource, &lDataSink,
			lBGMPlayerIIDCount, lBGMPlayerIIDs, lBGMPlayerReqs);
		if (lRes != SL_RESULT_SUCCESS) goto ERROR;
		lRes = (*mBGMPlayerObj)->Realize(mBGMPlayerObj,
			SL_BOOLEAN_FALSE);
		if (lRes != SL_RESULT_SUCCESS) goto ERROR;


		lRes = (*mBGMPlayerObj)->GetInterface(mBGMPlayerObj,
			SL_IID_PLAY, &mBGMPlayer);
		if (lRes != SL_RESULT_SUCCESS) goto ERROR;
		lRes = (*mBGMPlayerObj)->GetInterface(mBGMPlayerObj,
				SL_IID_BUFFERQUEUE, &mBGMPlayerQueue);
		if (lRes != SL_RESULT_SUCCESS) goto ERROR;

        return STATUS_OK;

    ERROR:
        LOGE("Error while starting bgm players");
        return STATUS_KO;
    }

    Status SoundService::PlayBGMPlaylist(const char* pPath, bool forced)
    {
    	SLresult lRes;

    	if(mPlaylist == NULL)
    	{
    		mPlaylist = new Playlist(pPath);
			mPlaylist->setPlayMode(FORWARD);

			// Enables looping and starts playing.
			lRes = (*mBGMPlayerQueue)->RegisterCallback(mBGMPlayerQueue, bqBGMPlayerCallback, (void*)mPlaylist);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;

			lRes = (*mBGMPlayer)->SetPlayState(mBGMPlayer,
				SL_PLAYSTATE_PLAYING);
			if (lRes != SL_RESULT_SUCCESS) goto ERROR;

			Sound *snd;
			snd = mPlaylist->GetSound();
			(*mBGMPlayerQueue)->Enqueue(mBGMPlayerQueue, snd->GetPCMData(), snd->GetPCMLength());
    	}
    	else
    	{
    	    SLuint32 state;
            (*mBGMPlayer)->GetPlayState(mBGMPlayer, &state);
            
    		if(forced || state == SL_PLAYSTATE_STOPPED)
    		{
    			lRes = (*mBGMPlayer)->SetPlayState(mBGMPlayer,
    						SL_PLAYSTATE_STOPPED);
    			if (lRes != SL_RESULT_SUCCESS) goto ERROR;
                (*mBGMPlayerQueue)->Clear(mBGMPlayerQueue);
                
    			mPlaylist->Load(pPath, forced);
                
				Sound* snd = mPlaylist->GetSound();
				(*mBGMPlayerQueue)->Enqueue(mBGMPlayerQueue, snd->GetPCMData(), snd->GetPCMLength());

    			lRes = (*mBGMPlayer)->SetPlayState(mBGMPlayer,
							SL_PLAYSTATE_PLAYING);
				if (lRes != SL_RESULT_SUCCESS) goto ERROR;

    		}
    		else
    		{
    			mPlaylist->Load(pPath, forced);
    		}

    	}

        return STATUS_OK;

    ERROR:
        return STATUS_KO;
    }

    
    void SoundService::RegisterSound(const char* pPath)
    {
    	LOGI("Registering sound");
        // Finds out if texture already loaded.
    	if(mSounds.count(pPath) == 0){
    		mSounds.insert(pair<string, Sound*>(pPath, new Sound(pPath)));
    		mSounds[pPath]->Load();
    		LOGI("Just loaded %s", pPath);
    	}
    	else
    	{
    		LOGI("Already loaded %s", pPath);
    		return;
    	}
    }

    void SoundService::UnregisterSound(const char* pPath)
    {
    	LOGI("Unregistering sound");
    	if(mSounds.count(pPath) == 1){
    		mSounds[pPath]->Unload();
    		delete mSounds[pPath];
			mSounds.erase(pPath);
			LOGI("Unloaded %s", pPath);
		}
    	else
    	{
    		LOGI("Not registered %s", pPath);
    		return;
    	}

    }

    void SoundService::PlaySFX(const char* pPath, bool load)
    {
    	if(load)
    	{
    		RegisterSound(pPath);
    	}
    	LOGI("Playing sound");
        SLresult lRes;
        SLuint32 lPlayerState;

        SLBufferQueueState state;
        int i;

        int16_t* lBuffer;
        off_t    lLength;

        map<string, Sound*>::iterator it;

        for(i = 0; i < MAX_SOUNDS - 1; i++)
        {
        	lRes = (*(mPlayerQueue[i]))->GetState(mPlayerQueue[i], &state);
        	if (lRes != SL_RESULT_SUCCESS)  goto ERROR;

        	if(state.count == 0)
        		break;
        }


        it = mSounds.find(pPath);
        if(it != mSounds.end())
        {
        	lBuffer = (int16_t*) it->second->GetPCMData();
			lLength = it->second->GetPCMLength();

			LOGI("Sound was registered %s", pPath);
        }
        else
        {
        	tempSounds[i] = new Sound(pPath);
        	tempSounds[i]->Load();

        	lBuffer = (int16_t*) tempSounds[i]->GetPCMData();
			lLength = tempSounds[i]->GetPCMLength();

			LOGI("Sound was loaded in temp %s", pPath);
			LOGI("Address %x", tempSounds[i]);
        }

        // Removes any sound from the queue.
        lRes = (*mPlayerQueue[i])->Clear(mPlayerQueue[i]);
        if (lRes != SL_RESULT_SUCCESS) goto ERROR;

        // Plays the new sound.
        lRes = (*mPlayerQueue[i])->Enqueue(mPlayerQueue[i], lBuffer, lLength);
        if (lRes != SL_RESULT_SUCCESS) goto ERROR;

        return;

    ERROR:
        LOGE("Error trying to play sound");
    }


    void SoundService::bqBGMPlayerCallback(SLBufferQueueItf bq, void *context)
    {
    	LOGI("BGM Track done. Loading next one");
    	if(((Playlist*)context)->Next())
    	{
    			Sound* snd = ((Playlist*)context)->GetSound();
    			(*bq)->Enqueue(bq, snd->GetPCMData(), snd->GetPCMLength());
    	}
    }

    void SoundService::bqSFXPlayerCallback(SLBufferQueueItf bq, void *context)
    {
    	Sound* &snd = *((Sound**)context);
    	if(snd != NULL)
    	{
    		LOGI("Unloading temp sound %s", snd->GetPath());
    		LOGI("Address %x", snd);
    		snd->Unload();
    		delete snd;
    		snd = NULL;
    	}
    }


    void SoundService::SetBGMState(SLuint32 state)
    {
       	(*mBGMPlayer)->SetPlayState(mBGMPlayer, state);
    }

    void SoundService::SetSFXState(SLuint32 state)
    {
    	for(int i = 0; i < MAX_SOUNDS; i++){
    		(*mPlayer[i])->SetPlayState(mPlayer[i], state);
    	}
    }
    
    void SoundService::StopBGM()
    {
        SetBGMState(SL_PLAYSTATE_STOPPED);
    }

    void SoundService::PauseBGM()
    {
    	SetBGMState(SL_PLAYSTATE_PAUSED);
    }

	void SoundService::PauseSFX()
	{
		SetSFXState(SL_PLAYSTATE_PAUSED);
	}

	void SoundService::PauseAll()
	{
		PauseBGM();
		PauseSFX();
	}

	void SoundService::ResumeBGM()
	{
		SetBGMState(SL_PLAYSTATE_PLAYING);
	}

	void SoundService::ResumeSFX()
	{
		SetSFXState(SL_PLAYSTATE_PLAYING);
	}

	void SoundService::ResumeAll()
	{
		ResumeBGM();
		ResumeSFX();
	}
}
