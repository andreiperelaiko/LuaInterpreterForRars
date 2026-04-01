static unsigned int abs_u32(int x) {
    unsigned int ux = (unsigned int)x;
    if (x >= 0) return ux;
    return (~ux) + 1u;
}

int __mulsi3(int a, int b) {
    int neg = ((a < 0) ^ (b < 0));
    unsigned int ua = abs_u32(a);
    unsigned int ub = abs_u32(b);
    unsigned int res = 0;

    while (ub) {
        if (ub & 1u) res += ua;
        ua <<= 1;
        ub >>= 1;
    }

    if (!neg) return (int)res;
    return -(int)res;
}

static unsigned int udiv_u32(unsigned int n, unsigned int d) {
    if (d == 0u) return 0u;
    unsigned int q = 0u;
    unsigned int r = 0u;

    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) {
            r -= d;
            q |= (1u << i);
        }
    }
    return q;
}

unsigned int __udivsi3(unsigned int a, unsigned int b) {
    return udiv_u32(a, b);
}

unsigned int __umodsi3(unsigned int a, unsigned int b) {
    if (b == 0u) return 0u;
    unsigned int q = udiv_u32(a, b);
    return a - (__mulsi3((int)q, (int)b));
}

int __divsi3(int a, int b) {
    if (b == 0) return 0;
    int neg = ((a < 0) ^ (b < 0));
    unsigned int ua = abs_u32(a);
    unsigned int ub = abs_u32(b);
    unsigned int q = udiv_u32(ua, ub);
    if (!neg) return (int)q;
    return -(int)q;
}

int __modsi3(int a, int b) {
    if (b == 0) return 0;
    int q = __divsi3(a, b);
    return a - __mulsi3(q, b);
}
