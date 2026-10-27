#ifndef SKYAURORA_H
#define SKYAURORA_H

// Texture aurora: shaped purely from AuroraTexture.png (R: curtain line, G: rays, B: coverage mask).
// No hash/noise helper functions, only texture reads. Only included by Sky/fragment.sc.

#ifndef INSTANCING
  #ifdef NL_AURORA_TEX

  // buffer: src/materials/Sky/buffers/AuroraTexture.json (sampler name = s_ + buffer name)
  SAMPLER2D_AUTOREG(s_AuroraTexture);

  // dir: normalized view direction, y > 0 above the horizon
  vec3 nlAuroraTex(vec3 sky, vec3 dir, float t, float dayFactor, float rain) {
    float vis = (1.0 - smoothstep(-0.12, 0.25, dayFactor)) * (1.0 - rain)
              * smoothstep(0.0, NL_AURORA_TEX_FADE, dir.y);
    if (vis <= 0.0) { return sky; }

    vec2 p = dir.xz / (dir.y + 0.15);

    // where the aurora is (very coarse read of the texture)
    float cover = smoothstep(NL_AURORA_TEX_COVER - 0.12, NL_AURORA_TEX_COVER + 0.12,
                             texture2DLod(s_AuroraTexture, p*0.07 + vec2(0.0, t*NL_AURORA_TEX_SPEED*0.3), 0.0).b);
    if (cover <= 0.0) { return sky; }

    vec3 acc = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < NL_AURORA_TEX_LAYERS; i++) {
      float k = float(i) / float(NL_AURORA_TEX_LAYERS - 1);
      vec2 q = vec2(p.x*NL_AURORA_TEX_STRETCH, p.y) * NL_AURORA_TEX_SCALE * (1.0 + 0.5*k)
             + vec2(0.31*k, 0.77*k + t*NL_AURORA_TEX_SPEED);

      // curtain line: bright where the texture is around 0.5
      float n = texture2DLod(s_AuroraTexture, q, 0.0).r;
      float line = clamp(1.0 - abs(n - 0.5) * NL_AURORA_TEX_WIDTH, 0.0, 1.0);
      line *= line;

      // vertical rays
      float rays = texture2DLod(s_AuroraTexture, vec2(p.x*0.9 + k*0.2, q.y*0.3 + t*0.02), 0.0).g;
      line *= 1.0 + (rays*2.0 - 1.0) * NL_AURORA_TEX_RAYS;

      acc += line * mix(NL_AURORA_TEX_COL1, NL_AURORA_TEX_COL2, k) * (1.0 - 0.4*k);
    }
    return sky + acc * (NL_AURORA_TEX * vis * cover / float(NL_AURORA_TEX_LAYERS));
  }

  #endif
#endif

#endif
