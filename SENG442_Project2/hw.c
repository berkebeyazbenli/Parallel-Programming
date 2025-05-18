#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

#define MAX_THREADS 64

// Fonksiyon seçimi
typedef double (*Func)(double);

double func_sin(double x) { return sin(x); }
double func_square(double x) { return x * x; }
double func_exp(double x) { return exp(-x * x); }

// Riemann
double integrate_riemann(Func f, double a, double b, int steps, int num_threads, const char *sched)
{
    double h = (b - a) / steps;
    double sum = 0.0;

#pragma omp parallel for schedule(runtime) num_threads(num_threads) reduction(+ : sum)
    for (int i = 0; i < steps; i++)
    {
        double x = a + i * h;
        int tid = omp_get_thread_num();
        if (i % (steps / 10) == 0)
            printf("[Riemann-%s] Thread %d handling x=%.5f\n", sched, tid, x);
        sum += f(x) * h;
    }
    return sum;
}

// Trapezoidal
double integrate_trapezoidal(Func f, double a, double b, int steps, int num_threads, const char *sched)
{
    double h = (b - a) / steps;
    double sum = 0.0;

#pragma omp parallel for schedule(runtime) num_threads(num_threads) reduction(+ : sum)
    for (int i = 1; i < steps; i++)
    {
        double x = a + i * h;
        int tid = omp_get_thread_num();
        if (i % (steps / 10) == 0)
            printf("[Trapezoidal-%s] Thread %d handling x=%.5f\n", sched, tid, x);
        sum += f(x);
    }
    sum += (f(a) + f(b)) / 2.0;
    return sum * h;
}

// Simpson
double integrate_simpson(Func f, double a, double b, int steps, int num_threads, const char *sched)
{
    if (steps % 2 != 0)
        steps++;
    double h = (b - a) / steps;
    double sum = f(a) + f(b);

#pragma omp parallel for schedule(runtime) num_threads(num_threads) reduction(+ : sum)
    for (int i = 1; i < steps; i++)
    {
        double x = a + i * h;
        int tid = omp_get_thread_num();
        if (i % (steps / 10) == 0)
            printf("[Simpson-%s] Thread %d handling x=%.5f\n", sched, tid, x);
        sum += f(x) * (i % 2 == 0 ? 2 : 4);
    }
    return sum * h / 3.0;
}

// Zamanlama ve loglama
void benchmark(Func f, const char *fname, const char *method,
               double(integrate)(Func, double, double, int, int, const char *),
               double a, double b, int steps, int num_threads, const char *sched)
{
    if (strcmp(sched, "dynamic") == 0)
        omp_set_schedule(omp_sched_dynamic, 0);
    else if (strcmp(sched, "guided") == 0)
        omp_set_schedule(omp_sched_guided, 0);
    else
        omp_set_schedule(omp_sched_static, 0);

    double start = omp_get_wtime();
    double result = integrate(f, a, b, steps, num_threads, sched);
    double end = omp_get_wtime();
    printf("[%s][%s-%s] Threads: %d, Result: %.8f, Time: %.6fs\n", fname, method, sched, num_threads, result, end - start);
}

int main(int argc, char *argv[])
{
    double a = 0.0, b = M_PI;
    int steps = 10000000;
    int num_threads = 8;

    Func functions[] = {func_sin, func_square, func_exp};
    const char *func_names[] = {"sin(x)", "x^2", "exp(-x^2)"};
    const char *sched_types[] = {"static", "dynamic", "guided"};

    for (int s = 0; s < 3; s++)
    {
        for (int i = 0; i < 3; i++)
        {
            benchmark(functions[i], func_names[i], "Riemann", integrate_riemann, a, b, steps, num_threads, sched_types[s]);
            benchmark(functions[i], func_names[i], "Trapezoidal", integrate_trapezoidal, a, b, steps, num_threads, sched_types[s]);
            benchmark(functions[i], func_names[i], "Simpson", integrate_simpson, a, b, steps, num_threads, sched_types[s]);
        }
    }

    return 0;
}