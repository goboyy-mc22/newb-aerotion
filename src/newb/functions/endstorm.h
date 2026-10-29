#ifndef ENDSTORM_H
#define ENDSTORM_H

// Storm wall around the main End island, fixed in WORLD space (not on the sky).
// Very light: few steps (NL_END_STORM_STEPS), one 2D noise per step, one analytic bolt.
// Ray-marches a swirling cylindrical shell centered on NL_END_STORM_CENTER, using the absolute camera position.
// Same function is used by terrain (ray ends at the fragment) and EndSky (ray goes to infinity).

#include "noise.h"

#ifdef NL_END_STORM

// lightning event for slot k: xy = bolt position (world xz), z = flash envelope 0..1 (0 = no bolt now)
vec3 nlEndBolt(float t, float k) {
  float ts = t * NL_END_STORM_RATE + k * 0.37;
  float slot = floor(ts);
  float ph = ts - slot;
  float h = fract(sin(slot * 12.9898 + k * 78.233) * 43758.5453);
  float on = step(0.3, h);
  float e = (1.0 - ph / 0.3) * step(ph, 0.3);
  e *= 0.65 + 0.35 * step(0.5, fract(ph * 17.0)); // flicker
  float ang = 6.2831853 * fract(h * 7.31 + k * 0.21);
  float rad = NL_END_STORM_RADIUS + NL_END_STORM_THICK * (0.3 + 0.4 * fract(h * 13.7));
  return vec3(NL_END_STORM_CENTER + rad * vec2(cos(ang), sin(ang)), e * on * NL_END_STORM_FLASH);
}

// overall flash amount (0..1) used to light up the island
float nlEndStormFlash(float t) {
  return nlEndBolt(t, 0.0).z;
}

// returns vec4(rgb = added light, a = transmittance)
vec4 nlEndStorm(vec3 camPos, vec3 dir, float maxDist, float t, float dither) {
  const float R0 = NL_END_STORM_RADIUS;
  const float R1 = NL_END_STORM_RADIUS + NL_END_STORM_THICK;

  vec2 o = camPos.xz - NL_END_STORM_CENTER;
  vec2 d = dir.xz;
  float a = max(dot(d, d), 1e-5);
  float b = dot(o, d);

  // inside outer circle segment
  float disc = b * b - a * (dot(o, o) - R1 * R1);
  if (disc < 0.0) { return vec4(0.0, 0.0, 0.0, 1.0); }
  float s = sqrt(disc);
  float tA = max((-b - s) / a, 0.0);
  float tB = min((-b + s) / a, maxDist);

  // camera inside the hole: start where the ray leaves the inner circle
  float c0 = dot(o, o) - R0 * R0;
  if (c0 < 0.0) { tA = max(tA, (-b + sqrt(max(b * b - a * c0, 0.0))) / a); }

  // height slab
  float dy = abs(dir.y) < 1e-3 ? 1e-3 : dir.y;
  float ty0 = (NL_END_STORM_BOTTOM - camPos.y) / dy;
  float ty1 = (NL_END_STORM_TOP - camPos.y) / dy;
  tA = max(tA, min(ty0, ty1));
  tB = min(tB, max(ty0, ty1));
  if (tB <= tA) { return vec4(0.0, 0.0, 0.0, 1.0); }

  vec3 bolt0 = nlEndBolt(t, 0.0);
  float rot0 = mod(t * NL_END_STORM_SPEED, 6.2831853);

  float dt = (tB - tA) / float(NL_END_STORM_STEPS);
  float T = 1.0;
  vec3 col = vec3(0.0);

  for (int i = 0; i < NL_END_STORM_STEPS; i++) {
    float tt = tA + (float(i) + dither) * dt;
    vec3 p = camPos + dir * tt;
    vec2 q = p.xz - NL_END_STORM_CENTER;
    float r = length(q);

    float ring = smoothstep(R0, R0 + NL_END_STORM_THICK * 0.35, r) * (1.0 - smoothstep(R0 + NL_END_STORM_THICK * 0.6, R1, r));
    float hy = smoothstep(NL_END_STORM_BOTTOM, NL_END_STORM_BOTTOM + 25.0, p.y) * (1.0 - smoothstep(NL_END_STORM_TOP - 60.0, NL_END_STORM_TOP, p.y));
    float env = ring * hy;
    if (env > 0.01) {
      // swirl: rotate the noise domain around the island axis, faster near the center
      float ang = rot0 + r * 0.012;
      float cs = cos(ang);
      float sn = sin(ang);
      vec2 qr = vec2(cs * q.x - sn * q.y, sn * q.x + cs * q.y) * 0.028;
      float n = noise2D(qr + vec2(p.y * 0.02, t * 0.04));
      float dens = env * smoothstep(0.42, 0.72, n) * NL_END_STORM_DENSITY;

      if (dens > 0.0) {
        float sigma = dens * dt * 0.09;
        float absorb = 1.0 - exp(-sigma);

        // cloud color: darker at the bottom, lighter at the top
        vec3 c = mix(NL_END_STORM_COL1, NL_END_STORM_COL2, smoothstep(NL_END_STORM_BOTTOM, NL_END_STORM_TOP, p.y) * (0.4 + 0.6 * n));

        // clouds near a bolt light up
        vec2 e0 = p.xz - bolt0.xy;
        float lit = bolt0.z * exp(-dot(e0, e0) / 2500.0);
        c += NL_END_STORM_LIGHTNING * lit * 1.5;

        col += T * absorb * c;
        T *= 1.0 - absorb;
      }
    }
  }

  // bolts: analytic vertical lines (stay thin and stable regardless of step count)
  {
    vec3 bo = bolt0;
    if (bo.z > 0.0) {
      float tc = clamp(dot(bo.xy - NL_END_STORM_CENTER - o, d) / a, 0.0, maxDist);
      float yc = camPos.y + dir.y * tc;
      float y0 = NL_END_STORM_BOTTOM + 10.0;
      float y1 = NL_END_STORM_TOP - 25.0;
      if (yc > y0 && yc < y1) {
        // jagged offset along the bolt
        vec2 side = vec2(-d.y, d.x) / sqrt(a);
        float jag = (noise1D(yc * 0.12 + bo.x) - 0.5) * 14.0 + (noise1D(yc * 0.45 + bo.y) - 0.5) * 4.0;
        vec2 pc = o + d * tc;
        float hd = length(pc - (bo.xy - NL_END_STORM_CENTER) - side * jag);
        float w = 1.2 + tc * 0.006;
        float g = exp(-hd * hd / (w * w)) + 0.25 * exp(-hd * hd / (9.0 * w * w));
        float fade = smoothstep(y0, y0 + 25.0, yc) * (1.0 - smoothstep(y1 - 25.0, y1, yc));
        col += NL_END_STORM_LIGHTNING * 3.0 * g * fade * bo.z * (0.4 + 0.6 * T);
      }
    }
  }

  return vec4(col, T);
}

#endif
#endif
