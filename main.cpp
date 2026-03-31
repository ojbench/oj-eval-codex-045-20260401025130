#include <cstdio>

// Elevator system with two heaps (min-heap for upwards requests, max-heap for downwards requests)
// We maintain a custom hash map to track whether a floor request is currently active.
// Only cstdio/iostream permitted; we use cstdio for IO.

struct MinHeap {
    int *a;
    int cnt;
    int cap;
    MinHeap(): a(0), cnt(0), cap(0) {}
    void init(int *buf, int capacity) {
        a = buf; cap = capacity; cnt = 0;
    }
    inline bool empty() const { return cnt == 0; }
    inline int top() const { return a[1]; }
    inline void push(int x) {
        // assume cap is sufficient
        int i = ++cnt;
        a[i] = x;
        for (int p = i >> 1; p; i = p, p >>= 1) {
            if (a[p] > a[i]) { int t = a[p]; a[p] = a[i]; a[i] = t; }
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
            if (s != i) { int t = a[i]; a[i] = a[s]; a[s] = t; i = s; }
            else break;
        }
    }
};

struct MaxHeap {
    int *a;
    int cnt;
    int cap;
    MaxHeap(): a(0), cnt(0), cap(0) {}
    void init(int *buf, int capacity) {
        a = buf; cap = capacity; cnt = 0;
    }
    inline bool empty() const { return cnt == 0; }
    inline int top() const { return a[1]; }
    inline void push(int x) {
        int i = ++cnt;
        a[i] = x;
        for (int p = i >> 1; p; i = p, p >>= 1) {
            if (a[p] < a[i]) { int t = a[p]; a[p] = a[i]; a[i] = t; }
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
            if (s != i) { int t = a[i]; a[i] = a[s]; a[s] = t; i = s; }
            else break;
        }
    }
};

static const int MAXN = 600000; // safety margin for operations

// Buffers for heaps (index from 1)
int up_buf[MAXN + 5];     // min-heap for requests above current floor
int down_buf[MAXN + 5];   // max-heap for requests below current floor

MinHeap up;    // upwards requests
MaxHeap down;  // downwards requests

// Custom open-addressing hash map: key -> present flag (0/1)
// Capacity as power of two for fast masking
static const int HCAP = 1 << 21; // ~2 million slots
int hkey[HCAP];
unsigned char huse[HCAP]; // 0: empty, 1: occupied
unsigned char hval[HCAP]; // presence flag for the key (0/1)

inline int hslot(int x) {
    unsigned int k = (unsigned int)x;
    unsigned int h = (k * 2654435761u) & (HCAP - 1);
    while (huse[h] && hkey[h] != x) {
        h = (h + 1) & (HCAP - 1);
    }
    return (int)h;
}

inline void set_present(int x, int v) {
    int s = hslot(x);
    if (!huse[s]) {
        huse[s] = 1; hkey[s] = x; hval[s] = (unsigned char)(v != 0);
    } else {
        hval[s] = (unsigned char)(v != 0);
    }
}

inline int is_present(int x) {
    int s = hslot(x);
    if (!huse[s]) return 0;
    return hval[s] != 0;
}

inline void clean_up() {
    while (!up.empty() && !is_present(up.top())) up.pop();
}

inline void clean_down() {
    while (!down.empty() && !is_present(down.top())) down.pop();
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
                if (x > cur) up.push(x);
                else /* x < cur */ down.push(x);
                set_present(x, 1);
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
                        int t = up.top(); up.pop();
                        cur = t;
                        set_present(t, 0);
                    } else if (has_down) {
                        dir = -1;
                        clean_down();
                        if (!down.empty()) { int t = down.top(); down.pop(); cur = t; set_present(t, 0); }
                    }
                } else { // dir == -1
                    if (has_down) {
                        int t = down.top(); down.pop();
                        cur = t;
                        set_present(t, 0);
                    } else if (has_up) {
                        dir = 1;
                        clean_up();
                        if (!up.empty()) { int t = up.top(); up.pop(); cur = t; set_present(t, 0); }
                    }
                }
            }
        } else if (op[0] == 'c') {
            // cancel
            int x; std::scanf("%d", &x);
            set_present(x, 0);
        } else {
            // locate
            std::printf("%d\n", cur);
        }
    }
    return 0;
}
