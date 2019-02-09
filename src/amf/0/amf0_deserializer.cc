#include "../../../private/amf/amf.h"
#include "../../../private/logger.h"

#include <iostream>

using namespace RtmpKit;
using namespace AMF;
using namespace v0;

TTypes Deserializer::deserialize(const v8& data) const
{
	auto iter = data.cbegin();
	auto result = TTypes{};

	try
	{
		while (iter < data.cend())
		{
			const auto item = deserialize(data, iter);
			result.push_back(item);
		}
	}
	catch (const std::exception &e)
	{
		LOG_ERROR(e.what());
	}

	return result;
}

TVariant Deserializer::deserialize(const v8& data, v8::const_iterator& iter) const
{
	if (iter >= data.cend() || std::distance(iter, data.cend()) < 1)
	{
		LOG_WARNING("Invalid iterator");
		return UndefinedVariant(nullptr);
	}

	auto marker = this->fromMarker(*iter);
	iter += 1;

	switch (marker)
	{
	case v0::DataType::StringMarker:
		return deserializeString(data, iter);

	case v0::DataType::NumberMarker:
		return this->deserializeNumber(data, iter);

	case v0::DataType::BooleanMarker:
		return this->deserializeBoolean(data, iter);

	case v0::DataType::ObjectMarker:
		return this->deserializeObject(data, iter);

	case v0::DataType::NullMarker:
		return this->deserializeNullObject(data, iter);

	case v0::DataType::EcmaArrayMarker:
	{
		if (iter == data.cend() || std::distance(iter, data.cend()) < 4)
		{
			LOG_WARNING("Invalid iterator");
			return UndefinedVariant(nullptr);
		}

		auto associativeCount = this->length32({iter, iter + 4});
		iter += 4;

		return deserializeEcmaArray(data, iter, associativeCount);
	}

	default:
		LOG_WARNING("Invalid marker");
		return UndefinedVariant(nullptr);
	}
}

TVariantSubset Deserializer::deserializeSubset(const v8& data, v8::const_iterator& iter) const
{
	if (iter >= data.cend() || std::distance(iter, data.cend()) < 1)
	{
		LOG_WARNING("Invalid iterator");
		return UndefinedVariant(nullptr);
	}

	auto marker = this->fromMarker(*iter);
	iter += 1;

	switch (marker)
	{
	case v0::DataType::StringMarker:
		return deserializeString(data, iter);

	case v0::DataType::NumberMarker:
		return this->deserializeNumber(data, iter);

	case v0::DataType::BooleanMarker:
		return this->deserializeBoolean(data, iter);

	case v0::DataType::NullMarker:
		return this->deserializeNullObject(data, iter);

	default:
		LOG_WARNING("Invalid marker");
		return UndefinedVariant(nullptr);
	}
}

EcmaArrayVariant Deserializer::deserializeEcmaArray(const v8& data, v8::const_iterator& iter, u32 size) const
{
	auto result = TDictSubset{};

	for (u32 i = 0; i < size; ++i)
	{
		if (iter == data.cend() || std::distance(iter, data.cend()) < 2)
		{
			return EcmaArrayVariant(result);
		}

		auto lenght = this->length16({iter, iter + 2});
		iter += 2;
		
		if (iter == data.cend() 
			|| std::distance(iter, data.cend()) < static_cast<v8::const_iterator::difference_type>(lenght))
		{
			return EcmaArrayVariant(result);
		}

		auto key = this->toString({iter, iter + lenght}, lenght);
		iter += lenght;

		auto value = deserializeSubset(data, iter);
		result[key] = value;
	}

	return EcmaArrayVariant(result);
}

StringVariant Deserializer::deserializeString(const v8& data, v8::const_iterator& iter) const
{
	if (iter == data.cend() || std::distance(iter, data.cend()) < 2)
	{
		LOG_WARNING("Invalid iterator");
		StringVariant("");
	}

	auto lenght = this->length16({iter, iter + 2});
	iter += 2;

	if (iter == data.cend()
		|| std::distance(iter, data.cend()) < static_cast<v8::const_iterator::difference_type>(lenght))
	{
		LOG_WARNING("Invalid iterator");
		StringVariant("");
	}

	auto value = this->toString({iter, iter + lenght}, lenght);
	iter += lenght;

	return StringVariant(value);
}

NumberVariant Deserializer::deserializeNumber(const v8& data, v8::const_iterator& iter) const
{
	if (iter == data.cend() || std::distance(iter, data.cend()) < 8)
	{
		LOG_WARNING("Invalid iterator");
		return NumberVariant(0.);
	}

	auto number = this->toNumber({iter, iter + 8}, 8);
	iter += 8;

	return NumberVariant(number);
}

BooleanVariant Deserializer::deserializeBoolean(const v8& data, v8::const_iterator& iter) const
{
	if (iter == data.cend())
	{
		LOG_WARNING("Invalid iterator");
		return BooleanVariant(false);
	}

	auto value = this->toBoolean(*iter);
	iter += 1;

	return BooleanVariant(value);
}

ObjectVariant Deserializer::deserializeObject(const v8& data, v8::const_iterator& iter) const
{
	// { key = [length, string] : value = [TVariant] }
	auto dict = TDictSubset{};

	while (*iter != static_cast<u8>(DataType::ObjectEndMarker))
	{
		const auto key = deserializeString(data, iter);

		if (!key.value().empty())
		{
			const auto value = deserializeSubset(data, iter);
			dict[key.value()] = value;
		}
	}

	iter += 1;
	return ObjectVariant{dict};
}

NullObjectVariant Deserializer::deserializeNullObject(const v8& data, v8::const_iterator& iter) const
{
	return NullObjectVariant{};
}
