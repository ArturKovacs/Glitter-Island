#include "AttributeCategory.hpp"

AttributeCategoryInfo GetAttributeCategoryInfo(AttributeCategory attrib)
{
	switch (attrib)
	{
	case AttributeCategory::TEX_COORD:
		return AttributeCategoryInfo(2);
		break;

	default:
		return AttributeCategoryInfo(3);
		break;
	}
}
