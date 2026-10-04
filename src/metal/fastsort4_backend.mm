#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "fastsort4_backend.h"
#include "fastsort4_kernels.h"
#include "../fastsort4.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>

namespace fastsort4Detail {
namespace {
std::string describe(NSError* error){
	return error ? std::string(error.localizedDescription.UTF8String) : "unknown Metal error";
}
struct Context {
	id<MTLDevice> device;
	id<MTLCommandQueue> queue;
	id<MTLComputePipelineState> block, merge;
	std::string error;
	Context(){
		device = MTLCreateSystemDefaultDevice();
		if(!device){ error = "no Metal device"; return; }
		queue = [device newCommandQueue];
		if(!queue){ error = "cannot create Metal command queue"; return; }
		MTLCompileOptions* options = [MTLCompileOptions new];
		options.preprocessorMacros = @{@"BLOCK_SIZE": @(FastSort4::GetBlockSize()),
			@"MERGE_ITEMS": @(FastSort4::GetMergeItems())};
		NSError* failure = nil;
		id<MTLLibrary> library = [device newLibraryWithSource:[NSString stringWithUTF8String:fastsort4MetalSource]
			options:options error:&failure];
		if(!library){ error = describe(failure); return; }
		block = [device newComputePipelineStateWithFunction:[library newFunctionWithName:@"block_sort"] error:&failure];
		if(!block){ error = describe(failure); return; }
		merge = [device newComputePipelineStateWithFunction:[library newFunctionWithName:@"merge_pass"] error:&failure];
		if(!merge){ error = describe(failure); return; }
		if(block.maxTotalThreadsPerThreadgroup < static_cast<NSUInteger>(FastSort4::GetBlockSize()))
			error = "unsupported Metal threadgroup size";
	}
};
struct Params { std::uint32_t count, width; };
}

bool metalSort(const std::vector<int>& input, std::vector<int>& output, std::string& error){
	static_assert(sizeof(int) == 4, "Metal sort requires 32-bit int");
	@autoreleasepool {
		static Context context;
		if(!context.error.empty()){ error = context.error; return false; }
		// Keep width*2 and rounded dispatch indices inside uint32_t.
		if(input.size() > (std::size_t(1) << 30) || input.size() > context.device.maxBufferLength / sizeof(int)){
			error = "input exceeds Metal buffer/index limits"; return false;
		}
		if(input.empty()){ output.clear(); return true; }
		std::size_t bytes = input.size() * sizeof(int);
		id<MTLBuffer> first = [context.device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
		id<MTLBuffer> second = [context.device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
		if(!first || !second){ error = "cannot allocate Metal buffers"; return false; }
		std::memcpy(first.contents, input.data(), bytes);
		id<MTLCommandBuffer> command = [context.queue commandBuffer];
		if(!command){ error = "cannot create Metal command buffer"; return false; }
		Params params = {static_cast<std::uint32_t>(input.size()), 0};
		auto encode = [&](id<MTLComputePipelineState> pipeline, NSUInteger threads, NSUInteger groupSize){
			id<MTLComputeCommandEncoder> encoder = [command computeCommandEncoder];
			if(!encoder) return false;
			[encoder setComputePipelineState:pipeline];
			[encoder setBuffer:first offset:0 atIndex:0];
			[encoder setBuffer:second offset:0 atIndex:1];
			[encoder setBytes:&params length:sizeof(params) atIndex:2];
			[encoder dispatchThreadgroups:MTLSizeMake((threads + groupSize - 1) / groupSize, 1, 1)
				threadsPerThreadgroup:MTLSizeMake(groupSize, 1, 1)];
			[encoder endEncoding];
			id<MTLBuffer> saved = first; first = second; second = saved;
			return true;
		};
		if(!encode(context.block, input.size(), FastSort4::GetBlockSize())){
			error = "cannot create Metal block encoder"; return false;
		}
		for(std::size_t width = FastSort4::GetBlockSize(); width < input.size(); width *= 2){
			params.width = static_cast<std::uint32_t>(width);
			NSUInteger threads = (input.size() + FastSort4::GetMergeItems() - 1) / FastSort4::GetMergeItems();
			NSUInteger group = std::min<NSUInteger>(256, context.merge.maxTotalThreadsPerThreadgroup);
			if(!encode(context.merge, threads, group)){ error = "cannot create Metal merge encoder"; return false; }
		}
		[command commit];
		[command waitUntilCompleted];
		if(command.status != MTLCommandBufferStatusCompleted){ error = describe(command.error); return false; }
		try { output.resize(input.size()); }
		catch(const std::bad_alloc&){ error = "cannot allocate output vector"; return false; }
		std::memcpy(output.data(), first.contents, bytes);
		return true;
	}
}
}
