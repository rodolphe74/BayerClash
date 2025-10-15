#include "gx2reduce.h"

dword Round_div(dword numerator, dword divisor)
{
	return numerator / divisor;
}

void Reduce_palette(short *used_colors, int nb_colors_asked, T_Palette palette, dword *color_usage)
{
	char str[5];				// buffer d'affichage du compteur
	byte conversion_table[256]; // Table de conversion
	int color_1;				// |_ Variables de balayages
	int color_2;				// |  de la palette
	int best_color_1 = 0;
	int best_color_2 = 0;
	long best_difference;
	dword used;
	dword best_used;

	// On commence par initialiser la table de conversion dans un état où
	// aucune conversion ne sera effectuée.
	for (color_1 = 0; color_1 <= 255; color_1++) conversion_table[color_1] = color_1;

	//   Si on ne connait pas encore le nombre de couleurs utilisées, on le
	// calcule! (!!! La fonction appelée Efface puis Affiche le curseur !!!)
	// if ((*used_colors) < 0) Update_color_count(used_colors, color_usage);

	//   On tasse la palette vers le début parce qu'elle doit ressembler à
	// du Gruyère (et comme Papouille il aime pas le fromage...)

	// Pour cela, on va scruter la couleur color_1 et se servir de l'indice
	// color_2 comme position de destination.
	for (color_1 = color_2 = 0; color_1 <= 255; color_1++) {
		if (color_usage[color_1]) {
			// On commence par s'occuper des teintes de la palette
			palette[color_2].R = palette[color_1].R;
			palette[color_2].G = palette[color_1].G;
			palette[color_2].B = palette[color_1].B;

			// Ensuite, on met à jour le tableau d'occupation des couleurs.
			color_usage[color_2] = color_usage[color_1];

			// On va maintenant s'occuper de la table de conversion:
			conversion_table[color_1] = color_2;

			// Maintenant, la place désignée par color_2 est occupée, alors on
			// doit passer à un indice de destination suivant.
			color_2++;
		}
	}

	// On met toutes les couleurs inutilisées en noir
	for (; color_2 < 256; color_2++) {
		palette[color_2].R = 0;
		palette[color_2].G = 0;
		palette[color_2].B = 0;
		color_usage[color_2] = 0;
	}

	// Maintenant qu'on a une palette clean, on va boucler en réduisant
	// le nombre de couleurs jusqu'à ce qu'on atteigne le nombre désiré.
	// (The stop condition is further down)
	while (1) {
		//   Il s'agit de trouver les 2 couleurs qui se ressemblent le plus
		// parmis celles qui sont utilisées (bien sûr) et de les remplacer par
		// une seule couleur qui est la moyenne pondérée de ces 2 couleurs
		// en fonction de leur utilisation dans l'image.

		best_difference = 26 * 255 * 26 * 255 + 55 * 255 * 255 + 19 * 255 * 19 * 255;
		best_used = 0x7FFFFFFF;

		for (color_1 = 0; color_1 < (*used_colors); color_1++)
			for (color_2 = color_1 + 1; color_2 < (*used_colors); color_2++)
				if (color_1 != color_2) {
					long dr, dg, db;
					long difference; // could also be called distance (or distance square)

					dr = (long)palette[color_1].R - (long)palette[color_2].R;
					dg = (long)palette[color_1].G - (long)palette[color_2].G;
					db = (long)palette[color_1].B - (long)palette[color_2].B;
					difference = 26 * 26 * dr * dr + 55 * 55 * dg * dg + 19 * 19 * db * db;

					if (difference <= best_difference) {
						used = color_usage[color_1] + color_usage[color_2];
						if ((difference < best_difference) || (used < best_used)) {
							best_difference = difference;
							best_used = used;
							best_color_1 = color_1;
							best_color_2 = color_2;
						}
					}
				}

		// Stop condition: when no more duplicates exist
		// and the number of colors has reached the target.
		if (best_difference != 0 && (*used_colors) <= nb_colors_asked) break;

		//   Maintenant qu'on les a trouvées, on va pouvoir mettre à jour nos
		// données pour que le remplacement se fasse sans encombres.

		// En somme, on va remplacer best_color_2 par best_color_1,
		// mais attention, on ne remplace pas best_color_1 par
		// best_color_2 !

		// On met à jour la palette.
		palette[best_color_1].R = Round_div((color_usage[best_color_1] * palette[best_color_1].R) +
												(color_usage[best_color_2] * palette[best_color_2].R),
											best_used);
		palette[best_color_1].G = Round_div((color_usage[best_color_1] * palette[best_color_1].G) +
												(color_usage[best_color_2] * palette[best_color_2].G),
											best_used);
		palette[best_color_1].B = Round_div((color_usage[best_color_1] * palette[best_color_1].B) +
												(color_usage[best_color_2] * palette[best_color_2].B),
											best_used);

		// On met à jour la table d'utilisation.
		color_usage[best_color_1] += color_usage[best_color_2];
		color_usage[best_color_2] = 0;

		// On met à jour la table de conversion.
		for (color_1 = 0; color_1 <= 255; color_1++) {
			if (conversion_table[color_1] == best_color_2) {
				//   La color_1 avait déjà prévue de se faire remplacer par la
				// couleur que l'on veut maintenant éliminer. On va maintenant
				// demander à ce que la color_1 se fasse remplacer par la
				// best_color_1.
				conversion_table[color_1] = best_color_1;
			}
		}

		//   Bon, maintenant que l'on a fait bouger nos petites choses concernants
		// la couleur à éliminer, on va s'occuper de faire bouger les couleurs
		// situées après la couleur à éliminer pour qu'elles se déplaçent d'une
		// couleur en arrière.
		for (color_1 = 0; color_1 <= 255; color_1++) {
			//   Commençons par nous occuper des tables d'utilisation et de la
			// palette.

			if (color_1 > best_color_2) {
				// La color_1 va scroller en arrière.

				//   Donc on transfère son utilisation dans l'utilisation de la
				// couleur qui la précède.
				color_usage[color_1 - 1] = color_usage[color_1];

				//   Et on transfère ses teintes dans les teintes de la couleur qui
				// la précède.
				palette[color_1 - 1].R = palette[color_1].R;
				palette[color_1 - 1].G = palette[color_1].G;
				palette[color_1 - 1].B = palette[color_1].B;
			}

			//   Une fois la palette et la table d'utilisation gérées, on peut
			// s'occuper de notre table de conversion.
			if (conversion_table[color_1] > best_color_2)
				//   La color_1 avait l'intention de se faire remplacer par une
				// couleur que l'on va (ou que l'on a déjà) bouger en arrière.
				conversion_table[color_1]--;
		}

		//   On vient d'éjecter une couleur, donc on peut mettre à jour le nombre
		// de couleurs utilisées.
		(*used_colors)--;

		// A la fin, on doit passer (dans la palette) les teintes du dernier
		// élément de notre ensemble en noir.
		palette[*used_colors].R = 0;
		palette[*used_colors].G = 0;
		palette[*used_colors].B = 0;

		// Au passage, on va s'assurer que l'on a pas oublié de la mettre à une
		// utilisation nulle.
		color_usage[*used_colors] = 0;
	}
}
