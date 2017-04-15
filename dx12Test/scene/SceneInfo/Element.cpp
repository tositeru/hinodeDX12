#include "stdafx.h"

#include "Element.h"

namespace scene {
}

ENUM_HASH_TYPE(scene::ELEMENT_TYPE) = {
	{ "object", scene::ELEMENT_TYPE::eOBJECT },
	{ "model", scene::ELEMENT_TYPE::eMODEL },
	{ "pipeline", scene::ELEMENT_TYPE::ePIPELINE },
	{ "render_phase", scene::ELEMENT_TYPE::eRENDER_PHASE },
};
