#ifndef WATEROBJECT_HPP
#define WATEROBJECT_HPP

#include <vector>
#include "WaterSpring.hpp"
#include "WaterObject.hpp"

#include "DemoApplication.hpp"
#include "Context.hpp"

using namespace MPACK;
using namespace MPACK::Graphics;

class WaterObject
{
public:
									WaterObject();

	void							Update(float dtime);
	void 							Render();
	int 							GetSpringsCount();
	void 							Splash(int index, Vector2f velocity);
	void							ClickSplash(Vector2f pos);

									~WaterObject();


	static const Vector2f 			s_targetHeight;
	const float 					m_spread;

	static const float				s_dampening;





private:

	Texture2D*						m_pWhiteTexture;

	void 							CreateWavesVertices();
	void 							CreateSprings();
	std::vector <WaterSpring>		m_springs;

	static WaterObject*				g_water;
	static const int 				m_springsCount;

};

#endif