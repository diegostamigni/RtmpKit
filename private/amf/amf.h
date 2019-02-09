//
//  amf.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 23/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/utils.h"
#include "../../private/amf/types.h"

#include <iterator>
#include <type_traits>

namespace RtmpKit
{
	namespace AMF
	{
		template <typename DType, typename = std::enable_if<std::is_enum<DType>::value>>
		class Base
		{
		public:
			virtual ~Base() = default;

			DType fromMarker(u8 item) const
			{
				return static_cast<DType>(item);
			}

			u8 toMarker(DType item) const
			{
				return static_cast<u8>(item);
			}

			virtual std::string toString(const v8& data, std::size_t lenght) const
			{
				return std::string({data.cbegin(), data.cbegin() + lenght});
			}

			virtual v8 fromString(const std::string &data) const
			{
				auto result = v8(data.size(), 0);
				std::copy(data.cbegin(), data.cend(), result.begin());
				return result;
			}

			virtual bool toBoolean(u8 value) const
			{
				return value > 0;
			}

			virtual u8 fromBoolean(bool value) const
			{
				return static_cast<u8>(value);
			}

			virtual v8 fromNumber(double data) const
			{
				auto conversion = v8(sizeof(double), 0);
				::memcpy(conversion.data(), &data, sizeof(double));
				return conversion;
			}

			virtual double toNumber(const v8& data, std::size_t lenght) const
			{
				auto res = fromBigEndian64(data);
				double value = 0.;
				std::copy(res.cbegin(), res.cbegin() + lenght, reinterpret_cast<u8 *>(&value));
				return value;
			}

			/*!
			 * @brief Returns the lenght from a certain payload in u16
			 * @param data the buffer where getting the length; request size is >= 2
			 * @return the length
			 */
			u32 length16(const v8& data) const
			{
				return data.size() < 2 ? 0 : fromBigEndian16({data.cbegin(), data.cbegin() + 2});
			}

			/*!
			 * @brief Returns the lenght from a certain payload in u24
			 * @param data the buffer where getting the length; request size is >= 3
			 * @return the length
			 */
			u32 length24(const v8& data) const
			{
				return data.size() < 3 ? 0 : fromBigEndian24({data.cbegin(), data.cbegin() + 3});
			}

			/*!
			 * @brief Returns the lenght from a certain payload in u32
			 * @param data the buffer where getting the length; request size is >= 4
			 * @return the length
			 */
			u32 length32(const v8& data) const
			{
				return data.size() < 4 ? 0 : fromBigEndian32({data.cbegin(), data.cbegin() + 4});
			}
		};

		struct BaseDeserializer
		{
			virtual ~BaseDeserializer() = default;
			virtual TTypes deserialize(const v8& data) const = 0;
		};

		struct BaseSerializer
		{
			virtual ~BaseSerializer() = default;
			virtual v8 serialize(const TDict& data) const = 0;
		};

		namespace v0
		{
			enum class DataType : u8
			{
				NumberMarker = 0x00,
				BooleanMarker = 0x01,
				StringMarker = 0x02,
				ObjectMarker = 0x03,
				MovieclipMarker = 0x04,
				NullMarker = 0x05,
				UndefinedMarker = 0x06,
				ReferenceMarker = 0x07,
				EcmaArrayMarker = 0x08,
				ObjectEndMarker = 0x09,
				StrictArrayMarker = 0x0A,
				DateMarker = 0x0B,
				LongStringMarker = 0x0C,
				UnsupportedMarker = 0x0D,
				RecordsetMarker = 0x0E,
				XmlDocumentMarker = 0x0F,
				TypedObjectMarker = 0x10,
			};

			class Deserializer : public Base<DataType>, public BaseDeserializer
			{
			public:
				Deserializer() = default;

				Deserializer(const Deserializer&) = default;

				Deserializer(Deserializer&&) = default;

				Deserializer& operator=(const Deserializer&) = default;

				Deserializer& operator=(Deserializer&&) = default;

				TTypes deserialize(const v8& data) const override;

			protected:
				virtual TVariant deserialize(const v8& data, v8::const_iterator &it) const;

				virtual TVariantSubset deserializeSubset(const v8& data, v8::const_iterator& iter) const;

				// TODO(diegostamigni): implement the serializer/deserializer for the missing objects, check DataType for more details

				virtual EcmaArrayVariant deserializeEcmaArray(const v8& data, v8::const_iterator& iter, u32 size) const;

				virtual StringVariant deserializeString(const v8& data, v8::const_iterator& iter) const;

				virtual NumberVariant deserializeNumber(const v8& data, v8::const_iterator& iter) const;

				virtual BooleanVariant deserializeBoolean(const v8& data, v8::const_iterator& iter) const;

				virtual ObjectVariant deserializeObject(const v8& data, v8::const_iterator& iter) const;

				virtual NullObjectVariant deserializeNullObject(const v8& data, v8::const_iterator& iter) const;
			};

			class Serializer : public Base<DataType>, public BaseSerializer
			{
			public:
				Serializer() = default;

				Serializer(const Serializer&) = default;

				Serializer(Serializer&&) = default;

				Serializer& operator=(const Serializer&) = default;

				Serializer& operator=(Serializer&&) = default;

				virtual v8 serialize(const TDict& data) const override;

				virtual v8 serializeEcmaArray(const TDictSubset& data) const;

				virtual v8 serializeString(const std::string& data, bool includeMarker = true) const;

				virtual v8 serializeNumber(double data) const;

				virtual v8 serializeBoolean(bool data) const;

				virtual v8 serializeObject(const TDictSubset& properties) const;

				virtual v8 serializeNullObject() const;

			protected:
				virtual v8 serializeVariant(const TVariant &item) const;

				virtual v8 serializeSubset(const TVariantSubset& data) const;
			};
		} // end v0

		namespace v3
		{
			// TODO(diegostamigni): implement
		} // end v3
	} // end AMF
} // end RtmpKit
