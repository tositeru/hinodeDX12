///
///	Root Signature用のマクロをまとめたヘッダー
///

#ifndef __cplusplus

#define _MERGE(a, b) a##b
#define CM ","
#define COMMON_SPACE_NO 0
#define VS_SPACE_NO 1
#define HS_SPACE_NO 2
#define DS_SPACE_NO 3
#define GS_SPACE_NO 4
#define PS_SPACE_NO 5
#define CS_SPACE_NO 6

#define COMMON_SPACE _MERGE(space, COMMON_SPACE_NO)
#define VS_SPACE _MERGE(space, VS_SPACE_NO)
#define HS_SPACE _MERGE(space, HS_SPACE_NO)
#define DS_SPACE _MERGE(space, DS_SPACE_NO)
#define GS_SPACE _MERGE(space, GS_SPACE_NO)
#define PS_SPACE _MERGE(space, PS_SPACE_NO)
#define CS_SPACE _MERGE(space, CS_SPACE_NO)

#define ROOT_FLAGS(flags) "RootFlags("#flags")"
#define ROOT_CBV(slot, space, visibility, flags) "CBV("#slot", space="#space", visibility="#visibility", flags="#flags")"
#define ROOT_SRV(slot, space, visibility, flags) "SRV("#slot", space="#space", visibility="#visibility", flags="#flags")"
#define ROOT_UAV(slot, space, visibility, flags) "UAV("#slot", space="#space", visibility="#visibility", flags="#flags")"
#define ROOT_CONSTANTS(slot, space, count32Bit) "RootConstants(num32BitConstants="#count32Bit", "#slot", space="#space")"
#define DESCRIPTOR_TABLE(visibility) "DescriptorTable(visibility="#visibility","
#define DESCRIPTOR_TABLE_END ")"
#define END_DESCRIPTOR_TABLE ")"
#define CBV(slot, space, count, offset, flags) "CBV("#slot", space="#space", numDescriptors="#count", offset="#offset", flags="#flags")"
#define SRV(slot, space, count, offset, flags) "SRV("#slot", space="#space", numDescriptors="#count", offset="#offset", flags="#flags")"
#define UAV(slot, space, count, offset, flags) "UAV("#slot", space="#space", numDescriptors="#count", offset="#offset", flags="#flags")"
#define SAMPLER(slot, space, count, offset) "Sampler("#slot", space="#space", numDescriptors="#count", offset="#offset")"
//#define SAMPLER(slot, space, count, offset, flags) "Sampler("#slot", space="#space", numDescriptors="#count", offset="#offset", flags="#flags")" //サンプラにはフラグを設定できないみたいので、削除

/// ROOT_FLAGS関係
#define DENY_GRAPHICS_SHADER_ROOT_ACCESS DENY_VERTEX_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_PIXEL_SHADER_ROOT_ACCESS

#define STATIC_SAMPLER(slot, space, visibility, filter, address) \
	"StaticSampler("#slot", space="#space", visibility="#visibility"," \
	"  filter="#filter"," \
	"  addressU="#address"," \
	"  addressV="#address"," \
	"  addressW="#address"," \
	"  mipLODBias=0.f," \
	"  comparisonFunc=COMPARISON_NEVER," \
	"  borderColor=STATIC_BORDER_COLOR_TRANSPARENT_BLACK," \
	"  minLOD=0.f," \
	"  maxLOD=1000.f)"

#endif