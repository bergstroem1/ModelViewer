//
//  LightPass.h
//  GLModelViewer
//
//  Created by Mattias Bergström on 2014-02-01.
//  Copyright (c) 2014 Mattias Bergström. All rights reserved.
//

#ifndef GLModelViewer_LightingPass_h
#define GLModelViewer_LightingPass_h

#include <memory>
#include "RenderPass.h"
#include "PhongShader.h"
#include "DeferredPhongShader.h"
#include "SceneNode.h"
#include "ColorBuffer.h"
#include "GBuffer.h"
#include "Light.h"

// TODO: Implementera vanlig forward phong lighting först, sen deferred

/*
 * Lights a scene from a geometry buffer (deferred lighting) or lights a vector of scene nodes.
 */
class LightingPass : public RenderPass {
    PhongShader phong;
    ColorBuffer* resultBuffer;
    int width;
    int height;
    
public:
    bool shouldWriteDepth = true;
    
    void init(int width, int height);
    void init(int width, int height, std::shared_ptr<DepthAttachment> depthTexture);
    void resize(int width, int height);
    void resize(int width, int height, std::shared_ptr<DepthAttachment> depthTexture);
    void render(glm::mat4 proj, glm::mat4 view, std::vector<std::shared_ptr<SceneNode>> nodes, std::vector<std::shared_ptr<Light>> lights);
    FrameBuffer* getBuffer();
    void bindBufferTextures();
    void unbindBufferTextures();
};

#endif
