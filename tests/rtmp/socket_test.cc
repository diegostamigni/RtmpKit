//
//  socket_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 17/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"
#include "../../private/semaphore.h"
#include "../../private/socket/base_socket.h"

static RtmpKit::SimpleSemaphore sem;

struct DelegateCollector : RtmpKit::BaseSocketDelegate
{
	virtual void socketWillConnect() override
	{
	}

	virtual void socketDidConnect() override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}

	virtual void socketWillClose() override
	{
	}

	virtual void socketDidClose() override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}

	virtual void socketFailedToConnect(const std::string& errMessage) override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}
	
	void reconnectionRequired() override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}
};

TEST(Socket, Connect)
{
	RtmpKit::BaseSocket sock;
	auto delegate = std::make_shared<DelegateCollector>();
	sock.addDelegate(delegate);
	sock.connect("stream-stg.diegostamigni.com", "1935");
	sem.wait();
}

TEST(Socket, WrongConnect)
{
	RtmpKit::BaseSocket sock;
	auto delegate = std::make_shared<DelegateCollector>();
	sock.addDelegate(delegate);
	sock.connect("an-host-that-does-not-exist", "1935");
	sem.wait();
}
