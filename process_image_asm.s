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
			sub 	r12, 1

    boucle:


    		/* Calculer l'intensité */
    		/* Stocker dans img_temp1 */

    		/* mov 	word ptr [rcx+r12*4-4], 0xFFFF0000 Couleur unie */

    		/* Multiplication R*0.21 */
    		mov 	al, 0x36
    		mul 	byte ptr [rdx+r12*4]
    		shr 	ax, 8
    		mov 	r10b, al

    		/* Multiplication V*0.72 */
    		mov 	al, 0xB7
    		mul 	byte ptr [rdx+r12*4+1]
    		shr 	ax, 8
    		add 	r10b, al

    		/* Multiplication B*0.07 */
    		mov 	al, 0x12
    		mul 	byte ptr [rdx+r12*4+2]
    		shr 	ax, 8
    		add 	r10b, al

			/* Mettre l'intensité dans img_temp1 */

			mov 	byte ptr [rcx+r12*4], r10b

			sub 	r12, 1
			jns 	boucle /* Si rax est à 0 on ŝ'arrête */


			/* Fin de notre code */
			pop 	r12
			pop 	rbx
            pop     rbp
            ret
