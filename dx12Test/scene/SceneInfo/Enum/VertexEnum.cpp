#include "stdafx.h"

#include "VertexEnum.h"

namespace scene
{

}

ENUM_HASH_TYPE(scene::VERTEX) = {
	{ "p", scene::VERTEX::eP },
	{ "pc", scene::VERTEX::ePC },
	{ "pn", scene::VERTEX::ePN },
	{ "pt", scene::VERTEX::ePT },
	{ "ptn", scene::VERTEX::ePTN },
	{ "ptnc", scene::VERTEX::ePTNC },
};

