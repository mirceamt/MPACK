#ifndef MPACK_RESOURCEREADER_HPP
#define MPACK_RESOURCEREADER_HPP

namespace MPACK
{
	namespace Core
	{
		class Resource;
	}
}

namespace MPACK
{
	namespace Core
	{
		class ResourceReader
		{
		public:
			ResourceReader(Resource *resource, int bufferSize=DefaultBufferSize);
			~ResourceReader();

			char Char();

			bool EndOfFile();

			static const int DefaultBufferSize;
		private:
			void NextChunk();

			Resource *m_resource;

			int m_bufferIndex;
			int m_bufferSize;
			int m_bytesLeft;
			int m_bytesLeftInBuffer;
			char *m_buffer;
		};
	}
}

#endif
