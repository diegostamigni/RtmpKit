package com.diegostamigni.rtmpkit;

public class CodecAlreadyConfiguredException extends IllegalStateException
{
	public CodecAlreadyConfiguredException(String message)
	{
		super(message);
	}
}