#pragma once

#include "Enum.h"

namespace scene
{
	enum class VERTEX : size_t {
		eP,
		ePT,
		ePC,
		ePN,
		ePTN,
		ePTNC,
	};

	DEFINE_ENUM_CLASS(Vertex, VERTEX);

}