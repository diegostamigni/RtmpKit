//
//  network_types.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 6/02/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include <string>

namespace RtmpKit
{
	enum class NetworkType : std::size_t
	{
		_0 = 0, // unknown
		
		VeryLow,
		Low,
		Medium,
		High,
		Highest,
		
		NumberOfItems = 5 // excluding _0 and this
	};
		
    inline std::ostream& operator<<(std::ostream &os, const NetworkType& p)
    {
        std::string result;
        
        switch (p)
		{
			case NetworkType::Highest:
			{
				result = "Highest";
				break;
			}
            case NetworkType::High:
            {
                result = "High";
                break;
            }
            case NetworkType::Medium:
            {
                result = "Medium";
                break;
            }
            case NetworkType::Low:
            {
                result = "Low";
                break;
            }
            case NetworkType::VeryLow:
            {
                result = "VeryLow";
                break;
            }
            default:
            {
                result = "Unknown";
                break;
            }
        }
        
        return os << result;
    }
		
	inline NetworkType& operator++(NetworkType& p)
	{
		if (p != NetworkType::Highest)
		{
			auto uValue = std::underlying_type_t<NetworkType>(p);
			p = static_cast<NetworkType>(++uValue);
		}
		
		return p;
	}
		
	inline NetworkType& operator--(NetworkType& p)
	{
		if (p != NetworkType::VeryLow)
		{
			auto uValue = std::underlying_type_t<NetworkType>(p);
			p = static_cast<NetworkType>(--uValue);
		}
			
		return p;
	}
} // end RtmpKit
