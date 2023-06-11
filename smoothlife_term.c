#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 100
#define HEIGHT 100
#define STDOUT_BUF_LEN (50 * WIDTH * HEIGHT)

char level[] = " .-=coaA@#";
#define level_count (sizeof(level)/sizeof(level[0]) - 1)

float grid[HEIGHT][WIDTH] = {0};
float grid_diff[HEIGHT][WIDTH] = {0};
float ra = 11;
float alpha_n = 0.028;
float alpha_m = 0.147;
float b1 = 0.278;
float b2 = 0.365;
float d1 = 0.267;
float d2 = 0.445;
float dt = 0.05f;

float rand_float(void)
{
    return (float)rand()/(float)RAND_MAX;
}

void random_grid(void)
{
    size_t w = WIDTH/3;
    size_t h = HEIGHT/3;
    for (size_t dy = 0; dy < h; ++dy) {
        for (size_t dx = 0; dx < w; ++dx) {
            size_t x = dx + WIDTH/2 - w/2;
            size_t y = dy + HEIGHT/2 - h/2;
            grid[y][x] = rand_float();
        }
    }
}

void display_grid(float grid[HEIGHT][WIDTH])
{
    fputs("\x1B[1J", stdout);

#ifdef ANSI_TERM
#define GRAY_TO_RGB(x) x, x, x
    for (size_t y = 1; y < HEIGHT; y += 2) {
        for (size_t x = 0; x < WIDTH; ++x) {
            int curr = (int) (grid[y - 1][x] * 255.0f);
            int next = (int) (grid[y][x] * 255.0f);
            fprintf(stdout, "\x1B[38;2;%d;%d;%dm\x1B[48;2;%d;%d;%dm\u2580", GRAY_TO_RGB(curr), GRAY_TO_RGB(next));
        }
        fputs("\x1B[0m", stdout);
        fputc('\n', stdout);
    }
    fputs("\x1B[0m", stdout);
#else
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            char c = level[(int)(grid[y][x]*(level_count - 1))];
            fputc(c, stdout);
            fputc(c, stdout);
        }
        fputc('\n', stdout);
    }
#endif

	fflush(stdout);
}

int emod(int a, int b)
{
    return (a%b + b)%b;
}

float sigma(float x, float a, float alpha)
{
    return 1.0f/(1.0f + expf(-(x - a)*4/alpha));
}

float sigma_n(float x, float a, float b)
{
    return sigma(x, a, alpha_n)*(1 - sigma(x, b, alpha_n));
}

float sigma_m(float x, float y, float m)
{
    return x*(1 - sigma(m, 0.5f, alpha_m)) + y*sigma(m, 0.5f, alpha_m);
}

float s(float n, float m)
{
    return sigma_n(n, sigma_m(b1, d1, m), sigma_m(b2, d2, m));
}

void compute_grid_diff(void)
{
    for (int cy = 0; cy < HEIGHT; ++cy) {
        for (int cx = 0; cx < WIDTH; ++cx) {
            float m = 0, M = 0;
            float n = 0, N = 0;
            float ri = ra/3;

            for (int dy = -(ra - 1); dy <= (ra - 1); ++dy) {
                for (int dx = -(ra - 1); dx <= (ra - 1); ++dx) {
                    int x = emod(cx + dx, WIDTH);
                    int y = emod(cy + dy, HEIGHT);
                    if (dx*dx + dy*dy <= ri*ri) {
                        m += grid[y][x];
                        M += 1;
                    } else if (dx*dx + dy*dy <= ra*ra) {
                        n += grid[y][x];
                        N += 1;
                    }
                }
            }
            m /= M;
            n /= N;
            float q = s(n, m);
            grid_diff[cy][cx] = 2*q - 1;
        }
    }
}

void clamp(float *x, float l, float h)
{
    if (*x < l) *x = l;
    if (*x > h) *x = h;
}

void apply_grid_diff(void)
{
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            grid[y][x] += dt*grid_diff[y][x];
            clamp(&grid[y][x], 0, 1);
        }
    }
}

int main(void)
{
    srand(time(0));

    char stdout_buf[STDOUT_BUF_LEN];
    assert(setvbuf(stdout, stdout_buf, _IOFBF, STDOUT_BUF_LEN) == 0);

    random_grid();

    display_grid(grid);
    for (;;) {
        compute_grid_diff();
        apply_grid_diff();
        display_grid(grid);
    }

    return 0;
}
