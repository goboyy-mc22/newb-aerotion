#ifndef SKYCLOUDS_H
#define SKYCLOUDS_H

// Cloud type 4 (bicubic): clouds are painted directly on the sky dome.
// Only included by Sky/fragment.sc, so the noise sampler is declared in the Sky material only.

#ifndef INSTANCING
  #if NL_CLOUD_TYPE == 4

  float bsbeHash(float h) {
    return fract(sin(h) * 43758.5453);
  }

  #ifdef NL_CLOUD4_NOISE_TEX
    // buffer: src/materials/Sky/buffers/CloudsTexture.json (sampler name = s_ + buffer name)
    SAMPLER2D_AUTOREG(s_CloudsTexture);

    float bsbeVoronoi(vec2 pos) {
      return texture2DLod(s_CloudsTexture, pos * (1.0 / NL_CLOUD4_NOISE_CELLS), 0.0).r;
    }
  #else
    float bsbeVoronoi(vec2 pos) {
      vec2 p = floor(pos);
      vec2 f = fract(pos);
      float dist = 1.0;
      for (float y = -1.0; y <= 1.0; y++) {
        for (float x = -1.0; x <= 1.0; x++) {
          vec2 ne = vec2(x, y);
          vec2 pn = p + ne;
          float n = pn.x + pn.y * 57.0;
          dist = min(dist, length(ne + bsbeHash(n) - f));
        }
      }
      return dist;
    }
  #endif

  float bsbeFbm(vec2 pos, float pDens, float fTime, int nOct) {
    float sum = 0.0;
    float sDens = 1.0;
    pos += fTime * 0.001 * NL_CLOUD4_SPEED;
    for (int i = 0; i < 4; i++) {
      if (i < nOct) sum += bsbeVoronoi(pos) * sDens * pDens;
      else          sum += 0.42 * sDens * pDens;
      sDens *= 0.5;
      pos *= 2.5;
      pos += fTime * 0.05 * NL_CLOUD4_SPEED;
    }
    return saturate(1.0 - sum);
  }

  vec3 bsbeSatur(vec3 c, float sat) {
    float lum = dot(c, vec3(0.2125, 0.7154, 0.0721));
    return mix(vec3_splat(lum), c, sat);
  }

  vec3 bsbeClouds(vec3 color, vec3 nPos, float fTime, vec3 horC, vec3 zenC, vec2 fragCoord) {
    vec2 cloudP = (nPos.xz / nPos.y) * NL_CLOUD4_SCALE;
    cloudP -= cloudP * bsbeHash(fragCoord.x + fragCoord.y * 57.0) * 0.05;

    float sDens = NL_CLOUD4_DENSITY;
    vec3 cloDirC = horC * NL_CLOUD4_BRIGHTNESS;
    vec3 cloAmbC = bsbeSatur(zenC, 0.7);

    float lodY = nPos.y / NL_CLOUD4_LOD_Y;
    int nOct = int(step(1.0, lodY) + step(1.58, lodY) + step(2.5, lodY) + step(3.95, lodY));
    if (nOct > 0) {
      for (int i = 0; i < NL_CLOUD4_LAYERS; i++) {
        float cloudM = bsbeFbm(cloudP, sDens, fTime, nOct);
        cloDirC = mix(cloDirC, cloAmbC, NL_CLOUD4_AMBIENT_MIX);
        color = mix(color, cloDirC, cloudM * smoothstep(0.0, NL_CLOUD4_HORIZON_FADE, nPos.y));
        sDens += (i <= 6) ? -0.1 : 0.1;
        cloudP -= cloudP * NL_CLOUD4_LAYER_SQUEEZE;
      }
    }
    return color;
  }

  #endif
#endif

#endif
