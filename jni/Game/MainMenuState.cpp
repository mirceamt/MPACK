#include "MainMenuState.hpp"

#include "Application.hpp"
#include "TimeService.hpp"
#include "Texture2D.hpp"
#include "Global.hpp"
#include "GUIMenu.hpp"
#include "Particles.hpp"

using namespace Core;

namespace Game
{
	enum
	{
		GUIMENU_NOTHING,
		GUIMENU_BUTTON_NEWGAME,
		GUIMENU_BUTTON_HIGHSCORE,
		GUIMENU_BUTTON_CREDITS,
		GUIMENU_BUTTON_EXIT
	};

	MainMenu::MainMenu()
	{
		m_pButtonTexture=new Texture2D();
		m_pButtonTexture->Load("@GUI_Button.tga",Bilinear);

		m_pButtonSprite=new Sprite();
		m_pButtonSprite->SetTexture(m_pButtonTexture);

		m_pMenuGUI=new GUIMenu;
		m_pMenuGUI->m_sprite=m_pButtonSprite;

		m_pMenuGUI->AddButton("New Game",GUIMENU_BUTTON_NEWGAME);
		m_pMenuGUI->AddButton("Highscore",GUIMENU_BUTTON_HIGHSCORE);
		m_pMenuGUI->AddButton("Credits",GUIMENU_BUTTON_CREDITS);
		m_pMenuGUI->AddButton("Exit",GUIMENU_BUTTON_EXIT);
		m_pMenuGUI->BuildLayout();

		Texture2D *m_particleTex;
		ParticleEmitterCircle *m_pEmitter;

		m_particleTex = new Texture2D;
		m_particleTex->Load("@GUI_Button.tga");

		m_pEmitter = new ParticleEmitterCircle;

		m_pEmitter->SetAngularVelocity(5.0f,10.0f);
		m_pEmitter->SetParticleLife(10.0f,13.0f);
		m_pEmitter->SetVelocity(10.0f,15.0f);
		m_pEmitter->SetTexture(m_particleTex);
		m_pEmitter->SetPosition(Vector2f(200.0f,200.0f));
		m_pEmitter->SetSpawnDelay(1.0f/40.0f);
	}

	int MainMenu::Update()
	{
		float lTimeStep = Global::pContext->pTimeService->Elapsed();

		int action=m_pMenuGUI->Update(lTimeStep);
		switch(action)
		{
			case GUIMENU_BUTTON_NEWGAME:
				return EVENT_MAINMENU_NEWGAME;
			break;
			case GUIMENU_BUTTON_HIGHSCORE:
				return EVENT_MAINMENU_HIGHSCORE;
			break;
			case GUIMENU_BUTTON_CREDITS:
				return EVENT_MAINMENU_CREDITS;
			break;
			case GUIMENU_BUTTON_EXIT:
				return EVENT_MAINMENU_EXIT;
			break;
		}

		return EVENT_NOTHING;
	}

	void MainMenu::Render()
	{
		m_pMenuGUI->Render();
	}

	MainMenu::~MainMenu()
	{
		delete m_pMenuGUI;
		delete m_pButtonSprite;
		delete m_pButtonTexture;
	}
}