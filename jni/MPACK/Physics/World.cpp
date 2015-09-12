#include "World.hpp"

#include "CollisionInfo.hpp"
#include "Global.hpp"

using namespace std;
using namespace MPACK::Math;

namespace MPACK
{
	namespace Physics
	{
		World::World( float delta, int iterations )
			: m_stepDelta(delta), m_accumulator(0.0f), m_iterations(iterations)
		{
		}

		World::~World()
		{
			Clear();
		}

		void World::Update(float delta)
		{
			m_accumulator += delta;

			m_accumulator = Math::Misc<float>::Clamp(m_accumulator, 0.0f, 0.1f);
			while(m_accumulator >= m_stepDelta)
			{
				Step(m_stepDelta);
				m_accumulator -= m_stepDelta;
			}
			Step(m_accumulator);
			m_accumulator=0.0f;
		}

		void World::Step(float delta)
		{
			m_contacts.clear();
			for(unordered_set<Body*>::iterator it=m_bodies.begin();it!=m_bodies.end();++it)
			{
				Body *A=*it;

				unordered_set<Body*>::iterator it2Start=it;
				++it2Start;

				for(unordered_set<Body*>::iterator it2=it2Start;it2!=m_bodies.end();++it2)
				{
					Body *B=*it2;
					if(A->m_inverseMass==0 && B->m_inverseMass==0)
					{
						continue;
					}

					bool collide =
					          (A->maskBits & B->categoryBits) != 0 &&
					          (A->categoryBits & B->maskBits) != 0;

					if(collide)
					{
						Manifold m(A,B);
						m.Solve();
						if(m.m_contactCount)
						{
							m_contacts.emplace_back(m);
						}
					}
				}
			}

			for(unordered_set<Body*>::iterator it=m_bodies.begin();it!=m_bodies.end();++it)
			{
				(*it)->IntegrateForces(delta);
			}

			for(int i=0;i<m_contacts.size();++i)
			{
				m_contacts[i].Initialize();
			}

			for(int j=0;j<m_iterations;++j)
			{
				for(int i=0;i<m_contacts.size();++i)
				{
					m_contacts[i].ApplyImpulse();
				}
			}

			for(unordered_set<Body*>::iterator it=m_bodies.begin();it!=m_bodies.end();++it)
			{
				(*it)->IntegrateVelocity(delta);
			}

			for(int i=0;i<m_contacts.size();++i)
			{
				m_contacts[i].PositionalCorrection();
			}

			for(unordered_set<Body*>::iterator it=m_bodies.begin();it!=m_bodies.end();++it)
			{
				(*it)->ResetForces();
			}

			CollisionInfo collisionInfo;
			for(int i=0;i<m_contacts.size();++i)
			{
				for(vector<CollisionCallbackStruct>::iterator it=m_collisionCallbackFunc.begin();it!=m_collisionCallbackFunc.end();++it)
				{
					collisionInfo.contactCount=m_contacts[i].m_contactCount;
					collisionInfo.penetration=m_contacts[i].m_penetration;
					collisionInfo.normal=m_contacts[i].m_normal;
					collisionInfo.contacts[0]=m_contacts[i].m_contacts[0];
					collisionInfo.contacts[1]=m_contacts[i].m_contacts[1];
					it->function(it->param1, m_contacts[i].m_A, m_contacts[i].m_B, &collisionInfo);
				}
			}
		}

		Body* World::Add( Shape *shape, float x, float y )
		{
			Body *b = new Body(shape);
			b->m_position = Vector2f(x,y);
			m_bodies.insert(b);
			return b;
		}

		void World::Destroy(Body *body)
		{
			delete body;
			m_bodies.erase(body);
		}

		void World::Clear()
		{
			for(unordered_set<Body*>::iterator it=m_bodies.begin();it!=m_bodies.end();++it)
			{
				delete *it;
			}
			m_bodies.clear();
		}

		void World::LinkCollisionCallback(const CollisionCallbackStruct &link)
		{
			m_collisionCallbackFunc.push_back(link);
		}

		void World::UnLinkCollisionCallback(const CollisionCallbackStruct &link)
		{
			for(vector<CollisionCallbackStruct>::iterator it=m_collisionCallbackFunc.begin();it!=m_collisionCallbackFunc.end();++it)
			{
				if(*it==link)
				{
					m_collisionCallbackFunc.erase(it);
					return;
				}
			}
		}
	}
}
