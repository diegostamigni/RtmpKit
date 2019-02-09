//
//  types.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 28/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/utils.h"

#include <unordered_map>
#include <vector>

namespace RtmpKit
{
	// TODO(diegostamigni): implement the missing Variants
	namespace AMF
	{
		template <typename T>
		class TypeVariant
		{
		public:
			explicit TypeVariant(T value) : value_(value)
			{
			}

			TypeVariant(TypeVariant&&) = default;
			TypeVariant& operator=(TypeVariant&&) = default;

			TypeVariant(const TypeVariant&) = default;
			TypeVariant& operator=(const TypeVariant&) = default;

			virtual ~TypeVariant() = default;

			virtual const T& value() const
			{
				return value_;
			}

		protected:
			T value_;
		};

		class UndefinedVariant : public TypeVariant<void*>
		{
		public:
			explicit UndefinedVariant(void *value) : TypeVariant(value)
			{
			}
		};

		class BooleanVariant : public TypeVariant<bool>
		{
		public:
			BooleanVariant() : BooleanVariant(false)
			{
			}

			explicit BooleanVariant(bool value) : TypeVariant(value)
			{
			}
		};

		class StringVariant : public TypeVariant<::std::string>
		{
		public:
			explicit StringVariant(const ::std::string& value) : TypeVariant(value)
			{
			}
		};

		class NullObjectVariant : public TypeVariant<u8>
		{
		public:
			NullObjectVariant() : TypeVariant(0)
			{
			}
		};

		class NumberVariant : public TypeVariant<double>
		{
		public:
			explicit NumberVariant(double value) : TypeVariant(value)
			{
			}

			explicit NumberVariant(u8 value) : NumberVariant(static_cast<double>(value))
			{
			}

			explicit NumberVariant(u16 value) : NumberVariant(static_cast<double>(value))
			{
			}

			explicit NumberVariant(u32 value) : NumberVariant(static_cast<double>(value))
			{
			}

			explicit NumberVariant(u64 value) : NumberVariant(static_cast<double>(value))
			{
			}
		};

		using TVariantSubset = Variant<
			BooleanVariant,
			StringVariant,
			NumberVariant,
			NullObjectVariant,
			UndefinedVariant>;

		using TDictSubset = ::std::unordered_map<::std::string, TVariantSubset>;
		
		class EcmaArrayVariant : public TypeVariant<TDictSubset>
		{
		public:
			EcmaArrayVariant() = default;
			
			explicit EcmaArrayVariant(TDictSubset value) : TypeVariant(value)
			{
			}
		};

		class ObjectVariant : public TypeVariant<TDictSubset>
		{
		public:
			ObjectVariant() = default;
			
			explicit ObjectVariant(const TDictSubset& value) : TypeVariant(value)
			{
			}
		};

		using TVariant = Variant<
			BooleanVariant,
			StringVariant,
			NumberVariant,
			NullObjectVariant,
			UndefinedVariant,
			EcmaArrayVariant,
			ObjectVariant>;

		using TTypes = ::std::vector<TVariant>;

		using TDict = ::std::unordered_map<::std::string, TVariant>;
	} // end AMF
} // end RtmpKit
