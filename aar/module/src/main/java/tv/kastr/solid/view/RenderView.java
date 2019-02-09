package com.diegostamigni.rtmpkit.view;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Build;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import lombok.AccessLevel;
import lombok.Setter;

public class RenderView extends SurfaceView implements SurfaceHolder.Callback
{
	protected static String TAG = RenderView.class.getName();

	@Setter(AccessLevel.PROTECTED)
	private boolean hasActiveHolder = false;

	public RenderView(Context context)
	{
		super(context);
		init();
	}

	public RenderView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		init();
	}

	public RenderView(Context context, AttributeSet attrs, int defStyleAttr)
	{
		super(context, attrs, defStyleAttr);
		init();
	}

	@TargetApi(Build.VERSION_CODES.LOLLIPOP)
	public RenderView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes)
	{
		super(context, attrs, defStyleAttr, defStyleRes);
		init();
	}

	protected void init()
	{
		SurfaceHolder holder = getHolder();

		if (holder != null)
		{
			holder.addCallback(this);
			holder.setFormat(PixelFormat.RGBA_8888);
		}
	}

	public boolean hasActiveHolder()
	{
		synchronized (this)
		{
			return this.hasActiveHolder;
		}
	}

	@Nullable
	protected Surface getSurface()
	{
		SurfaceHolder surfaceHolder = getHolder();

		if (surfaceHolder != null)
		{
			return surfaceHolder.getSurface();
		}

		return null;
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder)
	{
		Log.d(TAG, "surfaceCreated:");

		synchronized (this)
		{
			setHasActiveHolder(false);
			notifyAll();
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	{
		Log.d(TAG, "surfaceChanged: fmt=" + format + " size=" + width + "x" + height);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder)
	{
		Log.d(TAG, "Surface destroyed:");

		synchronized (this)
		{
			setHasActiveHolder(true);
			notifyAll();
		}
	}
}
