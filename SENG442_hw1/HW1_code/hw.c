/*
    Aşağıda yorum satırı içinde bulunan kod parçaları, paralel algoritmanın düzgün çalışıp çalışmadığını anlamak için eklenmiş debug satırlarıdır.
    Bu satırları aktif ederek veya kapatarak, algoritmanın davranışını inceleyebilirsiniz.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 500 // Daha küçük bir dizi boyutu
#define NUM_THREADS 4  // Daha az thread kullanarak daha verimli bellek kullanımı

// Global değişkenler
int array[ARRAY_SIZE];
int total_perfect_squares = 0;                     // Global değişken için mutex koruma
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex değişkeni

// Tam kare olup olmadığını kontrol eden fonksiyon
bool is_perfect_square(int num)
{
    int sqrt_num = (int)sqrt(num);
    return (sqrt_num * sqrt_num == num);
}

void *generate_random_numbers(void *arg)
{
    int thread_id = *(int *)arg;
    int start = thread_id * (ARRAY_SIZE / NUM_THREADS);
    int end = (thread_id + 1) * (ARRAY_SIZE / NUM_THREADS);

    unsigned int seed = time(NULL) ^ thread_id;
    for (int i = start; i < end; i++)
    {
        array[i] = rand_r(&seed) % 100000;
    }

    pthread_exit(NULL);
}

// Thread'lerin çalıştıracağı fonksiyon
void *parallel_search(void *arg)
{
    int thread_id = *(int *)arg;
    int start = thread_id * (ARRAY_SIZE / NUM_THREADS);

    // int end = (thread_id == NUM_THREADS - 1) ? ARRAY_SIZE : (thread_id + 1) * (ARRAY_SIZE / NUM_THREADS);
    int end = (thread_id + 1) * (ARRAY_SIZE / NUM_THREADS);

    // Thread başlangıç zamanı
    clock_t start_time = clock();

    int local_count = 0;
    for (int i = start; i < end; i++)
    {
        if (is_perfect_square(array[i]))
        {
            // printf("Thread %d found perfect square: %d at index %d\n", thread_id, array[i], i);
            local_count++;
        }
    }

    // Mutex kullanarak global sayıyı güncelleme
    pthread_mutex_lock(&mutex);
    total_perfect_squares += local_count;
    pthread_mutex_unlock(&mutex);

    // Thread bitiş zamanı
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    // printf("Thread %d: Finished in %f seconds (Local Count: %d)\n", thread_id, time_taken, local_count);
    printf("Thread %d: Finished in %f seconds\n", thread_id, time_taken);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    printf("Number of threads: %d\n", NUM_THREADS);
    printf("Array size: %d\n", ARRAY_SIZE);

    // Toplam zaman ölçümünü başlat
    clock_t total_start = clock();

    /*
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            array[i] = (i % 2 == 0) ? 16 : 15; // Çift indekslere 16, tek indekslere 15 atar
        }
    */

    // Diziyi rastgele değerlerle doldur
    /*
        srand(time(NULL));
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            array[i] = rand() % 100000; // 100000'e kadar rastgele sayılar
        }
    */
    

    // Thread'leri oluştur ve başlat
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, generate_random_numbers, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, parallel_search, (void *)&thread_ids[i]);
    }

    // Thread'lerin bitmesini bekle
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Toplam işlem süresini hesapla
    clock_t total_end = clock();
    double total_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;

    printf("Total perfect squares found: %d\n", total_perfect_squares);
    printf("Total execution time: %f seconds\n", total_time);

    // Mutex kaynaklarını serbest bırakma
    pthread_mutex_destroy(&mutex);

    return 0;
}
