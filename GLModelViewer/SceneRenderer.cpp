//
//  SceneRenderer.cpp
//  ModelViewer
//
//  Created by Mattias Bergström on 2014-01-28.
//  Copyright (c) 2014 Mattias Bergström. All rights reserved.
//

#include <iostream>
#include "SceneRenderer.h"
#include "BlurShader.h"
#include <GLFW/glfw3.h>

void SceneRenderer::init(int width, int height) {
    depthPass.init(width, height);
    
    
    shadowPass.init(width, height);
    
    // Init lighting
    auto buffer = depthPass.getBuffer();
    lightingPass.init(width, height, buffer->getDepthAttachment());
    lightingPass.shouldWriteDepth = false;
    
    // Init geometry pass
    geometryPass.init(width, height);
    
    // Init deferred lighting
    buffer = geometryPass.getBuffer();
    deferredLightingPass.init(width, height, buffer->getDepthAttachment());
    
    passthrough.init();
    
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(UnitQuad::CreateUnitQuad());
    screenNode.init(mesh);
    
    this->width = width;
    this->height = height;
}

void SceneRenderer::updateResolution(int width, int height) {
    // Update everything that needs to be updated when resolution changes
    
    depthPass.resize(width, height);
    geometryPass.resize(width, height);
    
    shadowPass.resize(width, height);
    
    auto buffer = geometryPass.getBuffer();
    lightingPass.resize(width, height, buffer->getDepthAttachment());
    
    buffer = geometryPass.getBuffer();
    deferredLightingPass.resize(width, height, buffer->getDepthAttachment());
    
    this->width = width;
    this->height = height;
}

void SceneRenderer::renderScene() {
    
    RenderPass* pass;
    
    shadowPass.render(proj, view, nodes, lights);
    
    if(!isDeferred) {
        // Do a depth pass
        depthPass.render(proj, view, nodes);
        
        lightingPass.render(proj, view, nodes, lights);
        
        pass = &lightingPass;
    } else {
        // Deferred lighting
        geometryPass.render(proj, view, nodes);
        deferredLightingPass.render(proj, view, (GBuffer*)geometryPass.getBuffer(), lights);
        
        pass = &deferredLightingPass;
    }
    // Draw final pass to screen    
    pass->bindBufferTextures();
    
    passthrough.use();
    
    glDepthMask(GL_FALSE);
    
    passthrough.setUniforms(proj, view, screenNode.modelMatrix);
    
    screenNode.render();
    
    glDepthMask(GL_TRUE);
    
    pass->unbindBufferTextures();
}

