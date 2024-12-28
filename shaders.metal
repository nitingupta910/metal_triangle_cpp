#include <metal_stdlib>
using namespace metal;

struct VertexOutput {
    float4 position [[position]];
    float3 color;
};

vertex VertexOutput vertex_main(uint vertex_id [[vertex_id]]) {
    float3 positions[3] = {
        float3(0.0,  1.0, 0.0),
        float3(-1.0, -1.0, 0.0),
        float3(1.0,  -1.0, 0.0)
    };

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };

    VertexOutput out;
    out.position = float4(positions[vertex_id], 1.0);
    out.color = colors[vertex_id];
    return out;
}

fragment float4 fragment_main(VertexOutput in [[stage_in]]) {
    return float4(in.color, 1.0);
}