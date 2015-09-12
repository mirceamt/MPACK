#include "Object.hpp"

#include "Global.hpp"

#include <algorithm>

vector<Object*> Object::s_objects;

Object::Object()
	: m_debugInCollision(true)
{
	s_objects.push_back(this);
}

Object::~Object()
{
	for(vector<Object*>::iterator it=s_objects.begin();it!=s_objects.end();++it)
	{
		if(this==*it)
		{
			LOGD("DELETED_START");
			*it=s_objects[s_objects.size()-1];
			s_objects.pop_back();
			LOGD("DELETED_END");
			break;
		}
	}
}

void Object::UpdateAll(GLfloat delta)
{
	vector<Object*> objects;
	for(vector<Object*>::iterator it=s_objects.begin();it!=s_objects.end();++it)
	{
		if((*it)->Update(delta))
		{
			objects.push_back(*it);
		}
		else
		{
			delete *it;
		}
	}
	s_objects=objects;
}

void Object::RenderAll()
{
	for(vector<Object*>::iterator it=s_objects.begin();it!=s_objects.end();++it)
	{
		(*it)->Render();
	}
}
