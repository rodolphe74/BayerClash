#include "imquantize.h"
#include <stdio.h>
#include <stdlib.h>

void quantize(unsigned char *input_image, int width, int height, int comps, unsigned char *out_palette,
			  int palette_size)
{
   if (comps != 4) return;
    
    liq_attr *attr = liq_attr_create();
    if (!attr) return; 

    // 1. Réglages clés : Vitesse et Qualité
    
    // Vitesse 1 : La plus lente, mais maximise la qualité de la palette trouvée
    liq_set_speed(attr, 1); 

    // Qualité 0 : Assure que la quantification réussit toujours (LIQ_OK)
    liq_set_quality(attr, 0, 100); 

    // Définir le nombre de couleurs désiré (e.g., 16)
    liq_set_max_colors(attr, palette_size);

    // [ Optionnel : Si vous devez garantir une couleur transparente ]
    // liq_set_last_index_transparent(attr, 1); 

    // 2. Création de l'image
    liq_image *liq_input = liq_image_create_rgba(attr, input_image, width, height, 0);
    if (!liq_input) { liq_attr_destroy(attr); return; }

    // 3. Quantification
    liq_result *quant_result;
    liq_error err = liq_image_quantize(liq_input, attr, &quant_result);

    if (err != LIQ_OK) {
        fprintf(stderr, "Erreur de quantification (Code: %d). \n", err);
        liq_image_destroy(liq_input);
        liq_attr_destroy(attr);
        return;
    }
    
    // 4. Extraction de la Palette
    const liq_palette *palette = liq_get_palette(quant_result);
    int actual_colors = palette->count;

    for (int i = 0; i < actual_colors; i++) {
        const liq_color color = palette->entries[i];
        
        out_palette[i * 4 + 0] = color.r;
        out_palette[i * 4 + 1] = color.g;
        out_palette[i * 4 + 2] = color.b;
        out_palette[i * 4 + 3] = color.a;
    }
    
    // 5. Nettoyage
    liq_result_destroy(quant_result);
    liq_image_destroy(liq_input);
	liq_attr_destroy(attr);
}