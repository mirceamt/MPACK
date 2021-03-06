/*
	This file is owned by Murtaza Alexandru and may not be distributed, edited or used without written permission of the owner
	When using this work you accept to keep this header
	E-mails from murtaza_alexandru73@yahoo.com with permissions can be seen as valid.
*/



#ifndef MPACK_INDEXARRAY_HPP
#define MPACK_INDEXARRAY_HPP

#include <vector>

#include "Types.hpp"

namespace MPACK
{
	namespace Graphics
	{
		class MeshData;

		class IndexArray
		{
		public:
			IndexArray();
			~IndexArray();

			GLuint*	GetPointer();

			GLuint	GetSize();
	
			GLenum	GetPrimitiveType();

			void	SetPrimitiveType(GLuint primitiveType);
			void	AddIndex(GLuint index);
			void	Clear();
		protected:
			std::vector<GLuint> m_data;
			GLenum				m_primitiveType;
		};
	}
}

#endif
