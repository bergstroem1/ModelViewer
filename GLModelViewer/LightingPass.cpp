//
//  LightingPass.cpp
//  GLModelViewer
//
//  Created by Mattias Bergström on 2014-02-01.
//  Copyright (c) 2014 Mattias Bergström. All rights reserved.
//

#include "LightingPass.h"
#include "UnitQuad.h"
#include <iostream>

void LightingPass::init(int width, int height) {
    std::shared_ptr<DepthAttachment> depthTexture(new DepthAttachment);
    depthTexture->init(width, height);
    
    init(width, height, depthTexture);
}

void LightingPass::init(int width, int height, std::shared_ptr<DepthAttachment> depthTexture) {
    resultBuffer = new ColorBuffer();
    resultBuffer->init(width, height, depthTexture);
    
    phong.init();
    
    this->width = width;
    this->height = height;
}

void LightingPass::resize(int width, int height) {
    std::shared_ptr<DepthAttachment> depthTexture(new DepthAttachment);
    depthTexture->init(width, height);
    
    resize(width, height, depthTexture);
}

void LightingPass::resize(int width, int height, std::shared_ptr<DepthAttachment> depthTexture) {
    
    delete resultBuffer;
    resultBuffer = new ColorBuffer();
    resultBuffer->init(width, height, depthTexture);
    
    this->width = width;
    this->height = height;
}

void LightingPass::render(glm::mat4 proj, glm::mat4 view, std::vector<std::shared_ptr<SceneNode>> nodes, std::vector<std::shared_ptr<Light>> lights) {
    
    resultBuffer->bind();
    
    // Disable depth write
    if(!shouldWriteDepth) {
        glDepthMask(GL_FALSE);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    lights[0]->shadowTexture->bind();
    
    phong.use();
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        std::shared_ptr<SceneNode> node = (*it);
        
        phong.setUniforms(proj, view, node->modelMatrix);
        phong.setMaterial(node->mesh->material);
        
        // Must send the light mvp to the shader
        
        glm::vec3 position = glm::vec3(lights[0]->properties.position);
        glm::vec3 lightDir = glm::vec3(lights[0]->properties.direction);
        
        glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        
        glm::mat4 depthProjectionMatrix = glm::perspective(lights[0]->properties.angle * 2, width/(float)height, 0.1f, 50.0f);
        glm::mat4 depthViewMatrix = glm::lookAt(position, position + lightDir, glm::vec3(0,1,0));
        phong.setLightMvp(biasMatrix * depthProjectionMatrix * depthViewMatrix * node->modelMatrix);
        
        for(auto it = lights.begin(); it != lights.end(); it++) {
            phong.setLight((*it)->properties);
            node->render();
        }
    }
    
    lights[0]->shadowTexture->unbind();
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    resultBuffer->unbind();
}


FrameBuffer* LightingPass::getBuffer() {
    return resultBuffer;
}

void LightingPass::bindBufferTextures() {
    this->resultBuffer->bindAttachments();
}

void LightingPass::unbindBufferTextures() {
    this->resultBuffer->unbindAttachments();
}