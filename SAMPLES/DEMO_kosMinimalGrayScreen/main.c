#include <kos.h>


typedef struct {
    int fps;
    float frame_time;
    size_t total_ram;    // RAM totale disponible
    size_t used_ram;     // RAM utilisée
    uint32_t pvr_free;   // PVR libre
} perf_stats_t;

perf_stats_t monitor_performance() {
    static uint64_t last_time = 0;
    static uint32_t frames = 0;
    static uint64_t last_fps_update = 0;
    static int current_fps = 0;
    
    perf_stats_t stats = {0};
    
    // Calcul FPS
    uint64_t current_time = timer_ms_gettime64();
    frames++;
    if (current_time - last_fps_update >= 1000) {
        current_fps = frames;
        frames = 0;
        last_fps_update = current_time;
    }
    
    // Temps par frame
    float frame_time = current_time - last_time;
    last_time = current_time;

    // RAM totale disponible (16MB - zone système KOS)
    void* heap_end = sbrk(0);
    void* heap_start = (void*)0x8c010000;  // Début typique du heap
    
    stats.total_ram = 16 * 1024 * 1024;    // 16MB total
    stats.used_ram = (char*)heap_end - (char*)heap_start;
    stats.fps = current_fps;
    stats.frame_time = frame_time;
    stats.pvr_free = pvr_mem_available();
    
    return stats;
}


void test_memory_limit_binary() {
    size_t total_allocated = 0;
    size_t current_size = 8*1024*1024;  // Commencer par 8MB
    void** blocks = malloc(1000 * sizeof(void*));
    int block_count = 0;
    
    printf("Test allocation mémoire avec recherche binaire...\n");
    
    while(current_size >= 1) {  // Jusqu'à 1 octet
        void* block = malloc(current_size);
        if(block) {
            blocks[block_count++] = block;
            total_allocated += current_size;
            printf("Succès allocation de %lu octets (Total: %lu)\n", current_size, total_allocated);
        } else {
            current_size /= 2;  // Diviser par 2 si échec
        }
    }
    
    printf("\nTotal alloué: %lu octets\n", total_allocated);
    printf("Mémoire système: %lu octets\n", 16*1024*1024 - total_allocated);
    
    for(int i = 0; i < block_count; i++) {
        free(blocks[i]);
    }
    free(blocks);
}


void display_stats() {
    perf_stats_t stats = monitor_performance();
    
    printf("FPS: %d\n", stats.fps);
    printf("Frame Time: %.2f ms\n", stats.frame_time);
    printf("RAM Totale: %lu KB\n", stats.total_ram / 1024);
    printf("RAM Utilisée: %lu KB\n", stats.used_ram / 1024);
    printf("RAM Libre: %lu KB\n", (stats.total_ram - stats.used_ram) / 1024);
    printf("PVR Libre: %lu KB\n", stats.pvr_free / 1024);
}

void check_detailed_memory() {
    // Point de départ de la RAM utilisable
    uintptr_t ram_start = 0x8c000000;
    
    // Obtenir la fin actuelle du heap
    uintptr_t heap_end = (uintptr_t)sbrk(0);
    
    // Taille totale utilisée
    size_t total_used = heap_end - ram_start;
    
    // Obtenir l'info du heap
    struct mallinfo mi = mallinfo();
    
    printf("Mémoire totale utilisée: %lu KB\n", total_used / 1024);
    printf("Heap alloué: %lu KB\n", mi.arena / 1024);
    printf("Heap utilisé: %lu KB\n", mi.uordblks / 1024);
    printf("Heap libre: %lu KB\n", mi.fordblks / 1024);
}

int main(void) {

    pvr_init_defaults();
    pvr_set_bg_color(0.2f, 0.2f, 0.2f);


    test_memory_limit_binary();



    while(1) {
        check_detailed_memory();
        display_stats();

        pvr_wait_ready();

        pvr_scene_begin();
        pvr_scene_finish();



        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if(st->buttons & CONT_START)
            {
                return -1;
            }
        MAPLE_FOREACH_END()
    }

    return 0;
}