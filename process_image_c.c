/*  Atelier Photo - Travaux Pratiques UV MI01
 Copyright (C) 2019 S. Bonnet, Université de Technologie de Compiègne

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdlib.h>
#include <stdint.h>

/* process_image_c
 *
 * Implémentation en C de l'algorithme de traitement
 *
 */
void process_image_c(uint32_t img_width, uint32_t img_height,
        uint8_t *img_src, uint8_t *img_temp1, uint8_t *img_temp2, uint8_t *img_dest)
{
    unsigned int c, n;
    unsigned int x, y;
    int sum;

    /* Conversion en niveaux d'intensité */
    n = img_width * img_height * 4;

    for (c = 0; c < n; c = c + 4) {
        img_temp1[c] = (uint8_t)((((int)img_src[c + 2]) * 0x13
                    + ((int)img_src[c + 1] * 0xb7)
                    + ((int)img_src[c] * 0x36)) >> 8);
        img_temp1[c + 3] = 0xff;
    }
 
    /* Supprimez la ligne ci-dessous pour activer le détecteur de contours */
    return;

    /* Détecteur de contours de Sobel */
    n = img_width * 4;

    for (y = 0; y < img_height - 2; y++) {
        for (x = 0; x < (img_width - 2) * 4; x = x + 4) {
            sum = 255
                - (abs(-(int)((*(img_temp1 + (x + y * n))))
                    + (int)((*(img_temp1 + (x + 8 + y * n))))
                    - 2 * ((int)((*(img_temp1 + (x + (y + 1)* n)))))
                    + 2 * ((int)((*(img_temp1 + (x + 8 + (y + 1) * n)))))
                    - ((int)((*(img_temp1 + (x + (y + 2) * n)))))
                    + ((int)((*(img_temp1 + (x + 8 + (y + 2) * n))))))
                + abs((int)((*(img_temp1 + (x + y * n))))
                    + 2 * (int)((*(img_temp1 + (x + 4 + y * n))))
                    + (int)((*(img_temp1 + (x + 8 + y *n))))
                    - (int)((*(img_temp1 + (x + (y + 2) * n))))
                    - 2 * (int)((*(img_temp1 + (x + 4 + (y + 2) * n))))
                    - (int)((*(img_temp1 + (x + 8 + (y + 2) * n))))));

            if (sum < 0) {
                sum = 0;
            }

            *((int *)(img_temp2 + (x + 4 + (y + 1) * n))) = sum | (sum << 8) | (sum << 16) | 0xff000000;
        }
    }

    return;

    /* Seuillage de l'image */
    n = img_width * img_height * 4;

    for (c = 0; c < n; c = c + 4) {
        if (img_temp2[c] > 150) {
            *((int *)(img_dest + c)) = 0xffffffff;
        }
        else {
            *((int *)(img_dest + c)) = 0xff000000;
        }
    }
}
