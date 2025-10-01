# BayerClash

**Conversion C du plugin LUA https://github.com/Samuel-DEVULDER/UToPiC/blob/master/bayer4_mo5.lua**

<img src="result/original.png" width=320>&nbsp;<img src="result/output_mo5.png" width=320>

**Expérimentation de tramage ordonné avec une palette de 16 couleurs non regulièrement espacées.**

Avec les contraintes MO5 de 2 couleurs max par bloc horizontal de 8 pixels sur une palette de 16 couleurs:

Avec une matrice 8x8 standard :

<img src="result/output_mo5_t.png" width=320>

Avec une matrice [blue noise](https://github.com/matejlou/SimpleBlueNoise) :

<img src="result/output_mo5_t2.png" width=320>


Avec les contraintes MO6 de 2 couleurs max par bloc horizontal de 8 pixels sur une palette de 4096 couleurs:

Avec une matrice 8x8 standard :

<img src="result/output_mo6.png" width=320>

Avec une matrice [blue noise](https://github.com/matejlou/SimpleBlueNoise) :

<img src="result/output_mo6_bn.png" width=320>

# Liens
- [UToPic](https://github.com/Samuel-DEVULDER/UToPiC)
- [tetrapal](https://github.com/matejlou/tetrapal)
- [exoquant](https://github.com/exoticorn/exoquant)
- [C containers](https://github.com/bkthomps/Containers)
