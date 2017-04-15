#pragma once
#ifndef _HINODE_META_PROGRAMING_UTILITY_
#define _HINODE_META_PROGRAMING_UTILITY_

namespace hinode
{
	namespace graphics
	{
		template<int N>
		struct int_{
			static const int value = N;
			int operator()()
			{
				return N;
			}
		};

		template<bool N>
		struct bool_{
			static const bool value = N;
			using value_type = bool;
			bool operator()()
			{
				return N;
			}
		};

		using true_ = bool_ < true >;
		using false_ = bool_ < false >;
	}
}

#endif
