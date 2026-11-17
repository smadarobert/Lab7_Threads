#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

// Mutex si variabile de conditie pentru sincronizare
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_w = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_b = PTHREAD_COND_INITIALIZER;

// Contoare pentru starea firelor
int active_w = 0, active_b = 0;
int waiting_w = 0, waiting_b = 0;
int turn = 0; // 0 = Rândul Albilor, 1 = Rândul Negrilor

void* white_thread(void* arg) {
    int id = *(int*)arg;
    
    // --- ZONA DE INTRARE ---
    pthread_mutex_lock(&mtx);
    waiting_w++;
    // Albii așteaptă dacă sunt negri înăuntru SAU dacă așteaptă negri și e rândul lor
    while (active_b > 0 || (waiting_b > 0 && turn == 1)) {
        pthread_cond_wait(&cond_w, &mtx);
    }
    waiting_w--;
    active_w++;
    pthread_mutex_unlock(&mtx);

    // --- ZONA CRITICĂ (Resursa) ---
    cout << "[ALB " << id << "] A intrat. Albi activi in resursa: " << active_w << endl;
    sleep(1); // Simulam utilizarea resursei (1 secunda)
    cout << "[ALB " << id << "] Iese din resursa." << endl;

    // --- ZONA DE IEȘIRE ---
    pthread_mutex_lock(&mtx);
    active_w--;
    if (active_w == 0) {
        turn = 1; // Ultimul alb care iese dă rândul negrilor (prevenim înfometarea)
        pthread_cond_broadcast(&cond_b); // Trezim toți negrii
    }
    pthread_mutex_unlock(&mtx);

    return NULL;
}

void* black_thread(void* arg) {
    int id = *(int*)arg;
    
    // --- ZONA DE INTRARE ---
    pthread_mutex_lock(&mtx);
    waiting_b++;
    // Negrii așteaptă dacă sunt albi înăuntru SAU dacă așteaptă albi și e rândul lor
    while (active_w > 0 || (waiting_w > 0 && turn == 0)) {
        pthread_cond_wait(&cond_b, &mtx);
    }
    waiting_b--;
    active_b++;
    pthread_mutex_unlock(&mtx);

    // --- ZONA CRITICĂ (Resursa) ---
    cout << "[NEGRU " << id << "] A intrat. Negri activi in resursa: " << active_b << endl;
    sleep(1); // Simulam utilizarea resursei (1 secunda)
    cout << "[NEGRU " << id << "] Iese din resursa." << endl;

    // --- ZONA DE IEȘIRE ---
    pthread_mutex_lock(&mtx);
    active_b--;
    if (active_b == 0) {
        turn = 0; // Ultimul negru care iese dă rândul albilor (prevenim înfometarea)
        pthread_cond_broadcast(&cond_w); // Trezim toți albii
    }
    pthread_mutex_unlock(&mtx);

    return NULL;
}

int main() {
    const int NUM_THREADS = 5;
    pthread_t whites[NUM_THREADS], blacks[NUM_THREADS];
    int ids[NUM_THREADS];

    cout << "--- Incepem simularea cu " << NUM_THREADS << " fire albe si " << NUM_THREADS << " negre ---" << endl;

    // Cream firele de executie
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&whites[i], NULL, white_thread, &ids[i]);
        pthread_create(&blacks[i], NULL, black_thread, &ids[i]);
    }

    // Asteptam ca toate firele sa termine
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(whites[i], NULL);
        pthread_join(blacks[i], NULL);
    }

    cout << "--- Toate firele au terminat executia! ---" << endl;
    return 0;
}
