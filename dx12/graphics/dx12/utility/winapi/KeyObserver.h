#pragma once

#include <list>
#include <functional>
#include <Windows.h>

#include "../../common/Common.h"
#include "../../utility/math/SimpleMath.h"

namespace hinode
{
	namespace winapi
	{
		class KeyDownObserver
		{
		public:
			KeyDownObserver(int key, bool isOn = true, std::function<void(const KeyDownObserver&)> event = [](const KeyDownObserver&) {})noexcept
				: mKey(key)
				, mOn(isOn)
				, mEvent(event)
			{}
			KeyDownObserver(const KeyDownObserver& original)
				: mKey(original.mKey)
				, mOn(original.mOn)
				, mEvent(original.mEvent)
			{}

			virtual void update()noexcept {
				if (0x1 & GetAsyncKeyState(this->mKey)) {
					this->mOn = !this->mOn;
					this->mEvent(*this);
				}
			}
			operator bool()const noexcept {
				return this->mOn;
			}

		protected:
			int getKey()const { return this->mKey; }

		private:
			int mKey;
			bool mOn;
			std::function<void(const KeyDownObserver&)> mEvent;
		};

		class MouseObserver : public KeyDownObserver
		{
		public:
			static auto sGetCursor() noexcept -> hinode::math::float2 {
				POINT p;
				GetCursorPos(&p);
				return hinode::math::float2{ (float)p.x, (float)p.y };
			}

		public:
			MouseObserver(int key, bool isOn = true, std::function<void(const MouseObserver&)> event = [](const MouseObserver&) {})noexcept
				: KeyDownObserver(key, isOn)
				, mEvent(event)
			{}
			MouseObserver(const MouseObserver& original)noexcept
				: KeyDownObserver(original.getKey(), original)
				, mEvent(original.mEvent)
			{}

			virtual void update()noexcept override {
				if (0x8000 & GetAsyncKeyState(this->getKey())) {
					this->mEvent(*this);
				}
				this->mPrevCursorPos = MouseObserver::sGetCursor();
			}

			hinode::math::float2 getMove()const
			{
				return MouseObserver::sGetCursor() - this->mPrevCursorPos;
			}

		accessor_declaration:
			const hinode::math::float2& prevCursorPos()const {
				return this->mPrevCursorPos;
			}

		private:
			hinode::math::float2 mPrevCursorPos;
			std::function<void(const MouseObserver&)> mEvent;

		};

		class KeyDownObserverManager
		{
		public:
			KeyDownObserverManager()noexcept {}

			void update()noexcept
			{
				for (auto& it : this->mpList) {
					it->update();
				}
			}

			const KeyDownObserver& add(int key, bool isOn = true, std::function<void(const KeyDownObserver&)> event = [](const KeyDownObserver&) {})noexcept
			{
				auto p = std::make_shared<KeyDownObserver>(key, isOn, event);
				this->mpList.push_back(p);
				return *this->mpList.back();
			}
			const KeyDownObserver& add(const KeyDownObserver& key)noexcept
			{
				auto p = std::make_shared<KeyDownObserver>(key);
				this->mpList.push_back(p);
				return *this->mpList.back();
			}

			const MouseObserver& addMouse(int key, bool isOn = true, std::function<void(const MouseObserver&)> event = [](const MouseObserver&) {})noexcept
			{
				std::shared_ptr<KeyDownObserver> p = std::make_shared<MouseObserver>(key, isOn, event);
				this->mpList.push_back(p);
				return *((MouseObserver*)p.get());
			}
			const MouseObserver& addMouse(const MouseObserver& key)noexcept
			{
				std::shared_ptr<KeyDownObserver> p = std::make_shared<MouseObserver>(key);
				this->mpList.push_back(p);
				return *((MouseObserver*)p.get());
			}
		private:
			std::list<std::shared_ptr<KeyDownObserver>> mpList;
		};

	}
}