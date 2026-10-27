#ifndef SKYAURORA_H
#define SKYAURORA_H

// Texture aurora: shaped purely from AuroraTexture.png (voronoi border lines, only R is read).
// No hash/noise helper functions, only texture reads. Only included by Sky/fragment.sc.

#ifndef INSTANCING
  #ifdef NL_AURORA_TEX

  // buffer: src/materials/Sky/buffers/AuroraTexture.json (sampler name = s_ + buffer name)
  SAMPLER2D_AUTOREG(s_AuroraTexture);

  // dir: normalized view direction, y > 0 above the horizon
  // soft sample of voronoi-border texture (R: 0 = line). 5 taps for blur
float nlAuroraSoft(vec2 q) {
  float r = NL_AURORA_TEX_BLUR;
  float a = texture2DLod(s_AuroraTexture, q, 0.0).r;
  a += texture2DLod(s_AuroraTexture, q + vec2(r, 0.0), 0.0).r;
  a += texture2DLod(s_AuroraTexture, q - vec2(r, 0.0), 0.0).r;
  a += texture2DLod(s_AuroraTexture, q + vec2(0.0, 2.0*r), 0.0).r;
  a += texture2DLod(s_AuroraTexture, q - vec2(0.0, 2.0*r), 0.0).r;
  return a * 0.2;
}
vec3 nlAuroraTex(vec3 sky, vec3 dir, float t, float dayFactor, float rain) {
  float vis = (1.0 - smoothstep(-0.12, 0.25, dayFactor)) * (1.0 - rain) * smoothstep(0.0, NL_AURORA_TEX_FADE, dir.y);
  if (vis <= 0.0) { return sky; }
  vec2 p = dir.xz / (dir.y + 0.15);
  float cover = smoothstep(NL_AURORA_TEX_COVER - 0.15, NL_AURORA_TEX_COVER + 0.15,
                           1.0 - texture2DLod(s_AuroraTexture, p*0.03 + vec2(0.0, t*NL_AURORA_TEX_SPEED*0.3), 0.0).r);
  if (cover <= 0.0) { return sky; }
  vec3 acc = vec3(0.0);
  for (int i = 0; i < NL_AURORA_TEX_LAYERS; i++) {
    float k = float(i) / float(NL_AURORA_TEX_LAYERS - 1);
    vec2 q = vec2(p.x*NL_AURORA_TEX_STRETCH, p.y) * NL_AURORA_TEX_SCALE * (1.0 + 0.5*k) + vec2(0.31*k, 0.77*k + t*NL_AURORA_TEX_SPEED);
    float line = clamp((1.0 - nlAuroraSoft(q)) * 2.2 - 0.15, 0.0, 1.0);
    line = line * sqrt(line);
    acc += line * mix(NL_AURORA_TEX_COL1, NL_AURORA_TEX_COL2, k) * (1.0 - 0.4*k);
  }
  return sky + acc * (NL_AURORA_TEX * vis * cover / float(NL_AURORA_TEX_LAYERS));
}
#endif
#endif

#endif
