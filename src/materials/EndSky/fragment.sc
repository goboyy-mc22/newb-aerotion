#ifndef INSTANCING
$input v_texcoord0, v_posTime, v_dir
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <newb/main.sh>

  SAMPLER2D_AUTOREG(s_SkyTexture);
  uniform vec4 CameraPosition;
#endif

void main() {
  #ifndef INSTANCING
    vec4 diffuse = texture2D(s_SkyTexture, v_texcoord0);
    vec3 viewDir = normalize(v_posTime.xyz);

    vec3 color = renderEndSky(getEndHorizonCol(), getEndZenithCol(), normalize(v_posTime.xyz), v_posTime.w);
    color += endstreaks(getEndHorizonCol(), getEndZenithCol(), viewDir, v_posTime.w);
    #ifdef NL_BLACKHOLE
      vec4 bh = renderBlackhole(viewDir, v_posTime.w);
      color *= bh.a;
      color += bh.rgb;
    #endif

    color += 2.8*diffuse.rgb; // stars

    #ifdef NL_END_STORM
      // storm wall around the main island (world-space ring, ray goes to infinity)
      float stormDither = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
      vec4 storm = nlEndStorm(CameraPosition.xyz, normalize(v_dir), 1500.0, v_posTime.w, stormDither);
      color = color * storm.a + storm.rgb;
    #endif

    color = colorCorrection(color);

    gl_FragColor = vec4(color, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
