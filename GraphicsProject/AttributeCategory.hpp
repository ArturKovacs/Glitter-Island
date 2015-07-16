#pragma once

enum class AttributeCategory { _FIRST, POSITION, TEX_COORD, NORMAL, TANGENT /* calculate bitangent at vertex shader*/, _LAST };

struct AttributeCategoryInfo
{
	const int elementDimensions;

	AttributeCategoryInfo(int elementDimensions) : elementDimensions(elementDimensions){}
};

AttributeCategoryInfo GetAttributeCategoryInfo(AttributeCategory attrib);

template<typename FunctionType>
static void forEachAttribute(FunctionType f)
{
	for (int i = static_cast<int>(AttributeCategory::_FIRST) + 1; i != static_cast<int>(AttributeCategory::_LAST); i++) {
		f(static_cast<AttributeCategory>(i));
	}
}
