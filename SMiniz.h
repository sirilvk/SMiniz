#pragma once
#include "miniz.h"

#ifdef _WIN32
#  define EXPORTIT __declspec( dllexport )
#else
#  define EXPORTIT 
#endif

#define SMINIZ_BUF_SIZE (1048576)

typedef std::function<bool(const unsigned char* data, size_t length)> WriteFunc;

namespace util
{
	class EXPORTIT SMiniz
	{
	protected:
#pragma warning(push)
#pragma warning(disable:4251)
		int _compressionLevel = Z_BEST_COMPRESSION;
		z_stream _stream;
		unsigned char _outBuf[SMINIZ_BUF_SIZE];
		WriteFunc _writeFunc = nullptr;
		bool _isInitialized = false;
#pragma warning(pop)

	public:
		SMiniz(WriteFunc writeFunc);
		virtual ~SMiniz() = default;
		void printStat(int bufLength, size_t index, size_t length, int flush);
	};

	class EXPORTIT SMinizCompress : public SMiniz
	{
	public:
		SMinizCompress(WriteFunc funcWrite) :SMiniz(funcWrite) {}
		~SMinizCompress();
		void initialize(int compressionLevel = Z_BEST_COMPRESSION);
		void end();
		int writeDeflate(int flush);
		void compressData(const unsigned char* srcData, size_t length, int flush = Z_NO_FLUSH);
	};

	class EXPORTIT SMinizDecompress : public SMiniz
	{
	public:
		SMinizDecompress(WriteFunc funcWrite) :SMiniz(funcWrite) {}
		~SMinizDecompress();
		void initialize();
		void end();
		int writeInflate(int flush);
		void decompressData(const unsigned char* srcData, size_t length);
	};
}
