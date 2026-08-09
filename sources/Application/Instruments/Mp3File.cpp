
#include "Mp3File.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include <string.h>

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_EX_IMPLEMENTATION
#include "Externals/minimp3/minimp3_ex.h"

Mp3FileError Mp3File::lastError_ = MP3ERR_NONE ;

Mp3File::Mp3File() {
	samples_=0 ;
	size_=0 ;
	sampleRate_=0 ;
	channelCount_=0 ;
	readPosition_=0 ;
} ;

Mp3File::~Mp3File() {
	SAFE_FREE(samples_) ;
} ;

Mp3File *Mp3File::Open(const char *path) {

	// Read the whole file through the FileSystem so the decode path does
	// not depend on stdio (not available on every platform).

	I_File *file=FileSystem::GetInstance()->Open(path,"r") ;
	if (!file) {
		Trace::Error("Mp3File::Open(): failed to open %s",path) ;
		lastError_=MP3ERR_OPEN_FAILED ;
		return 0 ;
	} ;

	file->Seek(0,SEEK_END) ;
	long fileSize=file->Tell() ;
	file->Seek(0,SEEK_SET) ;

	if (fileSize<=0) {
		Trace::Error("Mp3File::Open(): empty file %s",path) ;
		file->Close() ;
		delete file ;
		lastError_=MP3ERR_READ_FAILED ;
		return 0 ;
	} ;

	char *buf=(char *)SYS_MALLOC(fileSize) ;
	if (!buf) {
		file->Close() ;
		delete file ;
		lastError_=MP3ERR_OUT_OF_MEMORY ;
		return 0 ;
	} ;

	file->Read(buf,1,fileSize) ;
	file->Close() ;
	delete file ;

	// Decode the whole stream to PCM16. MP3D_SEEK_TO_SAMPLE scans the
	// stream so dec.samples (total, channels included) is known up front.

	mp3dec_ex_t dec ;
	int err=mp3dec_ex_open_buf(&dec,(const uint8_t *)buf,fileSize,MP3D_SEEK_TO_SAMPLE) ;
	if (err) {
		Trace::Error("Mp3File::Open(): decode failed for %s (err=%d)",path,err) ;
		SAFE_FREE(buf) ;
		lastError_=MP3ERR_DECODE_FAILED ;
		return 0 ;
	} ;

	Mp3File *mp3=new Mp3File() ;
	mp3->sampleRate_=dec.info.hz ;
	mp3->channelCount_=dec.info.channels ;
	mp3->size_=dec.samples/dec.info.channels ; // samples per channel

	if ((mp3->size_<=0)||(dec.info.channels>2)) {
		Trace::Error("Mp3File::Open(): no decodable frames in %s",path) ;
		mp3dec_ex_close(&dec) ;
		SAFE_FREE(buf) ;
		delete mp3 ;
		lastError_=MP3ERR_DECODE_FAILED ;
		return 0 ;
	} ;

	mp3->samples_=(short *)SYS_MALLOC(dec.samples*sizeof(short)) ;
	if (!mp3->samples_) {
		mp3dec_ex_close(&dec) ;
		SAFE_FREE(buf) ;
		delete mp3 ;
		lastError_=MP3ERR_OUT_OF_MEMORY ;
		return 0 ;
	} ;

	// Read the whole decoded stream (interleaved, all channels)

	size_t total=0 ;
	while (total<dec.samples) {
		size_t n=mp3dec_ex_read(&dec,mp3->samples_+total,dec.samples-total) ;
		if (n<=0) break ;
		total+=n ;
	} ;

	mp3dec_ex_close(&dec) ;
	SAFE_FREE(buf) ;

	if (total<dec.samples) {
		Trace::Error("Mp3File::Open(): short read for %s (%d/%d)",path,(int)total,(int)dec.samples) ;
		delete mp3 ;
		lastError_=MP3ERR_DECODE_FAILED ;
		return 0 ;
	} ;

	Trace::Log("Mp3File","%s: %d Hz, %d ch, %d samples",path,mp3->sampleRate_,mp3->channelCount_,mp3->size_) ;
	lastError_=MP3ERR_NONE ;
	return mp3 ;
} ;

void *Mp3File::GetSampleBuffer(int note) {
	return samples_+readPosition_*channelCount_ ;
} ;

int Mp3File::GetSize(int note) {
	return size_ ;
} ;

int Mp3File::GetChannelCount(int note) {
	return channelCount_ ;
} ;

int Mp3File::GetSampleRate(int note) {
	return sampleRate_ ;
} ;

bool Mp3File::GetBuffer(long start,long sampleCount) {
	// whole stream is decoded up front; position the read offset so
	// GetSampleBuffer() mirrors WavFile's disk-backed behaviour
	readPosition_=start ;
	return true ;
} ;

void Mp3File::Close() {
	// nothing to release: the decoded buffer lives until destruction
} ;

int Mp3File::GetRootNote(int note) {
	return 60 ;
} ;

Mp3FileError Mp3File::GetLastError() {
	return lastError_ ;
} ;
