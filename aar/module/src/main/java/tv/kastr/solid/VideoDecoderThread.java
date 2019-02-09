package com.diegostamigni.rtmpkit;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;

import lombok.AccessLevel;
import lombok.Getter;
import lombok.Setter;

public class VideoDecoderThread extends Thread
{
	protected static String TAG = VideoDecoderThread.class.getName();
	private static final int waitingTime = 10000;

	@Setter(AccessLevel.PROTECTED) @Getter
	private boolean isConfigured = false;

	@Setter(AccessLevel.PROTECTED) @Getter
	private MediaCodec mediaCodec;

	private boolean first = false;
	private long startWhen = 0;

	public void configure(Surface surface, int width, int height, ByteBuffer sps, ByteBuffer pps)
			throws IOException, CodecAlreadyConfiguredException
	{
		if (isConfigured())
		{
			throw new CodecAlreadyConfiguredException("Decoder is already configured");
		}

		MediaFormat format = MediaFormat.createVideoFormat(Constants.KEY_MIME, width, height);

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
		{
			format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
		}
		else
		{
			format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
		}

		format.setByteBuffer("csd-0", sps);
		format.setByteBuffer("csd-1", pps);

		mediaCodec = MediaCodec.createDecoderByType(Constants.KEY_MIME);
		mediaCodec.configure(format, surface, null, 0);
		mediaCodec.start();

		setConfigured(true);
	}

	public void decodeSample(byte[] data, int offset, int size, long presentationTimeUs, int flags)
	{
		if (isConfigured() && !Thread.interrupted())
		{
			int index = mediaCodec.dequeueInputBuffer(waitingTime);

			if (index >= 0)
			{
				ByteBuffer buffer;

				// since API 21 we have new API to use
				if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
				{
					buffer = mediaCodec.getInputBuffers()[index];
					buffer.clear();
				}
				else
				{
					buffer = mediaCodec.getInputBuffer(index);
				}

				if (buffer != null)
				{
					buffer.put(data, offset, size);
					mediaCodec.queueInputBuffer(index, 0, size, presentationTimeUs, flags);
				}
			}
		}
	}

	@Override
	public void run()
	{
		try
		{
			MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

			while (!Thread.interrupted())
			{
				if (isConfigured())
				{
					int index = mediaCodec.dequeueOutputBuffer(info, waitingTime);

					if (index >= 0)
					{
						if (!first)
						{
							startWhen = System.currentTimeMillis();
							first = true;
						}

						long sleepTime = info.presentationTimeUs - (System.currentTimeMillis() - startWhen);
						Log.d(TAG, "info.presentationTimeUs: " + info.presentationTimeUs
								+ " playTime: " + (System.currentTimeMillis() - startWhen)
								+ " sleepTime : " + sleepTime);

						if (sleepTime > 0)
						{
							Thread.sleep(sleepTime);
						}

						// Setting true is telling system to render frame onto Surface
						mediaCodec.releaseOutputBuffer(index, true);

						if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) == MediaCodec.BUFFER_FLAG_END_OF_STREAM)
						{
							break;
						}
					}
				}
				else
				{
					try
					{
						// Waiting to be configured, then decode and render
						Thread.sleep(100);
					}
					catch (InterruptedException ignore)
					{
					}
				}
			}
		}
		catch (InterruptedException e)
		{
			Log.e(TAG, e.toString());
		}
		finally
		{
			if (isConfigured())
			{
				mediaCodec.stop();
				mediaCodec.release();
			}
		}
	}
}