#ifndef INSTANCING
  $input v_worldPos, v_underwaterRainTimeDay
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <newb/main.sh>
  #include <newb/functions/cloudnoise.h>
  uniform vec4 TimeOfDay;
  uniform vec4 FogColor;
  uniform vec4 FogAndDistanceControl;
#endif

// sky clouds generator (cloud type 4), helpers are in functions/cloudnoise.h
#ifndef INSTANCING
  #ifdef NL_SKY_CLOUDS

  vec3 bsbeClouds(vec3 color, vec3 nPos, float fTime, vec3 horC, vec3 zenC, vec2 fragCoord) {
    vec2 cloudP = (nPos.xz / nPos.y) * NL_SKY_CLOUDS_SCALE;
    cloudP -= cloudP * bsbeHash(fragCoord.x + fragCoord.y * 57.0) * 0.05;

    float sDens = NL_SKY_CLOUDS_DENSITY;
    vec3 cloDirC = horC * NL_SKY_CLOUDS_BRIGHTNESS;
    vec3 cloAmbC = bsbeSatur(zenC, 0.7);

    float lodY = nPos.y / NL_SKY_CLOUDS_LOD_Y;
    int nOct = int(step(1.0, lodY) + step(1.58, lodY) + step(2.5, lodY) + step(3.95, lodY));
    if (nOct > 0) {
      for (int i = 0; i < NL_SKY_CLOUDS_LAYERS; i++) {
        float cloudM = bsbeFbm(cloudP, sDens, fTime, nOct);
        cloDirC = mix(cloDirC, cloAmbC, NL_SKY_CLOUDS_AMBIENT_MIX);
        color = mix(color, cloDirC, cloudM * smoothstep(0.0, NL_SKY_CLOUDS_HORIZON_FADE, nPos.y));
        sDens += (i <= 6) ? -0.1 : 0.1;
        cloudP -= cloudP * NL_SKY_CLOUDS_LAYER_SQUEEZE;
      }
    }
    return color;
  }

  #endif
#endif

void main() {
  #ifndef INSTANCING
    vec3 viewDir = normalize(v_worldPos);

    nl_environment env;
    env.end = false;
    env.nether = false;
    env.underwater = v_underwaterRainTimeDay.x > 0.5;
    env.rainFactor = v_underwaterRainTimeDay.y;
    env.dayFactor = v_underwaterRainTimeDay.w;
    env.fogCol = FogColor.rgb;
    env = calculateSunParams(env, TimeOfDay.x);

    nl_skycolor skycol = nlOverworldSkyColors(env);

    vec3 skyColor = nlRenderSky(skycol, env, -viewDir, v_underwaterRainTimeDay.z, true);

    #ifdef NL_SKY_CLOUDS
      if (!env.underwater && env.rainFactor < 0.99) {
        // viewDir points away from the sky here (nlRenderSky gets -viewDir), so pass -viewDir
        vec3 withClouds = bsbeClouds(skyColor, -viewDir, v_underwaterRainTimeDay.z, skycol.horizon, skycol.zenith, gl_FragCoord.xy);
        skyColor = mix(withClouds, skyColor, env.rainFactor);
      }
    #endif

    skyColor = colorCorrection(skyColor);

    gl_FragColor = vec4(skyColor, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
