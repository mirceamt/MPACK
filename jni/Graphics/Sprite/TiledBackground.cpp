#include "TiledBackground.hpp"

#include "Render.hpp"
#include "Camera2D.hpp"

TiledBackground::TiledBackground()
	: m_depth(2.0f), m_isCameraEnabled(false)
{
}

TiledBackground::~TiledBackground()
{
}

void TiledBackground::Render()
{
	//This thing here is pure voodoo and should be left as it is ...
	SetSize(Render::GetScreenWidth()*2.0f,Render::GetScreenHeight()*2.0f);

	GLfloat Ulength=Render::GetScreenWidth()*2.0f/m_texture->GetWidth();
	GLfloat Vlength=Render::GetScreenHeight()*2.0f/m_texture->GetHeight();

	GLfloat Umin=0;
	GLfloat Umax=Umin+Ulength;
	GLfloat Vmin=0;
	GLfloat Vmax=Vmin+Vlength;

	Math::Vector2f center(Render::GetScreenWidth()*0.5f,Render::GetScreenHeight()*0.5f);

	Math::Vector2f v[4];
	v[0].x=-m_width;v[0].y=-m_height;
	v[1].x=+m_width;v[1].y=-m_height;
	v[2].x=+m_width;v[2].y=+m_height;
	v[3].x=-m_width;v[3].y=+m_height;

	if(m_isCameraEnabled && Global::pActiveCamera)
	{
		Vector2f position=Global::pActiveCamera->GetPosition();
		position/=m_depth;

		position.y*=-1.0f;

		position.x/=m_texture->GetWidth();
		position.y/=m_texture->GetHeight();

		swap(position.x,position.y);

		Umin+=position.y;
		Umax+=position.y;
		Vmin+=position.x;
		Vmax+=position.x;


		for(int i=0;i<4;++i)
		{
			v[i].Rotate(-Global::pActiveCamera->GetDirection().Angle());
		}
	}

	v[0]+=center;
	v[1]+=center;
	v[2]+=center;
	v[3]+=center;

	swap(v[0],v[3]);
	swap(v[1],v[2]);
	SpriteVertex vertexData[]={	SpriteVertex(v[0].x,v[0].y,	Umin,Vmin,	m_color[0].x,m_color[0].y,m_color[0].z,m_color[0].w,	m_spriteShadingType),
								SpriteVertex(v[1].x,v[1].y,	Umax,Vmin,	m_color[1].x,m_color[1].y,m_color[1].z,m_color[1].w,	m_spriteShadingType),
								SpriteVertex(v[2].x,v[2].y,	Umax,Vmax,	m_color[2].x,m_color[2].y,m_color[2].z,m_color[2].w,	m_spriteShadingType),
								SpriteVertex(v[3].x,v[3].y,	Umin,Vmax,	m_color[3].x,m_color[3].y,m_color[3].z,m_color[3].w,	m_spriteShadingType) };

	SpriteBatcher::Send(vertexData,4,m_texture,m_layer);
}

void TiledBackground::EnableCamera()
{
	m_isCameraEnabled=true;
}

void TiledBackground::DisableCamera()
{
	m_isCameraEnabled=false;
}

bool TiledBackground::IsCameraEnabled() const
{
	return m_isCameraEnabled;
}

void TiledBackground::SetDepth(const GLfloat depth)
{
	m_depth=depth;
}

GLfloat TiledBackground::GetDepth() const
{
	return m_depth;
}
