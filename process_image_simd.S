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
            mov     rbp, rsp

            /***********************************************************
              Ajoutez votre code ici
            ***********************************************************/

            pop     rbp
            ret
