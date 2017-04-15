#pragma once

#undef unless
#define unless(terms) if(!(terms))

#undef accessor_declaration
#define accessor_declaration public

#ifdef _MSC_VER
#undef _noexcept
#define _noexcept throw()
#endif

