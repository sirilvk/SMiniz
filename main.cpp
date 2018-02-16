#include <iostream>
#include <memory>
#include <unistd.h>
#include "SMiniz.h"
#include <vector>
#include <unordered_set>
class CompDecompFile
{
private:
    FILE* _fileHandle = nullptr;
    std::unique_ptr<util::SMinizDecompress> _decompressor;
    std::unique_ptr<util::SMinizCompress> _compressor;
    bool _doCompress;

public:
    CompDecompFile(const std::string& filepath, bool decompress = true):_doCompress(decompress)
	{
	    _fileHandle = fopen(filepath.c_str(), "rb");
	}

    void doWork(std::function<bool(const unsigned char* data, size_t length)> callback)
	{
	    if (_doCompress)
	    {
		_decompressor = std::make_unique<util::SMinizDecompress>(callback);
	    }
	    else
	    {
		_compressor = std::make_unique<util::SMinizCompress>(callback);
	    }
	}
};

void processfunc(unsigned char* buf)
{

}

int main(int argc, char** argv)
{
    bool decompress;
    std::string filepath;
    int c;
    while((c = getopt(argc, argv, "cdf:")) != -1)
    {
	switch(c)
	{
	case 'f':
	    if (optarg)
		filepath = optarg;
	    break;
	case 'd':
	    decompress = false;
	    break;
	case 'c':
	    decompress = true;
	    break;
	}
    }

    if (!filepath.empty())
    {
	CompDecompFile df(filepath, decompress);
	std::vector<char> dataRead;

	// with compressed message containing its size in the first int field
	df.doWork([&](const unsigned char* data, size_t length)->bool {
		// do all your work here
		dataRead.insert(dataRead.end(), data, data + length);
		int msgLen = 0;
		const int intLen = sizeof(msgLen);
		while (dataRead.size() > intLen)
		{
			unsigned char* ptr = (unsigned char*)dataRead.data();
			memcpy(&msgLen, ptr, intLen);

			if (msgLen > (dataRead.size() - intLen))
			{
				// not enough data to process ..
				break;
			}
			else
			{
				unsigned char* msg = ptr + intLen;
				processfunc(msg);
				dataRead.erase(dataRead.begin(), dataRead.begin() + msgLen + intLen);
			}
		}

		return true;
	    });

    }
}
    
    
