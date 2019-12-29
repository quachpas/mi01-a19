/*  process_image_asm.S
 
    MI01 - TP Assembleur 2 à 5

    Réalise le traitement d'une image bitmap 32 bits par pixel.
*/

.file "process_image_asm.S"
.intel_syntax noprefix

.text

/***********************************************************************
  Sous-programme process_image_asm 
    
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

.global process_image_asm
process_image_asm:
            push    rbp
            push 	rbx
            push 	r12
            mov     rbp, rsp

            /***********************************************************
              Ajoutez votre code ici

             **********************************************************/
			mov 	r12, rdi
			imul 	r12, rsi /* Compteur */

    boucle:
    		sub 	r12, 1
    		mov 	ebx, [rdx+r12*4] /* pixel de l'image */

    		/* Calculer l'intensité */
    		/* Stocker dans img_temp1 */

    		/* mov 	word ptr [rcx+r12*4-4], 0xFFFF0000 Couleur unie */



    		/* Multiplication R*0.21 */
    		mov 	al, 0x36 /* Constante rouge */
    		mul 	bl
    		mov 	r8w, ax

    		/* Shift à droite rbx pour récupérer le pixel vert */
    		shr 	ebx, 8

    		/* Multiplication V*0.72 */
    		mov 	al, 0xB7 /* Constante verte */
    		mul 	bl
    		add 	r8w, ax

    		/* Shift à droite rbx pour récupérer le pixel rouge */
    		shr 	ebx, 8

    		/* Multiplication B*0.07 */
    		mov 	al, 0x12 /* Constante bleu */
    		mul 	bl
    		add 	r8w, ax


			/* Mettre l'intensité dans img_temp1 */
			shr 	r8w, 8
			mov 	byte ptr [rcx+r12*4], r8b /* On récupère la partie haute */

    		cmp 	r12, 0
			ja 		boucle /* Si rax est à 0 on ŝ'arrête */


			/* Fin de notre code */
			pop 	r12
			pop 	rbx
            pop     rbp
            ret
