//
//  amf0_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 1/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"
#include "../../private/amf/amf.h"

#include <utility>

TEST(AMF0, Number)
{
	const RtmpKit::u32 value = 3191;

	const auto serializer = RtmpKit::AMF::v0::Serializer{};
	const auto serialized = serializer.serializeNumber(value);

	const auto deserializer = RtmpKit::AMF::v0::Deserializer{};
	const auto deserialized = deserializer.deserialize(serialized);

	ASSERT_EQ(deserialized.size(), 1ul);
	const auto *variant = boost::get<RtmpKit::AMF::NumberVariant>(&(deserialized.front()));

	ASSERT_TRUE(variant != nullptr);
	ASSERT_EQ(variant->value(), value);
}

TEST(AMF0, String)
{
	const std::string value("570686e3-43c1-4a18-b2d3-b029bc3d1f25?RtmpKitChannelId=f2ac27e5-6e27-4901-8614-52b7c71a68ef&RtmpKitClientId=2A87B538-D388-4E19-81D0-972C9690B38F&AuthToken=L2Jc7Duuy1Ln34TXYFhyGdtWhpJrOZlomWUuMolYdPXoGfjsq4p005jqh8yMzToq0K2S7nwvRfbJ9FPVf5jIjw%253D%253D");

	const auto serializer = RtmpKit::AMF::v0::Serializer{};
	const auto serialized = serializer.serializeString(value);

	const auto deserializer = RtmpKit::AMF::v0::Deserializer{};
	const auto deserialized = deserializer.deserialize(serialized);

	ASSERT_EQ(deserialized.size(), 1ul);
	const auto *variant = boost::get<RtmpKit::AMF::StringVariant>(&(deserialized.front()));

	ASSERT_TRUE(variant != nullptr);
	ASSERT_EQ(variant->value(), value);
}

TEST(AMF0, Boolean)
{
	const bool value = true;

	const auto serializer = RtmpKit::AMF::v0::Serializer{};
	const auto serialized = serializer.serializeBoolean(value);

	const auto deserializer = RtmpKit::AMF::v0::Deserializer{};
	const auto deserialized = deserializer.deserialize(serialized);

	ASSERT_EQ(deserialized.size(), 1ul);
	const auto *variant = boost::get<RtmpKit::AMF::BooleanVariant>(&(deserialized.front()));

	ASSERT_TRUE(variant != nullptr);
	ASSERT_EQ(variant->value(), value);
}

TEST(AMF0, Object)
{
	const auto namePair = std::make_pair("name", RtmpKit::AMF::StringVariant("Mike"));
	const auto agePair = std::make_pair("age", RtmpKit::AMF::NumberVariant(30u));
	const auto aliasPair = std::make_pair("alias", RtmpKit::AMF::StringVariant("Micky"));
	const auto value = RtmpKit::AMF::TDictSubset{ namePair, agePair, aliasPair };

	const auto serializer = RtmpKit::AMF::v0::Serializer{};
	const auto serialized = serializer.serializeObject(value);

	const auto deserializer = RtmpKit::AMF::v0::Deserializer{};
	const auto deserialized = deserializer.deserialize(serialized);

	ASSERT_EQ(deserialized.size(), 1ul);
	const auto *variant = boost::get<RtmpKit::AMF::ObjectVariant>(&(deserialized.front()));

	ASSERT_TRUE(variant != nullptr);

	const auto &dict = variant->value();
	ASSERT_EQ(dict.size(), value.size());

	const auto &nameVariant = dict.at(namePair.first);
	const auto *nameStringVariant = boost::get<decltype(namePair.second)>(&nameVariant);
	ASSERT_TRUE(nameStringVariant != nullptr);
	ASSERT_EQ(nameStringVariant->value(), namePair.second.value());

	const auto &ageVariant = dict.at(agePair.first);
	const auto *ageNumberVariant = boost::get<decltype(agePair.second)>(&ageVariant);
	ASSERT_TRUE(ageNumberVariant != nullptr);
	ASSERT_EQ(ageNumberVariant->value(), agePair.second.value());

	const auto &aliasVariant = dict.at(aliasPair.first);
	const auto *aliasStringVariant = boost::get<decltype(aliasPair.second)>(&aliasVariant);
	ASSERT_TRUE(aliasStringVariant != nullptr);
	ASSERT_EQ(aliasStringVariant->value(), aliasPair.second.value());
}
