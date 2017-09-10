/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "MVEPlayer.h"

#include "ie_types.h"

#include "Audio.h"
#include "Interface.h"
#include "Palette.h"
#include "Variables.h"
#include "Video.h"

#include <cassert>
#include <cstdio>

using namespace GemRB;

static const char MVESignature[] = "Interplay MVE File\x1A";
static const int MVE_SIGNATURE_LEN = 19;

static unsigned char g_palette[768];

MVEPlay::MVEPlay(void)
: decoder(this)
{
	video = core->GetVideoDriver();
	validVideo = false;
}

MVEPlay::~MVEPlay(void)
{
}

bool MVEPlay::Open(DataStream* stream)
{
	str = stream;
	validVideo = false;

	char Signature[MVE_SIGNATURE_LEN];
	str->Read( Signature, MVE_SIGNATURE_LEN );
	if (memcmp( Signature, MVESignature, MVE_SIGNATURE_LEN ) != 0) {
		return false;
	}

	str->Seek( 0, GEM_STREAM_START );

	memset( g_palette, 0, 768 );
	
	validVideo = decoder.start_playback();
	return validVideo;
}

bool MVEPlay::DecodeFrame(VideoBuffer& buf)
{
	vidBuf = &buf;
	++framePos;
	return (validVideo && decoder.next_frame());
}

unsigned int MVEPlay::fileRead(void* buf, unsigned int count)
{
	unsigned numread;

	numread = str->Read( buf, count );
	return ( numread == count );
}

void MVEPlay::showFrame(unsigned char* buf, unsigned int bufw, unsigned int bufh)
{
	assert(vidBuf);
	Size s = vidBuf->Size();
	int dest_x = unsigned(s.w - bufw) >> 1;
	int dest_y = unsigned(s.h - bufh) >> 1;
	vidBuf->CopyPixels(Region(dest_x, dest_y, bufw, bufh), buf, NULL, g_palette);
}

void MVEPlay::setPalette(unsigned char* p, unsigned start, unsigned count)
{
	//Set color 0 to be black
	g_palette[0] = g_palette[1] = g_palette[2] = 0;

	//Set color 255 to be our subtitle color
	g_palette[765] = g_palette[766] = g_palette[767] = 50;

	//movie libs palette into our array
	memcpy( g_palette + start * 3, p + start * 3, count * 3 );
}

int MVEPlay::setAudioStream()
{
	ieDword volume ;
	core->GetDictionary()->Lookup( "Volume Movie", volume) ;
	int source = core->GetAudioDrv()->SetupNewStream(0, 0, 0, volume, false, false) ;
	return source;
}

void MVEPlay::freeAudioStream(int stream)
{
	if (stream > -1)
		core->GetAudioDrv()->ReleaseStream(stream, true);
}

void MVEPlay::queueBuffer(int stream, unsigned short bits,
			int channels, short* memory,
			int size, int samplerate)
{
	if (stream > -1)
		core->GetAudioDrv()->QueueBuffer(stream, bits, channels, memory, size, samplerate) ;
}


#include "plugindef.h"

GEMRB_PLUGIN(0x218963DC, "MVE Video Player")
PLUGIN_IE_RESOURCE(MVEPlay, "mve", (ieWord)IE_MVE_CLASS_ID)
END_PLUGIN()
