// Définir différents pools pour différentes tailles
typedef enum {
    TEXTURE_SIZE_8 = 0,
    TEXTURE_SIZE_16,
    TEXTURE_SIZE_32,
    TEXTURE_SIZE_64,  
    TEXTURE_SIZE_128,     // 128x128
    TEXTURE_SIZE_256,
    TEXTURE_SIZE_512,     // 512x512
    TEXTURE_SIZE_CUSTOM,
    TEXTURE_SIZE_COUNT
} texture_size_type;

typedef struct {
    dct_texture_t* textures;
    int count;
    int capacity;
    uint16_t size;  // taille en pixels
} dct_texture_pool_t;

typedef struct dct_texture_manager{
    dct_texture_pool_t pools[TEXTURE_SIZE_COUNT];
} dct_texture_manager_t;

// Initialiser les pools avec des capacités appropriées
void init_texture_manager(dct_texture_manager_t* manager) {
    // Pool pour 32x32
    manager->pools[TEXTURE_SIZE_32].size = 32;
    manager->pools[TEXTURE_SIZE_32].capacity = 16;  // plus de petites textures
    manager->pools[TEXTURE_SIZE_32].count = 0;
    manager->pools[TEXTURE_SIZE_32].textures = malloc(sizeof(dct_texture_t) * 16);

    // Pool pour 128x128
    manager->pools[TEXTURE_SIZE_128].size = 128;
    manager->pools[TEXTURE_SIZE_128].capacity = 8;  // capacité moyenne
    manager->pools[TEXTURE_SIZE_128].count = 0;
    manager->pools[TEXTURE_SIZE_128].textures = malloc(sizeof(dct_texture_t) * 8);

    // Pool pour 512x512
    manager->pools[TEXTURE_SIZE_512].size = 512;
    manager->pools[TEXTURE_SIZE_512].capacity = 4;  // moins de grandes textures
    manager->pools[TEXTURE_SIZE_512].count = 0;
    manager->pools[TEXTURE_SIZE_512].textures = malloc(sizeof(dct_texture_t) * 4);
}

// Fonction pour ajouter une texture dans le bon pool
bool add_texture(dct_texture_manager_t* manager, dct_texture_t* texture) {
    texture_size_type pool_type;
    
    // Déterminer le bon pool selon la taille
    if (texture->width <= 32) pool_type = TEXTURE_SIZE_32;
    else if (texture->width <= 128) pool_type = TEXTURE_SIZE_128;
    else if (texture->width <= 512) pool_type = TEXTURE_SIZE_512;
    else return false;  // texture trop grande
    
    dct_texture_pool_t* pool = &manager->pools[pool_type];
    
    // Vérifier si il y a de la place
    if (pool->count >= pool->capacity) {
        return false;  // pool plein
    }
    
    // Ajouter la texture
    pool->textures[pool->count++] = *texture;
    return true;
}