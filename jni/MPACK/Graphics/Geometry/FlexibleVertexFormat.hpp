/*
	This file is owned by Murtaza Alexandru and may not be distributed, edited or used without written permission of the owner
	When using this work you accept to keep this header
	E-mails from murtaza_alexandru73@yahoo.com with permissions can be seen as valid.
*/



#ifndef MPACK_VERTEXFLEXIBLEFORMAT_HPP
#define MPACK_VERTEXFLEXIBLEFORMAT_HPP

#include "Types.hpp"

namespace MPACK
{
	namespace Graphics
	{
		namespace FVF
		{
			extern const GLuint POSITION;
			extern const GLuint COLOR;
			extern const GLuint TEXCOORD0;
			extern const GLuint TEXCOORD1;
			extern const GLuint TEXCOORD2;
			extern const GLuint TEXCOORD3;
			extern const GLuint TEXCOORD4;
			extern const GLuint TEXCOORD5;
			extern const GLuint TEXCOORD6;
			extern const GLuint TEXCOORD7;
			extern const GLuint NORMAL;
			extern const GLuint TANGENT;

			GLuint GetSize(GLuint format);
		}
	}
}

#endif
