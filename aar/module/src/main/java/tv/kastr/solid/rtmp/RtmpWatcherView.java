package com.diegostamigni.rtmpkit.rtmp;

import android.annotation.TargetApi;
import android.content.Context;
import android.media.MediaCodec;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;

import com.google.common.primitives.Bytes;

import java.io.IOException;
import java.nio.ByteBuffer;

import lombok.AccessLevel;
import lombok.Getter;
import lombok.Setter;

import com.diegostamigni.rtmpkit.CodecAlreadyConfiguredException;
import com.diegostamigni.rtmpkit.Constants;
import com.diegostamigni.rtmpkit.VideoDecoderThread;
import com.diegostamigni.rtmpkit.WatcherLiveSessionListener;
import com.diegostamigni.rtmpkit.view.RenderView;

interface RtmpWatcherDelegate
{
	void metadataPacketReceived(RtmpSettings settings);

	void audioPacketReceived(long timestamp, byte[] data);

	void videoPacketReceived(long timestamp, byte[] data);

	void videoFormatPacketReceived(long timestamp, int nalUnitSize, byte[] sps, byte[] pps);
}

public class RtmpWatcherView extends RenderView implements RtmpWatcherDelegate
{
	@Setter
	private WatcherLiveSessionListener watcherLiveSessionListener = null;

//	@Setter(AccessLevel.PROTECTED) @Getter(AccessLevel.PROTECTED)
//	private ConcurrentLinkedQueue<MediaItem> videoData = new ConcurrentLinkedQueue<>();

//	@Setter(AccessLevel.PROTECTED) @Getter(AccessLevel.PROTECTED)
//	private ConcurrentLinkedQueue<MediaItem> audioData = new ConcurrentLinkedQueue<>();

	@Setter(AccessLevel.PROTECTED) @Getter
	private RtmpSettings rtmpSettings;

	@Setter(AccessLevel.PROTECTED) @Getter
	private VideoDecoderThread videoDecoderThread;

	public RtmpWatcherView(Context context)
	{
		super(context);
	}

	public RtmpWatcherView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
	}

	public RtmpWatcherView(Context context, AttributeSet attrs, int defStyleAttr)
	{
		super(context, attrs, defStyleAttr);
	}

	@TargetApi(Build.VERSION_CODES.LOLLIPOP)
	public RtmpWatcherView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes)
	{
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	protected void init()
	{
		super.init();

		videoDecoderThread = new VideoDecoderThread();
		videoDecoderThread.start();
	}

	public native void start(String host, String port, String appName, String streamURL);

	public native void end();

	public native boolean isConnected();

	@Override
	public void metadataPacketReceived(RtmpSettings settings)
	{
		Log.d(TAG, "metadataPacketReceived:");

		this.rtmpSettings = settings;
	}

	@Override
	public void audioPacketReceived(long timestamp, byte[] data)
	{
//		audioData.add(new MediaItem(timestamp, data));
	}

	@Override
	public void videoFormatPacketReceived(long timestamp, int nalUnitSize, byte[] sps, byte[] pps)
	{
		try
		{
			Surface surface = getSurface();

			if (surface != null && this.rtmpSettings != null)
			{
				videoDecoderThread.configure(surface,
											 this.rtmpSettings.getWidth(),
											 this.rtmpSettings.getHeight(),
											 ByteBuffer.wrap(sps),
											 ByteBuffer.wrap(pps));
			}
		}
		catch (CodecAlreadyConfiguredException e)
		{
			videoDecoderThread.decodeSample(sps,
											0,
											sps.length,
											timestamp,
											MediaCodec.BUFFER_FLAG_CODEC_CONFIG);

			videoDecoderThread.decodeSample(pps,
											0,
											pps.length,
											timestamp,
											MediaCodec.BUFFER_FLAG_CODEC_CONFIG);
		}
		catch (RuntimeException | IOException e)
		{
			Log.e(TAG, e.toString());
		}
	}

	@Override
	public void videoPacketReceived(long timestamp, byte[] data)
	{
		int flags = 0;

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
		{
			flags = MediaCodec.BUFFER_FLAG_SYNC_FRAME;
		}
		else
		{
			flags = MediaCodec.BUFFER_FLAG_KEY_FRAME;
		}

		byte videoData[] = Bytes.concat(Constants.PICTURE_DATA_HEADER_WITH_NALU_TYPE, data);
		videoDecoderThread.decodeSample(videoData, 0, videoData.length, timestamp, flags);
	}

	static
	{
		System.loadLibrary("rtmpkit");
	}
}
