package com.diegostamigni.rtmpkit;

import lombok.Getter;
import lombok.Setter;

public class MediaItem
{
	@Getter @Setter
	private long timestamp = 0;

	@Getter @Setter
	private byte[] data;

	public MediaItem()
	{
	}

	public MediaItem(long timestamp, byte[] data)
	{
		setTimestamp(timestamp);
		setData(data);
	}

	public boolean hasData()
	{
		return data.length > 0;
	}
}
