#include "SMiniz.h"
#include <functional>

namespace util
{
	// base wrapper starts here
	SMiniz::SMiniz(WriteFunc writeFunc) :_writeFunc(writeFunc)
	{
		memset(&_stream, 0, sizeof(_stream));
		_stream.avail_out = SMINIZ_BUF_SIZE;
		_stream.next_out = _outBuf;
		_stream.avail_in = 0;
	}

	void SMiniz::printStat(int bufLength, size_t index, size_t length, int flush)
	{
		std::cout << ((flush == Z_FINISH) ? "Finish:" : "Buffered:") << bufLength << ":" << index << ":" << length << ":" << _stream.avail_in << ":" << _stream.next_in << std::endl;
	}


	// Compression wrapper starts here
	SMinizCompress::~SMinizCompress()
	{
		if (!_isInitialized)
			return;

		// ending the compression
		try
		{
			if (_writeFunc)
				compressData(reinterpret_cast<const unsigned char*>(""), 0, Z_FINISH);
			end();
		}
		catch (std::exception& ex)
		{
			// can't throw in destructor .. so just log
			std::string errMsg = "Failed to finalize the deflate because of ";
			errMsg += ex.what();
			std::cerr << errMsg << std::endl;
		}
	}

	void SMinizCompress::initialize(int compressionLevel)
	{
		_isInitialized = true;
		if (deflateInit(&_stream, compressionLevel) != Z_OK)
		{
			throw std::runtime_error("deflateInit() failed!");
		}
	}

	void SMinizCompress::end()
	{
		if (deflateEnd(&_stream) != Z_OK)
		{
			throw std::runtime_error("deflateEnd() failed!");
		}
	}

	int SMinizCompress::writeDeflate(int flush)
	{
		int status = deflate(&_stream, flush);
		if ((status == Z_STREAM_END) || (!_stream.avail_out))
		{
			size_t writenData = SMINIZ_BUF_SIZE - _stream.avail_out;
			_writeFunc(_outBuf, writenData);
			_stream.next_out = _outBuf;
			_stream.avail_out = SMINIZ_BUF_SIZE;
		}
		else if (status != Z_OK)
		{
			throw std::runtime_error("Deflate() failed with status [" + std::to_string(status) + "]");
		}
		return status;
	}

	void SMinizCompress::compressData(const unsigned char* srcData, size_t length, int flush/* = Z_NO_FLUSH*/)
	{
		if (flush == Z_FINISH)
		{
			_stream.next_in = reinterpret_cast<const unsigned char*>("");
			_stream.avail_in = 0;
			int status = Z_OK;
			do
			{
				// flush everything out
				status = writeDeflate(flush);
			} while (status != Z_STREAM_END);
		}
		else
		{
			size_t index = 0;
			while (_stream.avail_in || (index < length))
			{
				if (!_stream.avail_in)
				{
					// now read data
					if ((length - index) > SMINIZ_BUF_SIZE)
					{
						_stream.next_in = srcData + index;
						index += SMINIZ_BUF_SIZE;
						_stream.avail_in = SMINIZ_BUF_SIZE;
					}
					else
					{
						_stream.next_in = srcData + index;
						_stream.avail_in = (length - index);
						index += (length - index);
					}
				}

				writeDeflate(flush);
			}
		}
	}


	// Decompression wrapper starts here
	SMinizDecompress::~SMinizDecompress()
	{
		if (!_isInitialized)
			return;

		// ending decompression.
		try
		{
			end();
		}
		catch (std::exception& ex)
		{
			// can't throw in destructor .. so just log
			std::string errMsg = "Failed to finalize the deflate because of ";
			errMsg += ex.what();
			std::cerr << errMsg << std::endl;
		}
	}

	void SMinizDecompress::initialize()
	{
		_isInitialized = true;
		if (inflateInit(&_stream))
		{
			throw std::runtime_error("inflateInit() failed!");
		}
	}
	
	void SMinizDecompress::end()
	{
		if (inflateEnd(&_stream) != Z_OK)
		{
			throw std::runtime_error("inflateEnd() failed!\n");
		}
	}
	
	int SMinizDecompress::writeInflate(int flush)
	{
		int status = inflate(&_stream, flush);
		if ((status == Z_STREAM_END) || (!_stream.avail_out))
		{
			size_t writenData = SMINIZ_BUF_SIZE - _stream.avail_out;
			_writeFunc(_outBuf, writenData);
			_stream.next_out = _outBuf;
			_stream.avail_out = SMINIZ_BUF_SIZE;
		}
		else if (status != Z_OK)
		{
			throw std::runtime_error("Inflate() failed with status [" + std::to_string(status) + "]");
		}
		return status;
	}

	void SMinizDecompress::decompressData(const unsigned char* srcData, size_t length)
	{
		size_t index = 0;
		int status = Z_OK;
		while ((status != Z_STREAM_END) || (_stream.avail_in || (index < length)))
		{
			if (!_stream.avail_in)
			{
				if (index < length)
				{
					// now read data if available
					if ((length - index) > SMINIZ_BUF_SIZE)
					{
						_stream.next_in = srcData + index;
						index += SMINIZ_BUF_SIZE;
						_stream.avail_in = SMINIZ_BUF_SIZE;
					}
					else
					{
						_stream.next_in = srcData + index;
						_stream.avail_in = (length - index);
						index += (length - index);
					}
				}
				else
				{
					// probably need more data so break off
					break;
				}
			}

			status = writeInflate(Z_SYNC_FLUSH);
		}
	}
}
