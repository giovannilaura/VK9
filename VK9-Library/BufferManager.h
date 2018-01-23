/*VkPipelineShaderStageCreateInfo
Copyright(c) 2016 Christopher Joseph Dean Schaefer

This software is provided 'as-is', without any express or implied
warranty.In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software.If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#define UBO_SIZE 64

#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>
#include <Eigen/Dense>
#include <memory>
#include <chrono>

#include "CTypes.h"
#include "CIndexBuffer9.h"

class CDevice9;

struct SamplerRequest
{
	//Vulkan State
	VkSampler Sampler = VK_NULL_HANDLE;

	//D3D9 State
	DWORD SamplerIndex = 0;
	D3DTEXTUREFILTERTYPE MagFilter = D3DTEXF_NONE;
	D3DTEXTUREFILTERTYPE MinFilter = D3DTEXF_NONE;
	D3DTEXTUREADDRESS AddressModeU = D3DTADDRESS_FORCE_DWORD;
	D3DTEXTUREADDRESS AddressModeV = D3DTADDRESS_FORCE_DWORD;
	D3DTEXTUREADDRESS AddressModeW = D3DTADDRESS_FORCE_DWORD;
	DWORD MaxAnisotropy = 0;
	D3DTEXTUREFILTERTYPE MipmapMode = D3DTEXF_NONE;
	float MipLodBias = 0.0f;
	float MaxLod = 1.0f;

	//Resource Handling.
	std::chrono::steady_clock::time_point LastUsed = std::chrono::steady_clock::now();
	CDevice9* mDevice = nullptr;
	SamplerRequest(CDevice9* device) : mDevice(device) {}
	~SamplerRequest();
};

struct ResourceContext
{
	VkDescriptorImageInfo DescriptorImageInfo[16] = {};
	
	//Vulkan State
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
	BOOL WasShader = false; // descriptor set logic is different for shaders so mixing them makes Vulkan angry because the number of attachment is different and stuff.

	//Resource Handling.
	std::chrono::steady_clock::time_point LastUsed = std::chrono::steady_clock::now();
	CDevice9* mDevice = nullptr;
	ResourceContext(CDevice9* device) : mDevice(device) {}
	~ResourceContext();
};

struct DrawContext
{
	//Vulkan State
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

	//Misc
	//boost::container::flat_map<UINT, UINT> Bindings;
	UINT Bindings[64] = {};

	//D3D9 State - Pipe
	D3DPRIMITIVETYPE PrimitiveType = D3DPT_FORCE_DWORD;
	DWORD FVF = 0;
	CVertexDeclaration9* VertexDeclaration = nullptr;
	CVertexShader9* VertexShader = nullptr;
	CPixelShader9* PixelShader = nullptr;
	int32_t StreamCount = 0;

	//D3d9 State - Lights
	ShaderConstantSlots mVertexShaderConstantSlots = {};
	ShaderConstantSlots mPixelShaderConstantSlots = {};

	//Constant Registers
	SpecializationConstants mSpecializationConstants = {};

	//Resource Handling.
	std::chrono::steady_clock::time_point LastUsed = std::chrono::steady_clock::now();
	CDevice9* mDevice = nullptr;
	DrawContext(CDevice9* device) : mDevice(device) {}
	~DrawContext();
};

class BufferManager
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		BufferManager();
	explicit BufferManager(CDevice9* device);
	~BufferManager();

	VkResult mResult = VK_SUCCESS;

	CDevice9* mDevice = nullptr;
	bool mIsDirty = true;

	VkDynamicState mDynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE] = {};
	VkPipelineColorBlendAttachmentState mPipelineColorBlendAttachmentState[1] = {};

	VkPipelineVertexInputStateCreateInfo mPipelineVertexInputStateCreateInfo = {};
	VkPipelineInputAssemblyStateCreateInfo mPipelineInputAssemblyStateCreateInfo = {};
	VkPipelineRasterizationStateCreateInfo mPipelineRasterizationStateCreateInfo = {};
	VkPipelineColorBlendStateCreateInfo mPipelineColorBlendStateCreateInfo = {};
	VkPipelineDepthStencilStateCreateInfo mPipelineDepthStencilStateCreateInfo = {};
	VkPipelineViewportStateCreateInfo mPipelineViewportStateCreateInfo = {};
	VkPipelineMultisampleStateCreateInfo mPipelineMultisampleStateCreateInfo = {};
	VkPipelineDynamicStateCreateInfo mPipelineDynamicStateCreateInfo = {};
	VkGraphicsPipelineCreateInfo mGraphicsPipelineCreateInfo = {};
	VkPipelineCacheCreateInfo mPipelineCacheCreateInfo = {};

	VkPipelineShaderStageCreateInfo mPipelineShaderStageCreateInfo[2] = {};

	VkDescriptorSetAllocateInfo mDescriptorSetAllocateInfo = {};
	VkDescriptorSetLayoutBinding mDescriptorSetLayoutBinding[16] = {};
	VkDescriptorSetLayoutCreateInfo mDescriptorSetLayoutCreateInfo = {};
	VkPipelineLayoutCreateInfo mPipelineLayoutCreateInfo = {};
	VkWriteDescriptorSet mWriteDescriptorSet[3] = {};
	VkPushConstantRange mPushConstantRanges[2] = {};
	VkDescriptorBufferInfo mDescriptorBufferInfo[2] = {};

	//Created with max slots. I can pass a count to limit the number. This should prevent me from needing to realloc.
	VkVertexInputBindingDescription mVertexInputBindingDescription[16] = {};
	VkVertexInputAttributeDescription mVertexInputAttributeDescription[32] = {};

	//Command Buffer & Buffer Copy Setup
	VkCommandBufferAllocateInfo mCommandBufferAllocateInfo = {};
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
	VkCommandBufferBeginInfo mBeginInfo = {};
	VkBufferCopy mCopyRegion = {};
	VkSubmitInfo mSubmitInfo = {};

	//VkDescriptorSetLayout mDescriptorSetLayout;
	//VkPipelineLayout mPipelineLayout;
	VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
	//VkDescriptorSet mDescriptorSet;
	//VkPipeline mPipeline;

	VkShaderModule mVertShaderModule_XYZ_DIFFUSE = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_DIFFUSE = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_TEX1 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_TEX1 = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_TEX2 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_TEX2 = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_DIFFUSE_TEX1 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_DIFFUSE_TEX1 = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_DIFFUSE_TEX2 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_DIFFUSE_TEX2 = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_NORMAL = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_NORMAL = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_NORMAL_TEX1 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_NORMAL_TEX1 = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_NORMAL_DIFFUSE = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_NORMAL_DIFFUSE = VK_NULL_HANDLE;

	VkShaderModule mVertShaderModule_XYZ_NORMAL_DIFFUSE_TEX2 = VK_NULL_HANDLE;
	VkShaderModule mFragShaderModule_XYZ_NORMAL_DIFFUSE_TEX2 = VK_NULL_HANDLE;

	//Seems like there should be a way to generate these with constexpr.

	const VkSpecializationMapEntry mSlotMapEntries[1024] =
	{
		{ 0   , 0   , sizeof(uint32_t) },
		{ 1   , 1 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 2   , 2 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 3   , 3 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 4   , 4 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 5   , 5 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 6   , 6 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 7   , 7 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 8   , 8 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 9   , 9 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 10  , 10 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 11  , 11 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 12  , 12 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 13  , 13 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 14  , 14 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 15  , 15 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 16  , 16 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 17  , 17 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 18  , 18 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 19  , 19 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 20  , 20 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 21  , 21 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 22  , 22 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 23  , 23 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 24  , 24 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 25  , 25 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 26  , 26 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 27  , 27 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 28  , 28 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 29  , 29 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 30  , 30 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 31  , 31 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 32  , 32 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 33  , 33 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 34  , 34 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 35  , 35 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 36  , 36 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 37  , 37 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 38  , 38 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 39  , 39 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 40  , 40 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 41  , 41 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 42  , 42 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 43  , 43 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 44  , 44 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 45  , 45 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 46  , 46 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 47  , 47 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 48  , 48 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 49  , 49 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 50  , 50 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 51  , 51 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 52  , 52 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 53  , 53 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 54  , 54 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 55  , 55 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 56  , 56 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 57  , 57 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 58  , 58 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 59  , 59 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 60  , 60 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 61  , 61 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 62  , 62 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 63  , 63 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 64  , 64 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 65  , 65 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 66  , 66 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 67  , 67 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 68  , 68 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 69  , 69 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 70  , 70 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 71  , 71 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 72  , 72 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 73  , 73 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 74  , 74 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 75  , 75 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 76  , 76 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 77  , 77 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 78  , 78 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 79  , 79 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 80  , 80 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 81  , 81 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 82  , 82 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 83  , 83 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 84  , 84 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 85  , 85 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 86  , 86 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 87  , 87 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 88  , 88 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 89  , 89 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 90  , 90 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 91  , 91 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 92  , 92 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 93  , 93 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 94  , 94 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 95  , 95 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 96  , 96 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 97  , 97 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 98  , 98 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 99  , 99 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 100 , 100 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 101 , 101 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 102 , 102 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 103 , 103 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 104 , 104 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 105 , 105 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 106 , 106 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 107 , 107 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 108 , 108 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 109 , 109 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 110 , 110 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 111 , 111 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 112 , 112 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 113 , 113 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 114 , 114 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 115 , 115 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 116 , 116 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 117 , 117 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 118 , 118 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 119 , 119 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 120 , 120 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 121 , 121 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 122 , 122 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 123 , 123 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 124 , 124 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 125 , 125 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 126 , 126 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 127 , 127 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 128 , 128 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 129 , 129 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 130 , 130 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 131 , 131 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 132 , 132 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 133 , 133 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 134 , 134 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 135 , 135 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 136 , 136 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 137 , 137 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 138 , 138 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 139 , 139 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 140 , 140 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 141 , 141 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 142 , 142 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 143 , 143 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 144 , 144 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 145 , 145 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 146 , 146 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 147 , 147 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 148 , 148 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 149 , 149 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 150 , 150 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 151 , 151 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 152 , 152 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 153 , 153 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 154 , 154 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 155 , 155 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 156 , 156 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 157 , 157 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 158 , 158 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 159 , 159 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 160 , 160 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 161 , 161 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 162 , 162 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 163 , 163 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 164 , 164 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 165 , 165 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 166 , 166 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 167 , 167 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 168 , 168 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 169 , 169 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 170 , 170 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 171 , 171 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 172 , 172 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 173 , 173 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 174 , 174 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 175 , 175 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 176 , 176 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 177 , 177 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 178 , 178 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 179 , 179 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 180 , 180 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 181 , 181 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 182 , 182 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 183 , 183 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 184 , 184 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 185 , 185 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 186 , 186 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 187 , 187 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 188 , 188 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 189 , 189 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 190 , 190 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 191 , 191 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 192 , 192 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 193 , 193 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 194 , 194 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 195 , 195 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 196 , 196 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 197 , 197 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 198 , 198 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 199 , 199 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 200 , 200 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 201 , 201 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 202 , 202 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 203 , 203 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 204 , 204 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 205 , 205 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 206 , 206 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 207 , 207 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 208 , 208 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 209 , 209 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 210 , 210 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 211 , 211 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 212 , 212 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 213 , 213 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 214 , 214 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 215 , 215 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 216 , 216 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 217 , 217 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 218 , 218 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 219 , 219 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 220 , 220 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 221 , 221 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 222 , 222 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 223 , 223 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 224 , 224 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 225 , 225 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 226 , 226 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 227 , 227 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 228 , 228 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 229 , 229 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 230 , 230 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 231 , 231 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 232 , 232 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 233 , 233 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 234 , 234 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 235 , 235 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 236 , 236 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 237 , 237 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 238 , 238 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 239 , 239 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 240 , 240 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 241 , 241 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 242 , 242 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 243 , 243 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 244 , 244 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 245 , 245 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 246 , 246 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 247 , 247 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 248 , 248 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 249 , 249 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 250 , 250 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 251 , 251 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 252 , 252 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 253 , 253 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 254 , 254 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 255 , 255 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 256 , 256 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 257 , 257 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 258 , 258 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 259 , 259 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 260 , 260 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 261 , 261 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 262 , 262 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 263 , 263 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 264 , 264 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 265 , 265 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 266 , 266 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 267 , 267 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 268 , 268 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 269 , 269 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 270 , 270 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 271 , 271 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 272 , 272 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 273 , 273 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 274 , 274 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 275 , 275 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 276 , 276 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 277 , 277 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 278 , 278 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 279 , 279 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 280 , 280 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 281 , 281 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 282 , 282 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 283 , 283 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 284 , 284 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 285 , 285 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 286 , 286 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 287 , 287 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 288 , 288 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 289 , 289 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 290 , 290 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 291 , 291 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 292 , 292 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 293 , 293 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 294 , 294 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 295 , 295 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 296 , 296 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 297 , 297 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 298 , 298 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 299 , 299 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 300 , 300 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 301 , 301 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 302 , 302 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 303 , 303 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 304 , 304 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 305 , 305 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 306 , 306 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 307 , 307 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 308 , 308 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 309 , 309 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 310 , 310 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 311 , 311 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 312 , 312 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 313 , 313 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 314 , 314 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 315 , 315 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 316 , 316 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 317 , 317 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 318 , 318 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 319 , 319 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 320 , 320 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 321 , 321 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 322 , 322 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 323 , 323 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 324 , 324 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 325 , 325 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 326 , 326 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 327 , 327 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 328 , 328 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 329 , 329 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 330 , 330 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 331 , 331 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 332 , 332 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 333 , 333 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 334 , 334 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 335 , 335 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 336 , 336 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 337 , 337 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 338 , 338 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 339 , 339 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 340 , 340 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 341 , 341 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 342 , 342 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 343 , 343 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 344 , 344 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 345 , 345 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 346 , 346 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 347 , 347 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 348 , 348 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 349 , 349 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 350 , 350 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 351 , 351 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 352 , 352 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 353 , 353 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 354 , 354 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 355 , 355 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 356 , 356 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 357 , 357 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 358 , 358 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 359 , 359 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 360 , 360 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 361 , 361 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 362 , 362 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 363 , 363 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 364 , 364 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 365 , 365 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 366 , 366 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 367 , 367 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 368 , 368 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 369 , 369 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 370 , 370 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 371 , 371 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 372 , 372 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 373 , 373 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 374 , 374 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 375 , 375 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 376 , 376 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 377 , 377 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 378 , 378 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 379 , 379 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 380 , 380 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 381 , 381 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 382 , 382 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 383 , 383 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 384 , 384 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 385 , 385 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 386 , 386 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 387 , 387 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 388 , 388 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 389 , 389 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 390 , 390 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 391 , 391 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 392 , 392 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 393 , 393 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 394 , 394 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 395 , 395 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 396 , 396 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 397 , 397 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 398 , 398 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 399 , 399 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 400 , 400 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 401 , 401 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 402 , 402 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 403 , 403 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 404 , 404 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 405 , 405 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 406 , 406 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 407 , 407 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 408 , 408 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 409 , 409 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 410 , 410 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 411 , 411 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 412 , 412 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 413 , 413 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 414 , 414 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 415 , 415 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 416 , 416 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 417 , 417 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 418 , 418 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 419 , 419 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 420 , 420 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 421 , 421 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 422 , 422 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 423 , 423 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 424 , 424 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 425 , 425 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 426 , 426 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 427 , 427 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 428 , 428 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 429 , 429 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 430 , 430 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 431 , 431 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 432 , 432 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 433 , 433 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 434 , 434 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 435 , 435 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 436 , 436 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 437 , 437 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 438 , 438 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 439 , 439 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 440 , 440 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 441 , 441 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 442 , 442 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 443 , 443 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 444 , 444 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 445 , 445 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 446 , 446 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 447 , 447 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 448 , 448 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 449 , 449 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 450 , 450 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 451 , 451 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 452 , 452 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 453 , 453 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 454 , 454 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 455 , 455 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 456 , 456 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 457 , 457 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 458 , 458 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 459 , 459 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 460 , 460 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 461 , 461 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 462 , 462 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 463 , 463 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 464 , 464 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 465 , 465 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 466 , 466 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 467 , 467 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 468 , 468 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 469 , 469 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 470 , 470 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 471 , 471 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 472 , 472 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 473 , 473 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 474 , 474 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 475 , 475 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 476 , 476 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 477 , 477 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 478 , 478 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 479 , 479 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 480 , 480 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 481 , 481 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 482 , 482 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 483 , 483 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 484 , 484 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 485 , 485 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 486 , 486 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 487 , 487 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 488 , 488 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 489 , 489 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 490 , 490 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 491 , 491 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 492 , 492 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 493 , 493 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 494 , 494 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 495 , 495 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 496 , 496 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 497 , 497 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 498 , 498 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 499 , 499 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 500 , 500 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 501 , 501 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 502 , 502 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 503 , 503 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 504 , 504 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 505 , 505 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 506 , 506 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 507 , 507 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 508 , 508 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 509 , 509 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 510 , 510 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 511 , 511 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 512 , 512 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 513 , 513 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 514 , 514 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 515 , 515 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 516 , 516 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 517 , 517 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 518 , 518 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 519 , 519 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 520 , 520 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 521 , 521 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 522 , 522 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 523 , 523 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 524 , 524 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 525 , 525 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 526 , 526 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 527 , 527 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 528 , 528 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 529 , 529 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 530 , 530 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 531 , 531 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 532 , 532 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 533 , 533 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 534 , 534 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 535 , 535 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 536 , 536 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 537 , 537 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 538 , 538 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 539 , 539 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 540 , 540 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 541 , 541 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 542 , 542 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 543 , 543 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 544 , 544 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 545 , 545 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 546 , 546 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 547 , 547 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 548 , 548 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 549 , 549 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 550 , 550 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 551 , 551 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 552 , 552 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 553 , 553 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 554 , 554 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 555 , 555 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 556 , 556 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 557 , 557 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 558 , 558 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 559 , 559 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 560 , 560 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 561 , 561 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 562 , 562 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 563 , 563 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 564 , 564 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 565 , 565 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 566 , 566 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 567 , 567 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 568 , 568 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 569 , 569 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 570 , 570 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 571 , 571 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 572 , 572 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 573 , 573 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 574 , 574 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 575 , 575 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 576 , 576 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 577 , 577 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 578 , 578 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 579 , 579 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 580 , 580 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 581 , 581 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 582 , 582 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 583 , 583 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 584 , 584 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 585 , 585 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 586 , 586 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 587 , 587 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 588 , 588 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 589 , 589 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 590 , 590 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 591 , 591 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 592 , 592 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 593 , 593 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 594 , 594 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 595 , 595 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 596 , 596 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 597 , 597 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 598 , 598 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 599 , 599 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 600 , 600 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 601 , 601 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 602 , 602 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 603 , 603 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 604 , 604 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 605 , 605 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 606 , 606 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 607 , 607 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 608 , 608 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 609 , 609 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 610 , 610 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 611 , 611 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 612 , 612 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 613 , 613 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 614 , 614 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 615 , 615 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 616 , 616 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 617 , 617 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 618 , 618 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 619 , 619 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 620 , 620 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 621 , 621 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 622 , 622 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 623 , 623 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 624 , 624 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 625 , 625 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 626 , 626 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 627 , 627 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 628 , 628 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 629 , 629 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 630 , 630 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 631 , 631 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 632 , 632 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 633 , 633 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 634 , 634 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 635 , 635 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 636 , 636 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 637 , 637 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 638 , 638 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 639 , 639 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 640 , 640 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 641 , 641 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 642 , 642 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 643 , 643 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 644 , 644 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 645 , 645 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 646 , 646 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 647 , 647 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 648 , 648 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 649 , 649 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 650 , 650 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 651 , 651 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 652 , 652 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 653 , 653 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 654 , 654 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 655 , 655 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 656 , 656 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 657 , 657 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 658 , 658 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 659 , 659 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 660 , 660 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 661 , 661 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 662 , 662 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 663 , 663 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 664 , 664 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 665 , 665 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 666 , 666 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 667 , 667 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 668 , 668 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 669 , 669 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 670 , 670 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 671 , 671 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 672 , 672 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 673 , 673 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 674 , 674 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 675 , 675 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 676 , 676 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 677 , 677 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 678 , 678 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 679 , 679 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 680 , 680 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 681 , 681 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 682 , 682 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 683 , 683 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 684 , 684 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 685 , 685 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 686 , 686 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 687 , 687 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 688 , 688 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 689 , 689 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 690 , 690 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 691 , 691 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 692 , 692 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 693 , 693 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 694 , 694 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 695 , 695 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 696 , 696 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 697 , 697 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 698 , 698 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 699 , 699 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 700 , 700 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 701 , 701 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 702 , 702 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 703 , 703 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 704 , 704 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 705 , 705 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 706 , 706 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 707 , 707 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 708 , 708 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 709 , 709 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 710 , 710 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 711 , 711 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 712 , 712 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 713 , 713 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 714 , 714 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 715 , 715 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 716 , 716 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 717 , 717 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 718 , 718 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 719 , 719 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 720 , 720 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 721 , 721 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 722 , 722 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 723 , 723 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 724 , 724 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 725 , 725 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 726 , 726 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 727 , 727 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 728 , 728 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 729 , 729 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 730 , 730 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 731 , 731 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 732 , 732 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 733 , 733 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 734 , 734 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 735 , 735 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 736 , 736 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 737 , 737 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 738 , 738 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 739 , 739 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 740 , 740 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 741 , 741 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 742 , 742 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 743 , 743 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 744 , 744 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 745 , 745 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 746 , 746 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 747 , 747 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 748 , 748 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 749 , 749 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 750 , 750 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 751 , 751 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 752 , 752 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 753 , 753 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 754 , 754 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 755 , 755 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 756 , 756 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 757 , 757 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 758 , 758 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 759 , 759 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 760 , 760 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 761 , 761 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 762 , 762 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 763 , 763 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 764 , 764 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 765 , 765 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 766 , 766 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 767 , 767 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 768 , 768 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 769 , 769 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 770 , 770 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 771 , 771 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 772 , 772 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 773 , 773 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 774 , 774 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 775 , 775 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 776 , 776 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 777 , 777 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 778 , 778 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 779 , 779 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 780 , 780 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 781 , 781 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 782 , 782 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 783 , 783 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 784 , 784 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 785 , 785 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 786 , 786 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 787 , 787 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 788 , 788 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 789 , 789 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 790 , 790 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 791 , 791 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 792 , 792 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 793 , 793 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 794 , 794 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 795 , 795 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 796 , 796 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 797 , 797 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 798 , 798 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 799 , 799 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 800 , 800 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 801 , 801 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 802 , 802 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 803 , 803 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 804 , 804 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 805 , 805 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 806 , 806 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 807 , 807 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 808 , 808 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 809 , 809 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 810 , 810 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 811 , 811 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 812 , 812 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 813 , 813 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 814 , 814 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 815 , 815 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 816 , 816 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 817 , 817 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 818 , 818 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 819 , 819 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 820 , 820 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 821 , 821 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 822 , 822 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 823 , 823 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 824 , 824 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 825 , 825 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 826 , 826 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 827 , 827 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 828 , 828 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 829 , 829 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 830 , 830 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 831 , 831 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 832 , 832 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 833 , 833 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 834 , 834 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 835 , 835 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 836 , 836 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 837 , 837 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 838 , 838 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 839 , 839 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 840 , 840 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 841 , 841 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 842 , 842 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 843 , 843 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 844 , 844 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 845 , 845 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 846 , 846 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 847 , 847 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 848 , 848 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 849 , 849 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 850 , 850 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 851 , 851 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 852 , 852 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 853 , 853 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 854 , 854 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 855 , 855 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 856 , 856 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 857 , 857 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 858 , 858 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 859 , 859 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 860 , 860 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 861 , 861 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 862 , 862 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 863 , 863 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 864 , 864 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 865 , 865 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 866 , 866 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 867 , 867 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 868 , 868 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 869 , 869 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 870 , 870 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 871 , 871 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 872 , 872 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 873 , 873 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 874 , 874 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 875 , 875 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 876 , 876 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 877 , 877 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 878 , 878 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 879 , 879 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 880 , 880 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 881 , 881 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 882 , 882 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 883 , 883 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 884 , 884 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 885 , 885 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 886 , 886 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 887 , 887 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 888 , 888 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 889 , 889 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 890 , 890 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 891 , 891 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 892 , 892 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 893 , 893 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 894 , 894 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 895 , 895 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 896 , 896 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 897 , 897 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 898 , 898 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 899 , 899 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 900 , 900 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 901 , 901 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 902 , 902 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 903 , 903 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 904 , 904 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 905 , 905 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 906 , 906 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 907 , 907 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 908 , 908 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 909 , 909 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 910 , 910 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 911 , 911 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 912 , 912 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 913 , 913 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 914 , 914 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 915 , 915 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 916 , 916 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 917 , 917 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 918 , 918 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 919 , 919 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 920 , 920 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 921 , 921 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 922 , 922 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 923 , 923 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 924 , 924 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 925 , 925 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 926 , 926 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 927 , 927 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 928 , 928 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 929 , 929 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 930 , 930 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 931 , 931 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 932 , 932 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 933 , 933 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 934 , 934 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 935 , 935 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 936 , 936 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 937 , 937 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 938 , 938 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 939 , 939 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 940 , 940 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 941 , 941 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 942 , 942 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 943 , 943 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 944 , 944 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 945 , 945 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 946 , 946 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 947 , 947 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 948 , 948 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 949 , 949 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 950 , 950 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 951 , 951 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 952 , 952 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 953 , 953 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 954 , 954 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 955 , 955 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 956 , 956 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 957 , 957 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 958 , 958 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 959 , 959 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 960 , 960 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 961 , 961 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 962 , 962 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 963 , 963 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 964 , 964 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 965 , 965 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 966 , 966 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 967 , 967 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 968 , 968 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 969 , 969 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 970 , 970 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 971 , 971 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 972 , 972 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 973 , 973 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 974 , 974 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 975 , 975 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 976 , 976 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 977 , 977 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 978 , 978 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 979 , 979 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 980 , 980 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 981 , 981 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 982 , 982 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 983 , 983 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 984 , 984 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 985 , 985 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 986 , 986 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 987 , 987 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 988 , 988 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 989 , 989 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 990 , 990 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 991 , 991 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 992 , 992 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 993 , 993 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 994 , 994 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 995 , 995 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 996 , 996 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 997 , 997 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 998 , 998 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 999 , 999 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1000, 1000 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1001, 1001 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1002, 1002 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1003, 1003 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1004, 1004 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1005, 1005 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1006, 1006 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1007, 1007 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1008, 1008 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1009, 1009 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1010, 1010 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1011, 1011 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1012, 1012 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1013, 1013 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1014, 1014 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1015, 1015 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1016, 1016 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1017, 1017 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1018, 1018 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1019, 1019 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1020, 1020 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1021, 1021 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1022, 1022 * sizeof(uint32_t), sizeof(uint32_t) },
		{ 1023, 1023 * sizeof(uint32_t), sizeof(uint32_t) }
	};

	VkSpecializationInfo mVertexSpecializationInfo =
	{
		251,                                           // mapEntryCount
		mSlotMapEntries,							   // pMapEntries
		sizeof(SpecializationConstants),               // dataSize
		nullptr,// pData
	};

	VkSpecializationInfo mPixelSpecializationInfo =
	{
		251,                                           // mapEntryCount
		mSlotMapEntries,                               // pMapEntries
		sizeof(SpecializationConstants),               // dataSize
		nullptr,// pData
	};

	VkSampler mSampler = VK_NULL_HANDLE;
	VkImage mImage = VK_NULL_HANDLE;
	VkImageLayout mImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkDeviceMemory mDeviceMemory = VK_NULL_HANDLE;
	VkImageView mImageView = VK_NULL_HANDLE;
	int32_t mTextureWidth = 0;
	int32_t mTextureHeight = 0;

	int32_t mVertexCount = 0;

	VkBuffer mLightBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mLightBufferMemory = VK_NULL_HANDLE;
	VkBuffer mMaterialBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mMaterialBufferMemory = VK_NULL_HANDLE;


	boost::container::small_vector< std::shared_ptr<SamplerRequest>, 16> mSamplerRequests;
	boost::container::small_vector< std::shared_ptr<DrawContext>, 16> mDrawBuffer;

	VkDescriptorSet mLastDescriptorSet = VK_NULL_HANDLE;
	VkPipeline mLastVkPipeline = VK_NULL_HANDLE;

	Transformations mTransformations;

	float mEpsilon = std::numeric_limits<float>::epsilon();

	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;

	void BeginDraw(std::shared_ptr<DrawContext> context, std::shared_ptr<ResourceContext> resourceContext, D3DPRIMITIVETYPE type);
	void CreatePipe(std::shared_ptr<DrawContext> context);
	void CreateSampler(std::shared_ptr<SamplerRequest> request);

	void UpdatePushConstants(std::shared_ptr<DrawContext> context);
	void FlushDrawBufffer();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& deviceMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:

};

#endif // BUFFERMANAGER_H
