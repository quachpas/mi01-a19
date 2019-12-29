/*  process_image_simd.S
 
    MI01 - TP Assembleur 2 à 5

    Réalise le traitement d'une image bitmap 32 bits par pixel.
    Implémentation SIMD
*/

.file "process_image_simd.S"
.intel_syntax noprefix

.text

/***********************************************************************
  Sous-programme process_image_simd
    
  Réalise le traitement d'une image 32 bits.
  
  Le passage des paramètres respecte les conventions x64 sous Linux
  
  Par registres :
            Largeur de l'image : rdi
            Hauteur de l'image : rsi
            Pointeur sur l'image source : rdx
            Pointeur sur l'image tampon 1 : rcx
            Pointeur sur l'image tampon 2 : r8
            Pointeur sur l'image finale : r9  
  
  Les registes rbx, rbp, r12, r13, r14, et r15 doivent être sauvegardés
  si vous les utilisez (sauvegarde par l'appelé). Les autres registres 
  peuvent être modifiés sans risque (sauvegardés par l'appelant).
***********************************************************************/

.global process_image_simd
process_image_simd:  
            push    rbp
            push 	r12
            push 	rbx
            mov     rbp, rsp

            /***********************************************************
              Ajoutez votre code ici
            ***********************************************************/
			mov 	r12, rdi /* Largeur de l'image */
			imul 	r12, rsi /* Hauteur de l'image */
			sub 	r12, 1	/* Compteur de boucle */
			/* b = 8 bits */
			/* w = 16 bits */
			/* d = 32 bits */
			/* q = 64 bits */
			/* dq = 128 bits */

			/* Construction du vecteur de constantes 64 bits := 4*16bits */
			mov 	ebx, 0x0012
			movw 	xmm0, rbx
			pslldq 	xmmo0, 2
			mov 	ebx, 0x00B7
			movw 	xmm0, rbx
			pslldq 	xmmo0, 2
			mov 	ebx, 0x0036
			movw 	xmm0, rbx
			/* Fin Construction vecteur constantes */


boucle:
			/* CALCUL INTENSITE */

			/* Chargement du vecteur de deux pixels complets */
			movd 	xmm1, [rdx+r12*4] /* Pixel 1 */
			pslldq 	xmm1, 4			/* décalage 32 bits gauche */
			movd 	xmm1, [rdx+r12*4+4] /* Pixel 2 */
			/* Fin Chargement */

			/* Transformation vecteur chargé en deux vecteurs de mots de 16 */
			pxor 	xmm2, xmm2 /* Préparation de deux vecteurs nuls */
			pxor 	xmm3, xmm3
			punpcklbw	xmm2, xmm1 /* Déplacement du pixel 2 */
			punpckhbw 	xmm3, xmm1 /* Déplacement du pixel 1 */
			/* Fin Transformation */

			/* Produit scalaire entre vecteur constante et Pixel 1 */
			/* xmm1 buffer */
			pmaddwd xmm2, xmm0 /* On additionne et multiplie => mots de 32 bits */
			movd 	xmm1, xmm2 /* copie du résultat */
			pslrdq 	xmm1, 4 	/* décalage pour récupérer C(B1)*B1 */
			paddd 	xmm2, xmm1 /* résultat final sur 32 */
			pslrdq 	xmm2, 2 	/* décalage de 16 bits pour récupérer partie entière */
			/* Fin produit scalaire 1 */

			/* Produit scalaire entre vecteur constante et Pixel 2 */
			/* xmm1 buffer */
			pmaddwd xmm3, xmm0 /* On additionne et multiplie => mots de 32 bits */
			movd 	xmm1, xmm2 /* copie du résultat */
			pslrdq 	xmm1, 4 	/* décalage pour récupérer C(B2)*B2 */
			paddd 	xmm3, xmm1 /* résultat final sur 32 */
			pslrdq 	xmm3, 2 	/* décalage de 16 bits pour récupérer partie entière */
			/* Fin produit scalaire 2 */

			/* Canal alpha */
			pcmpeqq xmm1, xmm1 /* On crée un vecteur de 1s */
			pslrd  	xmm1, 6 /* décalage de 6 octets 0x00000011*/
			pand 	xmm2, xmm1 	/* on met à 0 tous les autres pixels sauf I */
			pand 	xmm3, xmm1 	/* on met à 0 tous les autres pixels sauf I */
			pxor 	xmm1, xmm1 /* remise à zéro du vecteur buffer */
			mov 	ebx, 0xff
			movb 	xmm1, ebx /* On charge Oxff dans xmm1 */
			psllqd 	xmm1, 3 /* décalage 24 bits pour le mettre en première position */
			paddb 	xmm2, xmm1
			paddb 	xmm3, xmm1 /* on rajoute le canal alpha dans les pixels */
			/* Fin Canal Alpha */

			/* Déplacement mémoire */
			movw 	ebx, xmm3 /* déplacement pixel 1*/
			shrl 	rbx, 32 /* shift left pour libérer l'espace pour pixel 2 */
			movw 	ebx, xmm2 /* déplacement pixel 2 */
			mov 	[rcx+r12], rbx

			/* Fin déplacement mémoire */
			/* CALCUL INTENSITE */

			sub 	r12, 2
			jns 	boucle /* Plus de pixels à traiter */
			/* Fin BOUCLE */

			pop 	rbx
			pop 	r12
            pop     rbp
            ret
