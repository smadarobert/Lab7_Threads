#include <windows.h>
#include <iostream>

using namespace std;

// Sincronizare nativa Windows
CRITICAL_SECTION cs;
CONDITION_VARIABLE cond_w, cond_b;

// Variabile de stare
int active_w = 0, active_b = 0;
int waiting_w = 0, waiting_b = 0;
int turn = 0; // 0 = Albi, 1 = Negri

// Functia pentru firele Albe
DWORD WINAPI white_thread(LPVOID arg) {
    int id = *(int*)arg;

    EnterCriticalSection(&cs);
    waiting_w++;
    
    // Asteapta daca: sunt negri activi SAU (asteapta negri SI e randul lor)
    while (active_b > 0 || (waiting_b > 0 && turn == 1)) {
        SleepConditionVariableCS(&cond_w, &cs, INFINITE);
    }
    
    waiting_w--;
    active_w++;
    LeaveCriticalSection(&cs);

    // ZONA CRITICĂ
    cout << "[ALB " << id << "] A intrat. Albi activi in resursa: " << active_w << endl;
    Sleep(1000); // 1 secunda
    cout << "[ALB " << id << "] Iese din resursa." << endl;

    EnterCriticalSection(&cs);
    active_w--;
    if (active_w == 0) {
        turn = 1; // Dam randul Negrilor
        WakeAllConditionVariable(&cond_b);
    }
    LeaveCriticalSection(&cs);

    return 0;
}

// Functia pentru firele Negre
DWORD WINAPI black_thread(LPVOID arg) {
    int id = *(int*)arg;

    EnterCriticalSection(&cs);
    waiting_b++;
    
    // Asteapta daca: sunt albi activi SAU (asteapta albi SI e randul lor)
    while (active_w > 0 || (waiting_w > 0 && turn == 0)) {
        SleepConditionVariableCS(&cond_b, &cs, INFINITE);
    }
    
    waiting_b--;
    active_b++;
    LeaveCriticalSection(&cs);

    // ZONA CRITICĂ
    cout << "[NEGRU " << id << "] A intrat. Negri activi in resursa: " << active_b << endl;
    Sleep(1000); // 1 secunda
    cout << "[NEGRU " << id << "] Iese din resursa." << endl;

    EnterCriticalSection(&cs);
    active_b--;
    if (active_b == 0) {
        turn = 0; // Dam randul Albilor
        WakeAllConditionVariable(&cond_w);
    }
    LeaveCriticalSection(&cs);

    return 0;
}

int main() {
    const int NUM_THREADS = 5;
    HANDLE whites[NUM_THREADS], blacks[NUM_THREADS];
    int ids[NUM_THREADS];

    InitializeCriticalSection(&cs);
    InitializeConditionVariable(&cond_w);
    InitializeConditionVariable(&cond_b);

    cout << "--- Incepem simularea (Windows) cu " << NUM_THREADS << " fire albe si " << NUM_THREADS << " negre ---" << endl;

    // Cream firele
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        whites[i] = CreateThread(NULL, 0, white_thread, &ids[i], 0, NULL);
        blacks[i] = CreateThread(NULL, 0, black_thread, &ids[i], 0, NULL);
    }

    // Asteptam executia firelor (echivalent pthread_join)
    WaitForMultipleObjects(NUM_THREADS, whites, TRUE, INFINITE);
    WaitForMultipleObjects(NUM_THREADS, blacks, TRUE, INFINITE);

    // Curatam resursele
    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(whites[i]);
        CloseHandle(blacks[i]);
    }
    DeleteCriticalSection(&cs);

    cout << "--- Toate firele au terminat executia! ---" << endl;
    return 0;
}