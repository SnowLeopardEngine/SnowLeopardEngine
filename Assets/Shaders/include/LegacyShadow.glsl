float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // remap to [0, 1] to fit depth

    
    if(projCoords.z > 1.0) return 1.0;

    float lightSize = 0.1; 
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); 
    float searchRadius = 0.005; 
    float shadow = 0.0; 

    
    float blockerDepth = 0.0;
    int countBlockers = 0;
    for(float x = -searchRadius; x <= searchRadius; x += texelSize.x) 
    {
        for(float y = -searchRadius; y <= searchRadius; y += texelSize.y) 
        {
            float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y)).r;
            if(sampleDepth < projCoords.z) 
            {
                blockerDepth += sampleDepth;
                countBlockers++;
            }
        }
    }
    
    if(countBlockers == 0) return 0.0;

    blockerDepth /= float(countBlockers);

    float shadowRadius = lightSize / (projCoords.z - blockerDepth);
    shadowRadius = clamp(shadowRadius, 0.0, 5 * texelSize.x); 

    for(float x = -shadowRadius; x <= shadowRadius; x += texelSize.x) 
    {
        for(float y = -shadowRadius; y <= shadowRadius; y += texelSize.y) 
        {
            float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y)).r;
            shadow += (projCoords.z > sampleDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= (2.0 * shadowRadius / texelSize.x) * (2.0 * shadowRadius / texelSize.y); 

    return shadow;
}