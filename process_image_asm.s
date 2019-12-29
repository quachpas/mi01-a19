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
            push 	r13
			push 	r14
			push 	r15
            mov     rbp, rsp

            /***********************************************************
              Ajoutez votre code ici

             **********************************************************/
			mov 	r12, rdi
			imul 	r12, rsi /* Compteur */
			sub 	r12, 1

			/* r10, r12 registre utilisé*/
    boucle:
/* R10 = RCX image temp1; R11 = img_temp2 INIT (2,2) */

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

			jmp		fin
			/* --------------------------------------------------------------------------------- */
			/* TP6 R10 ET R12 NE SONT PLUS UTILES. */
			/* R10 = RCX image temp1; R11 = img_temp2 INIT (2,2) */
			/* R12 : compteur de lignes restantes */
			/* R13 : compteur de colonnes restantes */
			/* R14 : registre de stockage calcul Gx */
			/* R15 : registre de stockage calcul Gy */

			lea 	r10, [rcx] /* sauvegarde l'adresse du premier pixel de img_temp*/
			lea 	r11, [r8+rdi*4+4] /* adresse du premier pixel où stocker G (2,2) */
			mov 	r12, rsi /* hauteur = nb lignes */
			sub 	r12, 2 /* i-2, nombre de lignes restantes */
			mov 	r13, rdi /* largeur = nb colonnes */
			sub 	r13, 2 /* j-2, nombre de colonnes restantes */

			/* itération ligne i */
boucle_ligne:

				/* itération colonne j */
boucle_colonne:
				/* TRAITEMENT DE L'IMAGE */
			/* mov 	dword ptr [r10], 0xFF00FF00 */
			/* mov 	dword ptr [r11], 0xFF00FF00 */

			/* Calcul de Gx */
			/* on récupère le pixel rouge = intensité de l'img source */
			movzx 	r14w, byte ptr [r10+8] /* + 1*a13 */

			movzx 	ax, byte ptr [r10+rdi*4+8]
			add 	r14w, ax
			add 	r14w, ax /* +2*a23 */

			movzx 	ax, byte ptr [r10+rdi*8+8]
			add 	r14w, ax /* + a33 */

			movzx 	ax, byte ptr [r10]
			sub		r14w, ax /* -1 a11 */

			movzx 	ax, byte ptr [r10+rdi*4]
			sub 	r14w, ax
			sub		r14w, ax  /* -2*a21 */


			movzx	ax, byte ptr [r10+rdi*8]
			sub  	r14w, ax  /* -1 a31 */


			/* Calcul de Gy */
			movzx 	r15w, byte ptr [r10] /* + 1*a11 */

			movzx 	ax, byte ptr [r10+4]
			add 	r15w, ax
			add 	r15w, ax /* +2*a12 */

			movzx 	ax, byte ptr [r10+8]
			add 	r15w, ax /* + a13 */

			movzx 	ax, byte ptr [r10+rdi*8]
			sub		r15w, ax /* -1 a31 */

			movzx 	ax, byte ptr [r10+rdi*8+4]
			sub 	r15w, ax
			sub 	r15w, ax  /* -2*a32 */

			movzx	ax, byte ptr [r10+rdi*8+8]
			sub  	r15w, ax  /* -1 a33 */

			/* Calcul de G */
				/* abs(Gx) */
			mov 	ax, r14w /* sauvegarde */
			neg 	r14w
			cmovl 	r14w, ax
				/* abs(Gy) */
			mov 	ax, r15w /* sauvegarde */
			neg 	r15w
			cmovl 	r15w, ax
				/* G */
			add 	r14w, r15w /* On fait la somme  G = abs(Gx) + abs(Gy) */
			mov 	r15w, 0x00FF /* La somme peut être > 255 */
			mov 	ax, 0 /* Donc, on va vérifier si G < 0 */
			sub 	r15w, r14w /* Si < 0, on le met à 0 => NOIR */
			cmovs 	r15w, ax /* On déplace 0 */

			/* Déplacement de G dans img_temp2 */
			mov 	[r11], r15b
			mov 	[r11+1], r15b
			mov 	[r11+2], r15b
			

				/* TRAITEMENT DE L'IMAGE */

			add 	r10, 4 /* incrémentation des deux pointeurs sur les pixels source/destinations de 4 */
			add 	r11, 4 /* => nouvelle colonne */
			sub 	r13, 1 /* décrémentation j */
			jnz 	boucle_colonne
				/* iteration colonne j */
			add 	r10, 8 /* incrémentation des deux pointeurs sur les pixels source/destinations de 8 */
			add 	r11, 8 /* => passage de ligne */
			mov 	r13, rdi /* réinitialisation du compteur j, nbr de colonnes restantes */
			sub 	r13, 2	
			sub 	r12, 1 /* décrémentation i */
			jnz 	boucle_ligne
			/* iteration ligne i*/
fin:
			/* Fin de notre code */
			pop 	r15
			pop 	r14
			pop 	r13
			pop 	r12
			pop 	rbx
            pop     rbp
            ret
