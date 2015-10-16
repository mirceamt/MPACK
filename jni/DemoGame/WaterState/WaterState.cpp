#include "WaterState.hpp"

#include "Render.hpp"
#include "InputService.hpp"
#include "TimeService.hpp"
#include "DemoApplication.hpp"
#include "TextureMappedFont.hpp"
#include "Global.hpp"
#include "Context.hpp"
#include "Particles.hpp"
#include "Camera2D.hpp"
#include "WaterObject.hpp"

#include <fstream>

using namespace MPACK;
using namespace MPACK::Algorithm;
using namespace MPACK::Math;
using namespace MPACK::Core;
using namespace MPACK::Global;
using namespace MPACK::Graphics;
using namespace MPACK::Physics;
using namespace MPACK::UI;

namespace Game
{
	WaterState::WaterState()
	:m_timeBetweenWaves(0.3f)
	,m_timeSinceLastWave(0.f)
	{
		m_pWSInputController = WSInputController::Initialize();
		Global::pActiveCamera = new Camera2D();

		m_pBackgroundTexture = new Texture2D();
		m_pBackgroundTexture->Load("@Backgrounds/skybk1.png");
		m_pBackgroundTexture->SetWrapMode(GL_REPEAT, GL_REPEAT);

		m_pWhiteTexture = new Texture2D();
		m_pWhiteTexture->Load("@Backgrounds/whitetexture.png");
		m_pWhiteTexture->SetWrapMode(GL_REPEAT, GL_REPEAT);

		m_pRockTexture = new Texture2D();
		m_pRockTexture->Load("@Sprites/Water/rocktexture.png");
		m_pRockTexture->SetWrapMode(GL_REPEAT, GL_REPEAT);

		m_pBackgroundSprite = new Sprite();
		m_pBackgroundSprite->SetTexture(m_pBackgroundTexture);
		m_pBackgroundSprite->SetColor(Vector4f(1.f, 1.f, 1.f, 1.f));
		m_pBackgroundSprite->m_position = Vector2f(Render::GetScreenWidth() / 2.f, Render::GetScreenHeight() / 2.f);
		m_pBackgroundSprite->SetLayer(-100000.0f);
		m_pBackgroundSprite->SetShading(SpriteVertex::ALPHA_BLEND);
		m_pBackgroundSprite->SetSize(Render::GetScreenWidth(), Render::GetScreenHeight());

		m_world = new World(1.f/60.f, 10);
	}

	int WaterState::Update()
	{
		float dtime = Global::pContext->pTimeService->Elapsed();

		m_timeSinceLastWave += dtime;

		if (m_timeSinceLastWave > m_timeBetweenWaves)
		{
			m_water.Splash(Random::Int(0, m_water.GetSpringsCount()), Vector2f(0.f, Random::Double(-60.f, 60.f)));
			m_timeSinceLastWave = 0.f;
		}

		m_pWSInputController->Update(dtime);

		m_water.Update(dtime);

		Camera2D::UpdateAll(dtime);

		if (m_pWSInputController->GetLeftMouseButtonPressed())
		{
			Vector2f pos = m_pWSInputController->GetMousePosition();
			m_water.ClickSplash(pos);
			//LOGI("(%f, %f)", pos.x, pos.y);
		}

		if (m_pWSInputController->GetLeftMouseButtonUp())
		{
			for (int i = 1; i <= 5; ++ i)
			{
				Vector2f position = m_pWSInputController->GetMousePosition();
				position += Vector2f(1.0f,0.0f).Rotated(Random::Double(0.0f,360.0f)) * Random::Double(0,50.0f);
				CreateRockObject(position);
			}
			//CreateRockObject(m_pWSInputController->GetMousePosition());
		}

		for (int rockIndex = 0; rockIndex < m_rockObjects.size(); ++ rockIndex)
		{
			WSRockObject *rock = m_rockObjects[rockIndex];

			rock->Update(dtime);

			float totalCoveredArea = 0.f;

			vector <Vector2f> springTriangle;
			springTriangle.resize(3);

			vector <Vector2f> rockPolygon;
			float minx, miny, maxx, maxy;
			minx = miny = 2000000000.f;
			maxx = maxy = -2000000000.f;

			for (int i = 0; i < rock->GetShape()->m_vertexCount; ++ i)
			{
				// get the updated points on the rock and store them in rockPolygon
				float rotation = rock->GetBody()->GetOrientation();
				Vector2f rockPoint = rock->GetShape()->m_vertices[i].Rotated(rotation); // rotation
				rockPoint += rock->GetBody()->GetPosition(); // translation
				rockPolygon.push_back(rockPoint);

				minx = min(minx, rockPoint.x);
				miny = min(miny, rockPoint.y);
				maxx = max(maxx, rockPoint.x);
				maxy = max(maxy, rockPoint.y);
			}


			// delete the rock if it is out of screen
			GLint screenWidth = Render::GetScreenWidth();
			GLint screenHeight = Render::GetScreenHeight();
			if (miny > 1.f * screenHeight || minx > 1.f * screenWidth || maxx < 0.f)
			{
				// the rock is out of screen and needs to be deleted
				swap(m_rockObjects[rockIndex], m_rockObjects[m_rockObjects.size() - 1]);
				m_rockObjects.pop_back();
				rock->~WSRockObject();
				continue;
			}

			// calculate the area of the rock covered by water
			vector <Vector2f> result;
			// optimize calculating the area of the rock covered by water by intersecting with the rock only the watersprings that could matter
			int leftSpringLimit, rightSpringLimit;
			float leftPercent = 1.f * minx / screenWidth;
			float rightPercent = 1.f * maxx / screenWidth;
			leftSpringLimit = leftPercent * m_water.GetSpringsCount() - 2;
			rightSpringLimit = rightPercent * m_water.GetSpringsCount() + 2;
			leftSpringLimit = max(1, leftSpringLimit);
			rightSpringLimit = min(m_water.GetSpringsCount(), rightSpringLimit);

			for (int i = leftSpringLimit; i < rightSpringLimit; ++ i)
			{
				float currentSpringIntersectionArea = 0.f;
				for (int j = 0; j < 3; ++ j)
					springTriangle[j] = m_water.m_springsVertices[i][j];

				ClipPolygon(springTriangle, rockPolygon, result);
				currentSpringIntersectionArea += PolygonArea(result);

				for (int j = 0; j < 3; ++ j)
					springTriangle[j] = m_water.m_springsVertices[i][j+3];

				ClipPolygon(springTriangle, rockPolygon, result);
				currentSpringIntersectionArea += PolygonArea(result);

				totalCoveredArea += currentSpringIntersectionArea;
			}

			float waterWeight = m_water.s_waterDensity * totalCoveredArea * 98.f;

			rock->SetLinearAcceleration(Vector2f(0.f, 98.f));
			rock->SetLinearAcceleration(Vector2f(0.f, -waterWeight));
		}


		m_world->Update(dtime);

		if (m_pWSInputController->IsUserRequestingExit())
		{
			return EVENT_WATER_EXIT;
		}
		return EVENT_NOTHING;
	}

	void WaterState::Render()
	{
		m_pBackgroundSprite->Render();

		/*SpriteVertex* vertices = new SpriteVertex[3];
		vertices[0] = SpriteVertex(0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, SpriteVertex::NONE);
		vertices[1] = SpriteVertex(0.f, 300.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, SpriteVertex::NONE);
		vertices[2] = SpriteVertex(400.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, SpriteVertex::NONE);
		GLushort* indices = new GLushort[3];

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;

		Batcher::SendSpriteVertexData(vertices, 3, indices, 3, m_pWhiteTexture, IndexData::TRIANGLES, 0.f);*/

		m_water.Render();

		for (auto &rock : m_rockObjects)
		{
			rock->Render();
			/////////////////debug////////////////
			for (int i = 0; i < rock->GetShape()->m_vertexCount; ++ i)
			{
				Vector2f debugPoint = rock->GetShape()->m_vertices[i].Rotated(rock->GetBody()->GetOrientation());
				debugPoint += rock->GetBody()->GetPosition();
				SpriteVertex debugPointVertex(debugPoint.x, debugPoint.y, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, SpriteVertex::NONE);
				GLushort pointIndex = 0;
				Batcher::SendSpriteVertexData(&debugPointVertex, 1, &pointIndex, 1, m_pWhiteTexture, IndexData::POINTS, 1.f);
			}
			//////////////debug////////////////////
		}


		m_pWSInputController->Render();
	}

	WSRockObject* WaterState::CreateRockObject(const Vector2f & pos)
	{
		WSRockObject* currentRock = new WSRockObject(m_world);

		Sprite* currentSprite = new Sprite();
		currentSprite->SetTexture(m_pRockTexture);
		currentSprite->SetColor(Vector4f(1.f, 1.f, 1.f, 1.f));
		currentSprite->SetLayer(0.f);
		currentSprite->SetShading(SpriteVertex::ALPHA_BLEND);
		currentSprite->m_position = pos;

		currentRock->SetSprite(currentSprite);

		PolygonShape* currentShape = new PolygonShape();

		ifstream f("assets/Sprites/Water/rocktexturepoints.txt");
		int count = 0;
		f >> count;
		Vector2f* vertices = new Vector2f[count];
		Vector2f offset;
		f >> offset.x >> offset.y;
		offset = -offset;

		for (int i = 0; i < count; ++ i)
		{
			Vector2f now;
			f >> now.x >> now.y;
			now += offset;
			vertices[i] = now;
		}

		currentShape->Set(vertices, count);

		currentRock->SetShape(currentShape, pos);

		delete currentShape;
		delete vertices;

		m_rockObjects.push_back(currentRock);

		return currentRock;

	}

	void WaterState::Pause()
	{

	}

	void WaterState::Continue()
	{

	}

	WaterState::~WaterState()
	{
		delete Global::pActiveCamera;
		delete m_pRockTexture;
		delete m_pBackgroundSprite;
		delete m_pBackgroundTexture;
		delete m_pWhiteTexture;
		delete m_pWSInputController;

		for (std::vector<WSRockObject*> :: iterator it = m_rockObjects.begin(); it != m_rockObjects.end(); ++ it)
		{
			delete *it;
		}

		delete m_world;
	}
}
