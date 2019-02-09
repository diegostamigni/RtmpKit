package com.diegostamigni.rtmpkit.rtmp;

import lombok.Getter;
import lombok.Setter;

public class RtmpSettings
{
	@Getter @Setter
	private int height = 0;

	@Getter @Setter
	private int width = 0;

	@Getter @Setter
	private int frameRate = 0;

	@Getter @Setter
	private int audioChannels = 0;

	@Getter @Setter
	private String videoCodecID = null;

	public RtmpSettings()
	{
	}

	public RtmpSettings(int height, int width, int frameRate, int audioChannels, String videoCodecID)
	{
		setHeight(height);
		setWidth(width);
		setFrameRate(frameRate);
		setAudioChannels(audioChannels);
		setVideoCodecID(videoCodecID);
	}
}