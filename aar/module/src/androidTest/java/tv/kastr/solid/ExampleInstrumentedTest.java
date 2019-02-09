package com.diegostamigni.rtmpkit;

import android.content.Context;
import android.os.Looper;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.CountDownLatch;

import com.diegostamigni.rtmpkit.rtmp.RtmpWatcherView;

import static org.junit.Assert.assertEquals;

/**
 * Instrumentation test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest
{
	private RtmpWatcherView rtmpWatcherView;

	@Test
	public void useAppContext() throws Exception
	{
		// Context of the app under test.
		Context appContext = InstrumentationRegistry.getTargetContext();

		assertEquals("com.diegostamigni.rtmpkit", appContext.getPackageName());
	}

	@Test
	public void rtmpConnectionTest() throws Exception
	{
		final CountDownLatch signal = new CountDownLatch(1);
		Context appContext = InstrumentationRegistry.getTargetContext();

		Looper.prepare();
		this.rtmpWatcherView = new RtmpWatcherView(appContext);

		rtmpWatcherView.setWatcherLiveSessionListener(new WatcherLiveSessionListener()
		{
			@Override
			public void sessionStarted()
			{
//				signal.countDown();
			}

			@Override
			public void sessionEnded()
			{
				signal.countDown();
			}
		});

		final String streamID = "6599de96-795d-4bc4-9356-28895b612b73";
		final String url = streamID + "?RtmpKitChannelId=2a2b442c-2a0e-4728-ad70-3d9f3b0ae7be&RtmpKitClientId=214E9700-BB91-4C04-B818-BF2E5012790A";
		rtmpWatcherView.start("strm-out.dev.ldn.diegostamigni.com", "1935", "live_out_host", url);

		signal.await();
	}
}
