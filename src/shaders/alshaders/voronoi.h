#pragma once

point floor(point p) { return point(floor(p[0]), floor(p[1]), floor(p[2])); }

// This is not a good hash function, but it works well enough
float hash_vec2(vector n) {
    float a = n[0] * 12.9898 + n[1] * 78.233;
    float b = sin(a) * 43758.5453;
    return mod(b, 1);
}

float hash_vec3(vector n) { return hash_vec2(vector(hash_vec2(n), n[2], 0.0)); }

float voronoi(int mode, point pp, float jitter, float shape, output float f1,
              output float f2, output point p1, output point p2) {
    // Thanks to IQ:
    // http://iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm
    point p = floor(pp);
    point f = pp - p;

    f1 = f2 = 8;
    color id = color(0);
    for (int k = -1; k <= 1; ++k)
        for (int j = -1; j <= 1; ++j)
            for (int i = -1; i <= 1; ++i) {
                vector b = vector(i, j, k);
                vector c = cellnoise(p + b);
                vector r = b - f + c * jitter;
                float d = pow(abs(r[0]), shape) + pow(abs(r[1]), shape) +
                          pow(abs(r[2]), shape);
                if (d < f1) {
                    f2 = f1;
                    f1 = d;
                    p1 = r;
                    id = c;
                } else if (d < f2) {
                    f2 = d;
                    p2 = r;
                }
            }
    if (mode == 0) {
        return pow(f1, 1 / shape) / pow(3.0, 1.0 / shape);
    } else if (mode == 1) {
        return pow(f2 - f1, 1 / shape) / pow(3.0, 1.0 / shape);
    } else {
        return hash_vec3(id);
    }
}

void voronoi3(point pp, float jitter, float shape, output float f[3],
              output point pc[3], output float rnd[3]) {
    // Thanks to IQ:
    // http://iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm
    point p = floor(pp);
    point v = pp - p;

    f[0] = f[1] = f[2] = 8;
    color id[3] = {color(0), color(0), color(0)};
    for (int k = -1; k <= 1; ++k)
        for (int j = -1; j <= 1; ++j)
            for (int i = -1; i <= 1; ++i) {
                vector b = vector(i, j, k);
                vector c = cellnoise(p + b);
                vector r = b - v + c * jitter;
                float d = pow(abs(r[0]), shape) + pow(abs(r[1]), shape) +
                          pow(abs(r[2]), shape);
                if (d < f[0]) {
                    f[2] = f[1];
                    f[1] = f[0];
                    f[0] = d;
                    pc[0] = r;
                    id[0] = c;
                } else if (d < f[1]) {
                    f[1] = d;
                    pc[1] = r;
                    id[1] = c;
                } else if (d < f[2]) {
                    f[2] = d;
                    pc[2] = r;
                    id[2] = c;
                }
            }

    for (int i = 0; i < 3; ++i) {
        rnd[i] = hash_vec3(id[i]);
    }
}
