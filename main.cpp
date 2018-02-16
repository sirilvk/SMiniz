#include <iostream>
#include <memory>
#include <unistd.h>
#include "SMiniz.h"
#include <vector>
#include <unordered_set>
#include <csignal>

#define BUF_SIZE 10000
volatile bool should_exit = false;

void sighandler(int)
{
    std::cout << "ending process" << std::endl;
    should_exit = true;
}

class CompDecompFile
{
private:
    FILE* _fileHandle = nullptr;
    std::unique_ptr<util::SMinizDecompress> _decompressor;
    std::unique_ptr<util::SMinizCompress> _compressor;

public:
    CompDecompFile(const std::string& filepath)
	{
	    _fileHandle = fopen(filepath.c_str(), "rb");
	}

    void deCompress(std::function<bool(const unsigned char* data, size_t length)> callback)
	{
	    char buf[BUF_SIZE];
	    _decompressor = std::make_unique<util::SMinizDecompress>(callback);
	    _decompressor->initialize();
	    while(!should_exit)
	    {
		size_t readLen = fread(buf, 1, BUF_SIZE, _fileHandle);
		if (0 == readLen)
		    break;
		_decompressor->decompressData((unsigned char*)buf, readLen);
	    }
	    _decompressor.reset();
	}
    
    void Compress(std::function<bool(const unsigned char* data, size_t length)> callback)
	{
	    char buf[BUF_SIZE];
	    _compressor = std::make_unique<util::SMinizCompress>(callback);
	    _compressor->initialize();
	    while(!should_exit)
	    {
		size_t readLen = fread(buf, 1, BUF_SIZE, _fileHandle);
		if (0 == readLen)
		    break;
		_compressor->compressData((unsigned char*)buf, readLen);
	    }
	    _compressor.reset();
	}
};

void processfunc(unsigned char* buf)
{
}

int main(int argc, char** argv)
{
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
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
	    decompress = true;
	    break;
	case 'c':
	    decompress = false;
	    break;
	}
    }

    if (!filepath.empty())
    {
	CompDecompFile df(filepath);
	std::vector<char> dataRead;
	if (decompress)
	{
	    df.deCompress([&](const unsigned char* data, size_t length)->bool {
		    // handle the uncompressed data here
		    return true;
		});
	}
	else
	{
	    df.Compress([&](const unsigned char* data, size_t length)->bool {
		    // handle the compressed data here
		    return true;
		});
	}
    }
}
    
    
