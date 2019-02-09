package com.diegostamigni.rtmpkit;

import com.google.common.primitives.Bytes;

public final class Constants
{
    public static final String KEY_MIME = "video/avc";

	public static byte NALU_HEADER[] = new byte[] {
			0x00,
			0x00,
			0x00,
			0x01,
	};

    public static byte SPS_HEADER[] = Constants.NALU_HEADER;

	public static byte PPS_HEADER[] = Constants.NALU_HEADER;

    public static byte PICTURE_DATA_HEADER[] = Constants.NALU_HEADER;

    public static byte SPS_HEADER_WITH_NALU_TYPE[] = Bytes.concat(Constants.SPS_HEADER, new byte[] { 0x27 });

    public static byte PPS_HEADER_WITH_NALU_TYPE[] = Bytes.concat(Constants.PPS_HEADER, new byte[] { 0x28 });

    public static byte PICTURE_DATA_HEADER_WITH_NALU_TYPE[] = Bytes.concat(Constants.PICTURE_DATA_HEADER, new byte[] { 0x25 });
}
