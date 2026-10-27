#ifndef INSTANCING
  $input v_worldPos, v_underwaterRainTimeDay
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <newb/main.sh>
  #include <newb/functions/skyclouds.h>
  #include <newb/functions/skyaurora.h>
  uniform vec4 TimeOfDay;
  uniform vec4 FogColor;
  uniform vec4 FogAndDistanceControl;
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

    #if NL_CLOUD_TYPE == 4
      // bicubic clouds painted on the sky dome (the Clouds material draws nothing for this type)
      if (!env.underwater && env.rainFactor < 0.99) {
        vec3 withClouds = bsbeClouds(skyColor, viewDir, v_underwaterRainTimeDay.z, skycol.horizon, skycol.zenith, gl_FragCoord.xy);
        skyColor = mix(withClouds, skyColor, env.rainFactor);
      }
    #endif

    #ifdef NL_AURORA_TEX
      // aurora made only from AuroraTexture (night only, hidden by rain and underwater)
      if (!env.underwater) {
        skyColor = nlAuroraTex(skyColor, viewDir, v_underwaterRainTimeDay.z, env.dayFactor, env.rainFactor);
      }
    #endif

    skyColor = colorCorrection(skyColor);

    gl_FragColor = vec4(skyColor, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
