#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_FOUNDATION_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    // SDL initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create an SDL window
    SDL_Window *window = SDL_CreateWindow("Metal + SDL2 Triangle",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_MetalView metalView = SDL_Metal_CreateView(window);

    // Metal setup
    MTL::Device *device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Failed to create Metal device" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    MTL::CommandQueue *commandQueue = device->newCommandQueue();
    // Load the compiled metallib file
    NS::Error *error = nullptr;
    MTL::Library *library = device->newLibrary(NS::String::string("shaders.metallib", NS::ASCIIStringEncoding), &error);
    if (!library) {
        std::cerr << "Failed to load Metal library: " << error->localizedDescription()->utf8String() << std::endl;
        return -1;
    }

    MTL::Function *vertexFunction = library->newFunction(NS::String::string("vertex_main", NS::ASCIIStringEncoding));
    MTL::Function *fragmentFunction = library->
            newFunction(NS::String::string("fragment_main", NS::ASCIIStringEncoding));

    MTL::RenderPipelineDescriptor *pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexFunction);
    pipelineDescriptor->setFragmentFunction(fragmentFunction);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);

    NS::Error* error2 = nullptr;
    MTL::RenderPipelineState *pipelineState = device->newRenderPipelineState(pipelineDescriptor, &error2);
    if (!pipelineState) {
        std::cerr << "Failed to create pipeline state: " << error2->localizedDescription()->utf8String() << std::endl;
        return -1;
    }

    vertexFunction->release();
    fragmentFunction->release();
    pipelineDescriptor->release();
    library->release();

    CA::MetalLayer *metalLayer = (CA::MetalLayer *) SDL_Metal_GetLayer(metalView);
    metalLayer->setDevice(device);
    metalLayer->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
    metalLayer->setFramebufferOnly(true);

    // Main loop
    bool isRunning = true;
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        MTL::Drawable *mtlDrawable = metalLayer->nextDrawable();
        if (!mtlDrawable) continue;

        const auto *drawable = (CA::MetalDrawable *)(mtlDrawable);
        MTL::RenderPassDescriptor *passDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
        passDescriptor->colorAttachments()->object(0)->setTexture(((MTL::Texture *)drawable->texture()));
        passDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadAction::LoadActionClear);
        passDescriptor->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));
        passDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreAction::StoreActionStore);

        MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
        MTL::RenderCommandEncoder *encoder = commandBuffer->renderCommandEncoder(passDescriptor);
        encoder->setRenderPipelineState(pipelineState);
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, 
                              NS::UInteger(0), 
                              NS::UInteger(3));
        encoder->endEncoding();

        commandBuffer->presentDrawable(drawable);
        commandBuffer->commit();

        passDescriptor->release();
    }

    // Cleanup
    metalLayer->release();
    pipelineState->release();
    commandQueue->release();
    device->release();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
