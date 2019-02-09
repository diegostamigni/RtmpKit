//
//  network_speed_indicator.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 16/02/2016.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include <chrono>

namespace RtmpKit
{
    class NetworkSpeedIndicator
    {
    public:
        NetworkSpeedIndicator();
        NetworkSpeedIndicator(const NetworkSpeedIndicator&) = default;
        NetworkSpeedIndicator(NetworkSpeedIndicator&&) = default;

        NetworkSpeedIndicator& operator=(const NetworkSpeedIndicator&) = default;
        NetworkSpeedIndicator& operator=(NetworkSpeedIndicator&&) = default;

    public:
        void setTransferredBytes(std::size_t value);

        std::size_t transferredbytes() const
        { 
            return currentBytes_;
        }
		
		std::size_t currentBitPerSeconds() const;
		
		std::size_t currentBytesPerSeconds() const;
		
		std::size_t currentKilobytesPerSeconds() const;
		
		void reset();

    private:
        std::chrono::system_clock::time_point start_;
        std::chrono::system_clock::time_point current_;
        std::size_t currentBytes_;
    };
} // end RtmpKit
