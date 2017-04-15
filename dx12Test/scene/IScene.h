#pragma once

#include <graphics\dx12\utility\DX12Utility.h>

namespace hinode {
	namespace graphics {
		namespace utility {
			class Graphics;
		}
	}
}

namespace scene
{
	class SceneInfo;
}

using namespace hinode;
using namespace hinode::graphics;

class IScene
{
public:
	static size_t sGetStartSceneNo()noexcept;
	static size_t sClampSceneNo(size_t sceneNo)noexcept;
	static std::unique_ptr<IScene> sBuild(size_t sceneNo);

public:
	virtual ~IScene() {}

	virtual void clear() = 0;
	virtual void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo) = 0;
	virtual bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo) = 0;
};