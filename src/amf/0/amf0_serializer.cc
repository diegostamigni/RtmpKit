#include "../../../private/amf/amf.h"
#include "../../../private/logger.h"

#include <iostream>

using namespace RtmpKit;
using namespace AMF;
using namespace v0;

v8 Serializer::serialize(const TDict& data) const
{
	auto result = v8{};
	result.reserve(data.size());

	try
	{
		for (const auto& kv : data)
		{
			auto r = serializeVariant(kv.second);
			std::move(r.begin(), r.end(), std::back_inserter(result));
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(e.what());
	}

	return result;
}

auto Serializer::serializeVariant(const TVariant &item) const -> v8
{
	auto result = v8{};

	const auto* arr = boost::get<EcmaArrayVariant>(&item);
	if (arr != nullptr)
	{
		auto r = serializeEcmaArray(arr->value());
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	const auto* str = boost::get<StringVariant>(&item);
	if (str != nullptr)
	{
		auto r = serializeString(str->value());
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	const auto* num = boost::get<NumberVariant>(&item);
	if (num != nullptr)
	{
		auto r = serializeNumber(num->value());
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	const auto* b = boost::get<BooleanVariant>(&item);
	if (b != nullptr)
	{
		auto r = serializeBoolean(b->value());
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	const auto* obj = boost::get<ObjectVariant>(&item);
	if (obj != nullptr)
	{
		auto r = serializeObject(obj->value());
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	const auto* nul = boost::get<NullObjectVariant>(&item);
	if (nul != nullptr)
	{
		auto r = serializeNullObject();
		std::move(r.begin(), r.end(), std::back_inserter(result));
	}

	return result;
}

auto Serializer::serializeSubset(const TVariantSubset& data) const -> v8
{
	// TODO(diegostamigni): implement
	return {};
}

auto Serializer::serializeEcmaArray(const TDictSubset& data) const -> v8
{
	// TODO(diegostamigni): implement
	return {};
}

auto Serializer::serializeString(const std::string& data, bool includeMarker) const -> v8
{
	auto size = data.length() + (includeMarker ? 3 : 2);
	auto result = v8(size, 0);
	auto iter = result.begin();

	if (includeMarker)
	{
		*iter = toMarker(DataType::StringMarker);
		iter += 1;
	}

	auto lenght = RtmpKit::toBigEndian16(static_cast<u16>(data.length()));
	std::move(lenght.begin(), lenght.end(), iter);
	iter += 2;

	auto converted = fromString(data);
	std::move(converted.begin(), converted.end(), iter);
	return result;
}

auto Serializer::serializeNumber(double data) const -> v8
{
	auto result = v8(9, 0);
	auto iter = result.begin();

	*iter = toMarker(DataType::NumberMarker);
	iter += 1;

	auto conversion = fromNumber(data);
	std::reverse_copy(conversion.cbegin(), conversion.cend(), iter);

	return result;
}

auto Serializer::serializeBoolean(bool data) const -> v8
{
	auto result = v8(2, 0);
	auto iter = result.begin();

	*iter = toMarker(DataType::BooleanMarker);
	iter += 1;
	*iter = fromBoolean(data);

	return result;
}

auto Serializer::serializeObject(const TDictSubset& properties) const -> v8
{
	auto result = v8{};
	result.push_back(toMarker(DataType::ObjectMarker));

	for (const auto &itemPair : properties)
	{
		auto keyConverted = serializeString(itemPair.first, false);
		std::move(keyConverted.begin(), keyConverted.end(), std::back_inserter(result));

		auto valueConverted = serializeVariant(itemPair.second);
		std::move(valueConverted.begin(), valueConverted.end(), std::back_inserter(result));
	}

	auto emptyStr = serializeString("", false);
	std::move(emptyStr.begin(), emptyStr.end(), std::back_inserter(result));
	result.push_back(toMarker(DataType::ObjectEndMarker));

	return result;
}

auto Serializer::serializeNullObject() const -> v8
{
	return v8(1, toMarker(DataType::NullMarker));
}
