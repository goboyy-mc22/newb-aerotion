#ifndef SKYAURORA_H
#define SKYAURORA_H

// Texture aurora (port of Voxels Reimagined GetAurora), shaped purely from AuroraTexture.png (R channel).
// Only included by Sky/fragment.sc.

#ifndef INSTANCING
  #ifdef NL_AURORA_TEX

  // buffer: src/materials/Sky/buffers/AuroraTexture.json (sampler name = s_ + buffer name)
  SAMPLER2D_AUTOREG(s_AuroraTexture);

  float nlAuroraCubic(vec2 p) {
    vec2 cell = vec2(1.0 / 128.0);
    vec2 quantized = floor(p / cell) * cell + cell * 2.0;
    return texture2DLod(s_AuroraTexture, quantized, 0.0).r;
  }

  vec3 nlAuroraTex(vec3 sky, vec3 vDir, float time, float dayFactor, float rain, float dither) {
    float night = (1.0 - smoothstep(-0.12, 0.25, dayFactor)) * (1.0 - rain);
    float VdotU = clamp(vDir.y, 0.0, 1.0);
    float visibility = sqrt(max(clamp(VdotU * 4.5 - 0.25, 0.0, 1.0), 0.0));
    visibility *= 8.0 - VdotU * 0.9;
    if (visibility <= 1.0 || night <= 0.0) { return sky; }

    vec3 aurora = vec3(0.0);
    vec3 wpos = vDir;
    wpos.xz /= max(wpos.y, 0.1);
    vec2 cameraPosM = vec2(time * NL_AURORA_TEX_SPEED, 0.0);

    float ditherM = dither + 10.0;
    float sampleP = float(NL_AURORA_TEX_SAMPLES + 10);

    for (int i = 0; i < NL_AURORA_TEX_SAMPLES; i++) {
      float current = (float(i) + ditherM) / sampleP;
      current *= current;
      float currentM = 1.0 - current;

      vec2 planePos = (wpos.xz * (0.9 + current) * 10.0 + cameraPosM) * NL_AURORA_TEX_SCALE;

      float n = nlAuroraCubic(planePos);
      n = clamp(1.0 - NL_AURORA_TEX_WIDTH * abs(n - NL_AURORA_TEX_CENTER), 0.0, 1.0);
      n = pow(n, NL_AURORA_TEX_POWER);

      float anim1 = nlAuroraCubic(planePos * 0.5);
      float anim2 = nlAuroraCubic(planePos * 0.8);
      n *= mix(anim1, anim2, 0.5);

      float cm2 = currentM * currentM;
      aurora += n * currentM * mix(NL_AURORA_TEX_COL2, NL_AURORA_TEX_COL1, cm2 * cm2);
    }

    return sky + aurora * (NL_AURORA_TEX * night * visibility / float(NL_AURORA_TEX_SAMPLES));
  }
  #endif
#endif

#endif
