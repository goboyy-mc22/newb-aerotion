#ifndef CLOUD_NOISE_H
#define CLOUD_NOISE_H

// Helper functions of the sky clouds (type 4): hash, texture voronoi, fbm, saturation.
// Code from clouds_bicubic.txt, only the sampler name matches the CloudsTextures buffer.
// Only included by Sky/fragment.sc.

#ifndef INSTANCING
  #ifdef NL_SKY_CLOUDS

  float bsbeHash(float h) {
    return fract(sin(h) * 43758.5453);
  }

  #ifdef NL_SKY_CLOUDS_NOISE_TEX
    SAMPLER2D_AUTOREG(s_CloudsTextures);

    float bsbeVoronoi(vec2 pos) {
      return texture2DLod(s_CloudsTextures, pos * (1.0 / NL_SKY_CLOUDS_NOISE_CELLS), 0.0).r;
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
    pos += fTime * 0.001 * NL_SKY_CLOUDS_SPEED;
    for (int i = 0; i < 4; i++) {
      if (i < nOct) sum += bsbeVoronoi(pos) * sDens * pDens;
      else          sum += 0.42 * sDens * pDens;
      sDens *= 0.5;
      pos *= 2.5;
      pos += fTime * 0.05 * NL_SKY_CLOUDS_SPEED;
    }
    return saturate(1.0 - sum);
  }

  vec3 bsbeSatur(vec3 c, float sat) {
    float lum = dot(c, vec3(0.2125, 0.7154, 0.0721));
    return mix(vec3_splat(lum), c, sat);
  }
  #endif
#endif

#endif
