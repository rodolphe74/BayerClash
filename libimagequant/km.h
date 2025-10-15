#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// --- Structures de couleurs ---
// Utilisé pour les couleurs normalisées (flottants 0.0-1.0)
typedef struct {
    float r, g, b;
} ColorKM;

// Utilisé pour les couleurs en octets (entiers 0-255)
typedef struct {
    int r, g, b;
} ColorByte;

// Structure pour l'entrée du K-Means (Couleur + Poids)
typedef struct {
    ColorByte color; // {R, G, B} [0, 255]
    int weight;      // Compteur de pixels
} WeightedPixel;

// Structure pour le calcul des centroïdes (Étapes 1 et 2)
typedef struct {
    double r_sum, g_sum, b_sum;
    double total_weight; // Poids total accumulé
} CentroidData;

// --- Constantes de l'algorithme ---
#define MAX_ITERATIONS 80
#define K_MEANS_CONVERGENCE_THRESHOLD 0.005
#define TARGET_SIZE 16 // K

// --- Constantes sRGB ---
#define SRGB_THRESHOLD 0.04045f
#define SRGB_POWER 2.4f
#define LINEAR_THRESHOLD 0.0031308f
#define INV_POWER (1.0f / 2.4f)


// float_to_byte
int float_to_byte(float float_value) {
    // 1. Limiter la valeur
    float clamped_float = fmaxf(0.0f, fminf(1.0f, float_value));
    // 2. Mettre à l'échelle, arrondir (avec +0.5) et convertir en int
    return (int)floorf(clamped_float * 255.0f + 0.5f);
}

// byte_to_float
float byte_to_float(int byte_value) {
    // 1. Limiter la valeur
    int clamped_byte = (int)fmax(0, fmin(255, (float)byte_value));
    // 2. Normaliser
    return (float)clamped_byte / 255.0f;
}

// srgb_to_linear
ColorKM srgb_to_linear_km(ColorKM pixel) {
    ColorKM c;
    
    // R
    if (pixel.r > SRGB_THRESHOLD) {
        c.r = powf((pixel.r + 0.055f) / 1.055f, SRGB_POWER);
    } else {
        c.r = pixel.r / 12.92f;
    }

    // G
    if (pixel.g > SRGB_THRESHOLD) {
        c.g = powf((pixel.g + 0.055f) / 1.055f, SRGB_POWER);
    } else {
        c.g = pixel.g / 12.92f;
    }

    // B
    if (pixel.b > SRGB_THRESHOLD) {
        c.b = powf((pixel.b + 0.055f) / 1.055f, SRGB_POWER);
    } else {
        c.b = pixel.b / 12.92f;
    }
    return c;
}

double distance_between_colors_custom(ColorKM c1, ColorKM c2) {
    float dr = c1.r - c2.r;
    float dg = c1.g - c2.g;
    float db = c1.b - c2.b;
    const double c = 1.8; // L'exposant

    double term_r = pow(fabs(dr) * 8.0, c);
    double term_g = pow(fabs(dg) * 11.0, c);
    double term_b = pow(fabs(db) * 8.0, c);

    return term_r + term_g + term_b;
}

// byte_to_linear (Version C de la fonction Lua)
ColorKM byte_to_linear(ColorByte color_byte) {
    ColorKM srgb_norm = {
        .r = (float)color_byte.r / 255.0f,
        .g = (float)color_byte.g / 255.0f,
        .b = (float)color_byte.b / 255.0f
    };
    return srgb_to_linear_km(srgb_norm);
}


// kmeans_quantize
// @param weighted_pixels: Tableau de structures WeightedPixel.
// @param N_UNIQUE: Nombre d'éléments dans le tableau.
// @param K: Taille de la palette cible.
// @return: Tableau de K structures ColorByte (la palette finale).
ColorByte* kmeans_quantize(WeightedPixel *weighted_pixels, int N_UNIQUE, int K) {
    if (N_UNIQUE == 0 || K <= 0) return NULL;

    // Allocation des tableaux principaux
    ColorByte *centroids = (ColorByte*)malloc(K * sizeof(ColorByte));
    int *assignments = (int*)malloc(N_UNIQUE * sizeof(int));
    
    if (!centroids || !assignments) { /* Gestion erreur mémoire */ return NULL; }

    // 1. Initialisation des K centroïdes (simple: K couleurs espacées)
    for (int k = 0; k < K; k++) { // k est 0-based
        int index = (int)floor(((double)N_UNIQUE * (double)k / (double)K));
        if (index >= N_UNIQUE) index = N_UNIQUE - 1;
        
        centroids[k] = weighted_pixels[index].color;
    }

    int has_converged = 0;
    int iteration = 0;
    double total_pixels = 0;
    for (int i = 0; i < N_UNIQUE; i++) { total_pixels += weighted_pixels[i].weight; }
    
    // Buffers pour le calcul itératif
    CentroidData *new_centroids_data = (CentroidData*)malloc(K * sizeof(CentroidData));
    ColorKM *linear_centroids = (ColorKM*)malloc(K * sizeof(ColorKM));
    ColorByte *old_centroids = (ColorByte*)malloc(K * sizeof(ColorByte));

    if (!new_centroids_data || !linear_centroids || !old_centroids) { /* Gestion erreur mémoire */ return NULL; }
    
    // Boucle d'itération
    while (!has_converged && iteration < MAX_ITERATIONS) {
        iteration++;
        int changes = 0;
        
        // Initialisation de new_centroids_data et sauvegarde des anciens centroïdes
        for (int k = 0; k < K; k++) {
            memset(&new_centroids_data[k], 0, sizeof(CentroidData));
            old_centroids[k] = centroids[k];
            linear_centroids[k] = byte_to_linear(centroids[k]); // Pré-calcul Linéaire
        }

        // Étape 1 : Affectation (Assigner chaque pixel pondéré au centroïde le plus proche)
        for (int i = 0; i < N_UNIQUE; i++) {
            WeightedPixel entry = weighted_pixels[i];
            ColorByte pixel_color = entry.color;
            int weight = entry.weight;
            
            ColorKM pixel_linear = byte_to_linear(pixel_color);
            
            double min_distance = MAXFLOAT; // Utilisez DBL_MAX pour la distance
            int closest_centroid_index = 0; // Index 0-based
            
            for (int k = 0; k < K; k++) {
                double d = distance_between_colors_custom(pixel_linear, linear_centroids[k]);
                
                if (d < min_distance) {
                    min_distance = d;
                    closest_centroid_index = k;
                }
            }
            
            // Vérification des changements
            if (assignments[i] != closest_centroid_index) {
                changes++;
                assignments[i] = closest_centroid_index;
            }

            // Mise à jour des sommes PONDÉRÉES
            CentroidData *data = &new_centroids_data[closest_centroid_index];
            data->r_sum += (double)pixel_color.r * (double)weight;
            data->g_sum += (double)pixel_color.g * (double)weight;
            data->b_sum += (double)pixel_color.b * (double)weight;
            data->total_weight += (double)weight;
        }
        
        // Étape 2 : Mise à jour (Calculer les NOUVEAUX centroïdes PONDÉRÉS)
        for (int k = 0; k < K; k++) {
            CentroidData data = new_centroids_data[k];
            if (data.total_weight > 0.0) {
                centroids[k].r = (int)floor(data.r_sum / data.total_weight + 0.5);
                centroids[k].g = (int)floor(data.g_sum / data.total_weight + 0.5);
                centroids[k].b = (int)floor(data.b_sum / data.total_weight + 0.5);
            } else {
                // Cluster vide, on garde l'ancien centroïde
                centroids[k] = old_centroids[k];
            }
        }

        // Condition de convergence
        if ((double)changes / total_pixels < K_MEANS_CONVERGENCE_THRESHOLD) {
            has_converged = 1;
        }

        // Simulation de statusmessage
        // printf("K-Means: Itération %d. Convergence: %.2f%% de changement.\n", 
        //         iteration, (double)changes / total_pixels * 100.0);
    }
    
    // Libération de la mémoire temporaire
    free(assignments);
    free(new_centroids_data);
    free(linear_centroids);
    free(old_centroids);

    return centroids; // La palette finale est retournée (doit être libérée par l'appelant)
}