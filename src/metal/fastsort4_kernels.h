#pragma once
// Embedded so executables do not depend on the source tree or working directory.
static const char* fastsort4MetalSource = R"metal(
#include <metal_stdlib>
using namespace metal;
struct Params { uint count; uint width; };

kernel void block_sort(device const int* input [[buffer(0)]],
                       device int* output [[buffer(1)]],
                       constant Params& p [[buffer(2)]],
                       uint lane [[thread_index_in_threadgroup]],
                       uint group [[threadgroup_position_in_grid]]) {
    threadgroup int values[BLOCK_SIZE];
    uint index = group * BLOCK_SIZE + lane;
    values[lane] = index < p.count ? input[index] : 2147483647;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for(uint k = 2; k <= BLOCK_SIZE; k <<= 1) {
        for(uint j = k >> 1; j != 0; j >>= 1) {
            uint other = lane ^ j;
            if(other > lane) {
                int a = values[lane], b = values[other];
                bool ascending = (lane & k) == 0;
                values[lane] = ascending ? min(a, b) : max(a, b);
                values[other] = ascending ? max(a, b) : min(a, b);
            }
            threadgroup_barrier(mem_flags::mem_threadgroup);
        }
    }
    if(index < p.count) output[index] = values[lane];
}

kernel void merge_pass(device const int* input [[buffer(0)]],
                       device int* output [[buffer(1)]],
                       constant Params& p [[buffer(2)]],
                       uint tid [[thread_position_in_grid]]) {
    uint out = tid * MERGE_ITEMS;
    if(out >= p.count) return;
    uint span = p.width * 2;
    uint base = (out / span) * span;
    uint aSize = min(p.width, p.count - base);
    uint bSize = min(p.width, p.count - base - aSize);
    uint diagonal = out - base;
    // A wins ties. Every thread independently finds its disjoint output range.
    uint low = diagonal > bSize ? diagonal - bSize : 0;
    uint high = min(diagonal, aSize);
    while(low < high) {
        uint a = low + (high - low) / 2;
        uint b = diagonal - a;
        if(b > 0 && a < aSize && input[base + a] <= input[base + aSize + b - 1])
            low = a + 1;
        else high = a;
    }
    uint a = low, b = diagonal - low;
    uint end = min(uint(MERGE_ITEMS), aSize + bSize - diagonal);
    for(uint i = 0; i < end; ++i) {
        bool takeA = a < aSize && (b >= bSize || input[base + a] <= input[base + aSize + b]);
        output[out + i] = takeA ? input[base + a++] : input[base + aSize + b++];
    }
}
)metal";
