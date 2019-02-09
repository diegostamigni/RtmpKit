package com.diegostamigni.rtmpkit.view;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;

import com.diegostamigni.rtmpkit.rtmp.RtmpWatcherView;

public class LiveWatcherView extends RtmpWatcherView
{
	public LiveWatcherView(Context context)
	{
		super(context);
	}

	public LiveWatcherView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
	}

	public LiveWatcherView(Context context, AttributeSet attrs, int defStyleAttr)
	{
		super(context, attrs, defStyleAttr);
	}

	@TargetApi(Build.VERSION_CODES.LOLLIPOP)
	public LiveWatcherView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes)
	{
		super(context, attrs, defStyleAttr, defStyleRes);
	}
}
