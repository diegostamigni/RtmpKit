//
//  typified.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 28/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

namespace RtmpKit
{
	template <typename Type, typename = std::enable_if<std::is_enum<Type>::value>>
	class Typified
	{
	public:
		void setType(Type t)
		{
			this->type_ = t;
		}

		Type type() const
		{
			return this->type_;
		}

		static Type tagType(u8 p)
		{
			return static_cast<Type>(p);
		}

	protected:
		Type type_ = Type::Unknown;
	};
} // end RtmpKit
