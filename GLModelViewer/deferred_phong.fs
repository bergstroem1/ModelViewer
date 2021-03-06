#version 150

#define M_PI 3.1415926535897932384626433832795

vec2 poissonDisk[16] = vec2[](
                              vec2( -0.94201624, -0.39906216 ),
                              vec2( 0.94558609, -0.76890725 ),
                              vec2( -0.094184101, -0.92938870 ),
                              vec2( 0.34495938, 0.29387760 ),
                              vec2( -0.91588581, 0.45771432 ),
                              vec2( -0.81544232, -0.87912464 ),
                              vec2( -0.38277543, 0.27676845 ),
                              vec2( 0.97484398, 0.75648379 ),
                              vec2( 0.44323325, -0.97511554 ),
                              vec2( 0.53742981, -0.47373420 ),
                              vec2( -0.26496911, -0.41893023 ), 
                              vec2( 0.79197514, 0.19090188 ), 
                              vec2( -0.24188840, 0.99706507 ), 
                              vec2( -0.81409955, 0.91437590 ), 
                              vec2( 0.19984126, 0.78641367 ), 
                              vec2( 0.14383161, -0.14100790 ) 
                              );

in vec2 uv;

layout (std140) uniform Light {
    vec4 position;
    vec4 intensity;
    vec4 direction;
    float angle;
    float spotExponent;
    float constantAtt;
    float linearAtt;
    float exponentialAtt;
} LightIn;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 mvp;
uniform mat3 normal_matrix;
uniform mat4 light_mvp;
uniform mat4 inverse_proj;
uniform mat4 inverse_view;

uniform sampler2D shadow_sampler;

uniform sampler2D normal_sampler;
uniform sampler2D depth_sampler;

//Material
uniform sampler2D diffuse_sampler;
uniform sampler2D ambient_sampler;
uniform sampler2D specular_sampler;
uniform sampler2D shininess_sampler;

out vec4 outColor;

float attenuation(vec3 dir){
    float dist = length(dir);
    float radiance = 1.0/(1.0+pow(dist/10.0, 2.0));
    return clamp(radiance*10.0, 0.0, 1.0);
}

void main() {
    vec4 color = vec4(0.0);
    
    vec3 normal = normalize(texture(normal_sampler, uv).xyz);
    float depth = texture(depth_sampler, uv).x * 2.0 - 1.0;
    
    
    // If depht == 1 there is nothing there. No need to bother with calculations
    if(depth == 1.0) {
        discard;
    }
    
    // Gamma correction
    vec4 ambient = pow(texture(ambient_sampler, uv), vec4(2.2));
    
    // Restore eye space position
    vec3 viewSpace;
    viewSpace.xy = uv * 2.0 -1.0; // uv coords are in [0-1], we need [-1, 1] for screen coords
    viewSpace.z=depth;
    vec4 worldSpace = inverse_proj * vec4(viewSpace,1.0);
    
    // Retrieve coordinates from light perspective
    vec4 projectedPosLightPov = (light_mvp * inverse_view * worldSpace);
    vec3 projectedPosLightPovNormal = projectedPosLightPov.xyz/projectedPosLightPov.w;
    vec3 shadow_coords = projectedPosLightPovNormal;
    
    // This 'normalization' is done after using it for light perspective retrival
    worldSpace.xyz/=worldSpace.w;
    
    // Specular Lighting
    vec4 spec = vec4(0.0);
    
    // Calculate light direction
    vec3 l = ((view * LightIn.position) - worldSpace).xyz;
    
    float lightDistance = length(l);
    l = normalize(l);
    
    float intensity = max(dot(l, normal), 0.0);
    if(intensity > 0.0f) {
        // Depth test for shadow
        float visibility = 1.0;
        float bias = 0.001;
        
        for (int i=0;i<16;i++) {
            if ( texture( shadow_sampler, shadow_coords.xy + poissonDisk[i]/400.0).x  <  shadow_coords.z - bias) {
                visibility -= 0.05;
            }
        }
        
        vec4 viewSpaceLightDir = view * LightIn.direction;
        float spotEffect = dot(normalize(viewSpaceLightDir.xyz), normalize(-l));
        float spotCutOff = cos(LightIn.angle*M_PI/180);
        
        if(spotEffect > spotCutOff) {
            
            // Retrieve material from textures
            vec4 specular = texture(specular_sampler, uv);
            float shininess = texture(shininess_sampler, uv).x;
            
            // Gamma correction
            vec4 diffuse = pow(texture(diffuse_sampler, uv), vec4(2.2));
            float lightIntensity = pow(LightIn.intensity.w, 2.2);
            
            spotEffect = pow(spotEffect, LightIn.spotExponent);
            float att = spotEffect / (LightIn.constantAtt + LightIn.linearAtt*lightDistance + LightIn.exponentialAtt*lightDistance*lightDistance);
            
            color += diffuse * intensity * att * LightIn.intensity * visibility * lightIntensity;
            
            vec3 e = normalize(- vec3(worldSpace));
            vec3 h = normalize(l + e);
        
            float intSpec = max(dot(h, normal), 0.0);
            color += specular * pow(intSpec, shininess) * att * LightIn.intensity * visibility * lightIntensity;
        }
    }
    
    outColor = max(color, ambient);
    outColor.a = 1.0;
}

