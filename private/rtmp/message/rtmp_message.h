//
//  rtmp_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 28/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/amf/types.h"
#include "../../../private/typed_tag.h"

#define NETCONNECTION_CONNECT_SUCCESS "NetConnection.Connect.Success"

/*!
 * The Server application name the client is connected to
 */
#define PROPERTY_APP "app"

/*!
 * Flash Player version. It is the same string as returned by the ApplicationScript getversion() function
 */
#define PROPERTY_FLASH_VER "flashver"

/*!
 * URL of the source SWF file making the connection
 */
#define PROPERTY_SWF_URL "swfUrl"

/*!
 * URL of the Server
 */
#define PROPERTY_TC_URL "tcUrl"

/*!
 * True if proxy is being used
 */
#define PROPERTY_FPAD "fpad"

/*!
 * Indicates what audio codecs the client supports
 */
#define PROPERTY_AUDIO_CODECS "audioCodecs"

/*!
 * Indicates what video codecs are supported
 */
#define PROPERTY_VIDEO_CODECS "videoCodecs"

/*!
 * Indicates what special video functions are supported
 */
#define PROPERTY_VIDEO_FUNCTION "videoFunction"

/*!
 * URL of the web page from where the SFW file was loaded
 */
#define PROPERTY_PAGE_URL "pageUrl"

/*!
 * AMF encoding method
 */
#define PROPERTY_OBJECT_ENCODING "objectEncoding"

/*!
 * Live data is published without recording it in a file
 */
#define PUBLISH_LIVE "live"

/*!
 * The stream is published and the data is recorded to a new file. The file is stored 
 * on the server in a subdirectory within the directory that contains the server application. 
 * If the file already exist, it is overwritten
 */
#define PUBLISH_RECORD "record"

/*!
 * The stream is published and the data is appended to a file. If no file is found, it is created
 */
#define PUBLISH_APPEND "append"

namespace RtmpKit
{
	enum class RtmpChunkStreamType : u8
	{
		Type0 = 0x00, // 11 bytes
		Type1 = 0x01, // 8 bytes
		Type2 = 0x02, // 3 bytes -- basic header (1 bytes), timestamp (3 bytes)
		Type3 = 0x03, // 0 bytes -- basic header (1 bytes), no header
	};

	enum class RtmpChunkStreamFmt : u8
	{
		Unknown = 0xFF,
		ControlMessage = 0x02,
		AMF_Object = 0x03,
		AudioMessage = 0x04,
		VideoMessage = 0x05,
	};

	enum class RtmpMessageType : u8
	{
		Unknown = 0x00,

		// Protocol Control Messages [[1,3], 5, 6]
		SetChunkSizeMessage = 0x01,
		AbortMessage = 0x02,
		AcknowledgementMessage = 0x03,
		WindowAcknowledgementSizeMessage = 0x05,
		SetPeerBandwidth = 0x06,

		// User Control Messages
		UserControlMessage = 0x04,

		// RTMP Command Messages
		CommandMessage_AMF0 = 0x14,
		CommandMessage_AMF3 = 0x11,
		DataMessage_AMF0 = 0x12,
		DataMessage_AMF3 = 0x0F,
		SharedObjectMessage_AMF0 = 0x13,
		SharedObjectMessage_AMF3 = 0x10,
		AudioMessage = 0x08,
		VideoMessage = 0x09,
		AggregateMessage = 0x16,
	};

	enum class RtmpAudioCodecType : u32
	{
		/*!
		 * Raw sound, no compression
		 */
			SUPPORT_SND_NONE = 0x0001,

		/*!
		 * ADPCM compression
		 */
			SUPPORT_SND_ADPCM = 0x0002,

		/*!
		 * mp3 compression
		 */
			SUPPORT_SND_MP3 = 0x0004,

		/*!
		 * Not used
		 */
			SUPPORT_SND_INTEL = 0x0008,

		/*!
		 * Not used
		 */
			SUPPORT_SND_UNUSED = 0x0010,

		/*!
		 * NellyMoser at 8-kHz compression
		 */
			SUPPORT_SND_NELLY8 = 0x0020,

		/*!
		 * NellyMoser compression (5, 11, 22, and 44 kHz)
		 */
			SUPPORT_SND_NELLY = 0x0040,

		/*!
		 * G711A sound compressio (Flash Media Server only)
		 */
			SUPPORT_SND_G711A = 0x0080,

		/*!
		 * G711U sound compression (Flash Media Server only)
		 */
			SUPPORT_SND_G711U = 0x0100,

		/*!
		 * NellyMouser at 16-kHz compression
		 */
			SUPPORT_SND_NELLY16 = 0x0200,

		/*!
		 * Advanced audio coding (AAC) codec
		 */
			SUPPORT_SND_AAC = 0x0400,

		/*!
		 * Speex Audi
		 */
			SUPPORT_SND_SPEEX = 0x0800,

		/*!
		 * All RTMP-supported audio codecs
		 */
			SUPPORT_SND_ALL = 0x0FFF,
	};

	enum class RtmpVideoCodecType : u32
	{
		/*!
		 * Obsolete value
		 */
			SUPPORT_VID_UNUSED = 0x0001,
		/*!
		 * Obsolete value
		 */
			SUPPORT_VID_JPEG = 0x0002,

		/*!
		 * Sorenson Flash video
		 */
			SUPPORT_VID_SORENSON = 0x0004,

		/*!
		 * V1 screen sharing
		 */
			SUPPORT_VID_HOMEBREW = 0x0008,

		/*!
		 * On2 video (Flash 8+)
		 */
			SUPPORT_VID_VP6 = 0x0010,

		/*!
		 * On2 video with alpha channel
		 */
			SUPPORT_VID_VP6ALPHA = 0x0020,

		/*!
		 * Screen sharing version (Flash 8+)
		 */
			SUPPORT_VID_HOMEBREWV = 0x0040,

		/*!
		 * H264 video
		 */
			SUPPORT_VID_H264 = 0x0080,

		/*!
		 * All RTMP-supported video codecs
		 */
			SUPPORT_VID_ALL = 0x00FF,
	};

	enum class RtmpVideoFunctionType : u32
	{
		/*!
		 * Indicates that the client can perform frame-accurate seeks
		 */
			SUPPORT_VID_CLIENT_SEEK = 1,
	};

	enum class RtmpObjectEncodingType : u32
	{
		/*!
		 * AMF0 object encoding supported by Flash 6 and later
		 */
			AMF0 = 0,

		/*!
		 * AMF3 encoding from Flash 9 (AS3)
		 */
			AMF3 = 3,
	};

	enum class RtmpPeerBandWidthLimitType : u8
	{
		/*!
		 * The peer SHOULD limit its output bandwidth to the indicated window size
		 */
			Hard = 0x00,

		/*!
		 * The peer SHOULD limit its output bandwidth to the the window indicated in
		 * this message or the limit already in effect, whichever is smaller
		 */
			Soft = 0x01,

		/*!
		 * If the previous Limit Type was Hard, treat this message as though it was marked Hard,
		 * otherwise ignore this message
		 */
			Dynamic = 0x02,
	};

	class RtmpMessage : public MediaTypedPacketTag<RtmpMessageType>
	{
	public:
		virtual const v8& serialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual std::size_t size() const override;
		
		void setChunkStreamFmt(u8 chunkStreamId)
		{
			chunkStreamId_ = chunkStreamId;
		}

		u8 chunkStreamFmt() const
		{
			return chunkStreamId_;
		}
		
		u8 chunkStreamID(RtmpChunkStreamType type) const;
		
		virtual std::vector<v8> trunkToSize(u32 size);

		const AMF::TDictSubset& commandObjectProperties() const
		{
			return commandObject_;
		}

		template <typename K, typename V,
			typename = std::enable_if<std::is_convertible<K, std::string>::value
				&& std::is_convertible<V, AMF::TVariantSubset>::value>>
		void addCommandObjectProperty(K&& key, V&& value)
		{
			addProperty(std::forward<K>(key), std::forward<V>(value), commandObject_);
		}

		RtmpChunkStreamType chunkStreamType() const
		{
			return chunkStreamType_;
		}

		void setChunkStreamType(RtmpChunkStreamType value)
		{
			chunkStreamType_ = value;
		}
		
		virtual u32 delay() const;
		
		virtual std::size_t hSize() const;

	protected:
		virtual void deserialize() override;

		template <typename K, typename V,
			typename = std::enable_if<std::is_convertible<K, std::string>::value
				&& std::is_convertible<V, AMF::TVariantSubset>::value>>
		void addProperty(K&& key, V&& value, AMF::TDictSubset& container)
		{
			container.insert({std::forward<K>(key), std::forward<V>(value)});
		}

	protected:
		RtmpChunkStreamType chunkStreamType_ = RtmpChunkStreamType::Type0;
		AMF::TDictSubset commandObject_;
		
	private:
		u8 chunkStreamId_ = static_cast<u8>(RtmpChunkStreamFmt::AMF_Object);
	};
} // end RtmpKit
