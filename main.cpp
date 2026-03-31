#include <cstdio>

// Elevator system with two heaps (min-heap for upwards requests, max-heap for downwards requests)
// We maintain a custom hash map to track whether a floor request is currently active.
// Only cstdio/iostream permitted; we use cstdio for IO.

typedef long long i64;

struct MinHeap {
    i64 *a;
    int cnt;
    int cap;
    MinHeap(): a(0), cnt(0), cap(0) {}
    void init(i64 *buf, int capacity) {
        a = buf; cap = capacity; cnt = 0;
    }
    inline bool empty() const { return cnt == 0; }
    inline i64 top() const { return a[1]; }
    inline void push(i64 x) {
        // assume cap is sufficient
        int i = ++cnt;
        a[i] = x;
        for (int p = i >> 1; p; i = p, p >>= 1) {
            if (a[p] > a[i]) { i64 t = a[p]; a[p] = a[i]; a[i] = t; }
            else break;
        }
    }
    inline void pop() {
        if (cnt == 0) return;
        a[1] = a[cnt--];
        int i = 1;
        while (true) {
            int l = i << 1, r = l + 1, s = i;
            if (l <= cnt && a[l] < a[s]) s = l;
            if (r <= cnt && a[r] < a[s]) s = r;
            if (s != i) { i64 t = a[i]; a[i] = a[s]; a[s] = t; i = s; }
            else break;
        }
    }
};

struct MaxHeap {
    i64 *a;
    int cnt;
    int cap;
    MaxHeap(): a(0), cnt(0), cap(0) {}
    void init(i64 *buf, int capacity) {
        a = buf; cap = capacity; cnt = 0;
    }
    inline bool empty() const { return cnt == 0; }
    inline i64 top() const { return a[1]; }
    inline void push(i64 x) {
        int i = ++cnt;
        a[i] = x;
        for (int p = i >> 1; p; i = p, p >>= 1) {
            if (a[p] < a[i]) { i64 t = a[p]; a[p] = a[i]; a[i] = t; }
            else break;
        }
    }
    inline void pop() {
        if (cnt == 0) return;
        a[1] = a[cnt--];
        int i = 1;
        while (true) {
            int l = i << 1, r = l + 1, s = i;
            if (l <= cnt && a[l] > a[s]) s = l;
            if (r <= cnt && a[r] > a[s]) s = r;
            if (s != i) { i64 t = a[i]; a[i] = a[s]; a[s] = t; i = s; }
            else break;
        }
    }
};

static const int MAXN = 600000; // safety margin for operations

// Buffers for heaps (index from 1)
i64 up_buf[MAXN + 5];     // min-heap for requests above current floor
i64 down_buf[MAXN + 5];   // max-heap for requests below current floor

MinHeap up;    // upwards requests
MaxHeap down;  // downwards requests

// Custom open-addressing hash map per floor: track generation and active generation
// Capacity as power of two for fast masking
static const int HCAP = 1 << 21; // ~2 million slots
int hkey[HCAP];
unsigned char huse[HCAP]; // 0: empty, 1: occupied
int hgen[HCAP];           // total generations counter per floor
int hactive[HCAP];        // currently active generation (0 if none)

inline int hslot(int x) {
    unsigned int k = (unsigned int)x;
    unsigned int h = (k * 2654435761u) & (HCAP - 1);
    while (huse[h] && hkey[h] != x) {
        h = (h + 1) & (HCAP - 1);
    }
    return (int)h;
}

typedef long long i64;
static const int SHIFT = 21;
static const int MASK = (1 << SHIFT) - 1;

inline i64 make_key(int x, int gen) {
    return ((i64)x << SHIFT) | (i64)(gen & MASK);
}

inline int key_floor(i64 key) { return (int)(key >> SHIFT); }
inline int key_gen(i64 key) { return (int)(key & MASK); }

inline int get_slot(int x) { return hslot(x); }
inline int get_active_gen(int x) { int s = get_slot(x); return (huse[s] && hkey[s]==x) ? hactive[s] : 0; }
inline void set_active_gen(int x, int gen) { int s = get_slot(x); if (!huse[s]) { huse[s]=1; hkey[s]=x; hgen[s]=0; } hactive[s]=gen; }
inline int bump_gen_and_activate(int x) { int s = get_slot(x); if (!huse[s]) { huse[s]=1; hkey[s]=x; hgen[s]=0; } int g = ++hgen[s]; hactive[s]=g; return g; }

inline void clean_up() {
    while (!up.empty()) {
        i64 k = up.top();
        int x = key_floor(k), g = key_gen(k);
        int s = get_slot(x);
        if (!huse[s] || hkey[s]!=x || hactive[s] != g) { up.pop(); }
        else break;
    }
}

inline void clean_down() {
    while (!down.empty()) {
        i64 k = down.top();
        int x = key_floor(k), g = key_gen(k);
        int s = get_slot(x);
        if (!huse[s] || hkey[s]!=x || hactive[s] != g) { down.pop(); }
        else break;
    }
}

int main() {
    up.init(up_buf, MAXN);
    down.init(down_buf, MAXN);

    int n;
    if (std::scanf("%d", &n) != 1) return 0;
    int cur = 0; // current floor
    int dir = 1; // 1 up, -1 down
    char op[16];
    for (int i = 0; i < n; ++i) {
        if (std::scanf("%15s", op) != 1) return 0;
        if (op[0] == 'a') {
            if (op[1] == 'd') {
                // add
                int x; std::scanf("%d", &x);
                int g = bump_gen_and_activate(x);
                i64 key = make_key(x, g);
                if (x > cur) up.push(key);
                else /* x < cur */ down.push(key);
            } else {
                // action
                clean_up(); clean_down();
                bool has_up = false, has_down = false;
                clean_up(); has_up = !up.empty();
                clean_down(); has_down = !down.empty();
                if (!has_up && !has_down) {
                    // nothing to do
                } else if (dir == 1) {
                    if (has_up) {
                        i64 k = up.top(); up.pop();
                        int x = key_floor(k);
                        cur = x;
                        set_active_gen(x, 0);
                    } else if (has_down) {
                        dir = -1;
                        clean_down();
                        if (!down.empty()) { i64 k = down.top(); down.pop(); int x = key_floor(k); cur = x; set_active_gen(x, 0); }
                    }
                } else { // dir == -1
                    if (has_down) {
                        i64 k = down.top(); down.pop();
                        int x = key_floor(k);
                        cur = x;
                        set_active_gen(x, 0);
                    } else if (has_up) {
                        dir = 1;
                        clean_up();
                        if (!up.empty()) { i64 k = up.top(); up.pop(); int x = key_floor(k); cur = x; set_active_gen(x, 0); }
                    }
                }
            }
        } else if (op[0] == 'c') {
            // cancel
            int x; std::scanf("%d", &x);
            set_active_gen(x, 0);
        } else {
            // locate
            std::printf("%d\n", cur);
        }
    }
    return 0;
}
