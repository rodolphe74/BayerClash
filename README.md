# BayerClash

Trammage ordonné avec une palette de 16 couleurs non régulièrement espacées avec contrainte color-clash des ordinateurs Thomson MO5 MO6 de 2 couleurs max par bloc de 8 pixels horizontaux.

Sur MO6, recherche d'une palette de 16 couleurs optimale sur 4096.

<div align="center">
  <table>
    <tr><td><img src="results/palette_mo6.png"></td><td><img src="results/mini.png" width=338></td></tr>
  </table>
</div>

Plusieurs modes sont disponibles :

```C
// Choix de la machine
// #define MO5
#define MO6

// Choix du dithering
// #define BLUE_NOISE
// #define BAYER_4
// #define BAYER_4_LOW
#define BAYER_8
// #define R_SEQUENCE

// Choix de l'algorithme 
// (KNOLL est plus adapté au color clash)
// #define TETRAPAL
// #define N_CANDIDATES 4
#define KNOLL
#define N_CANDIDATES 32
```

# Exemples



# Liens
- [UToPic](https://github.com/Samuel-DEVULDER/UToPiC)
- [GrafX2](https://grafx2.gitlab.io/grafX2)
- [libimagequant](https://pngquant.org/lib/)
- [tetrapal](https://github.com/matejlou/tetrapal)
- [exoquant](https://github.com/exoticorn/exoquant)
- [C containers](https://github.com/bkthomps/Containers)
- [stb](https://github.com/nothings/stb)
