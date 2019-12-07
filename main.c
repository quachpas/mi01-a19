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
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "process_functions.h"

/*
 * Variables globales
 */

/* Nom de l'application */
const gchar application_name[] = "Atelier photo";

/* Pointeur vers la fenêtre pricipale de l'application */
GtkWidget *main_window;

/* Pointeur vers le widget d'affichage de l'image */
GtkWidget *image;

/* Buffers d'image */
#define N_BITMAPS   (4)

#define IMG_SRC     (0)
#define IMG_TMP1    (1)
#define IMG_TMP2    (2)
#define IMG_DEST    (3)

typedef struct _bitmap {
    gchar *name;
    GdkPixbuf *pixbuf;
} bitmap_t;

bitmap_t bitmaps[] = {
    { "Image source", NULL },
    { "Image temporaire 1", NULL },
    { "Image temporaire 2", NULL },
    { "Image finale", NULL },
    { NULL, NULL }
};

/* Nombre de répétions à réaliser */
int process_repetitions = 1;

/* Map des fonctions de traitement */


typedef struct _process_task {
    gchar *target;
    void (*process_fun)(uint32_t, uint32_t, uint8_t *, uint8_t *, uint8_t *, uint8_t *); 
} process_task_t;

process_task_t process_tasks[] = {
    { "c", process_image_c },
    { "asm", process_image_asm },
    { "simd", process_image_simd },
    { NULL, NULL  }
};


/*
 * Boîtes de dialogue
 */

/* file_chooser 
 *
 * Sélection d'un fichier bitmap.
 * 
 * Retourne le nom du fichier ou NULL.
 * La chaine retournée doit être libérée avec g_free.
 * 
 */
gchar* file_chooser() {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choisissez une image",
                            GTK_WINDOW(main_window),
                            GTK_FILE_CHOOSER_ACTION_OPEN,
                            "Ouvrir",
                            GTK_RESPONSE_ACCEPT,
                            "Annuler",
                            GTK_RESPONSE_CANCEL,
                            NULL);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), ".");

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_mime_type(filter, "image/bmp");
    gtk_file_filter_add_mime_type(filter, "image/jpeg");
    gtk_file_filter_add_mime_type(filter, "image/png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar *filename = NULL;
    if (response == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    }

    gtk_widget_destroy(dialog);
    return filename;
}

/* message_dialog
 *
 * Affiche un message simple.
 * 
 */
void message_dialog(const gchar *title, GtkMessageType type, const gchar *format, ...) 
{
    va_list ap;

    va_start(ap, format);

    gchar *message;
    g_vasprintf(&message, format, ap);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            type,
                            GTK_BUTTONS_CLOSE,
                            "%s",
                            message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
    g_free(message);
}

/* repetitions_dialog
 *
 * Saisie du nombre de répétitions à effectuer.
 * 
 */
void repetitions_dialog() 
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Répétitions",
                            GTK_WINDOW(main_window),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            "Ok",
                            GTK_RESPONSE_ACCEPT,
                            "Annuler",
                            GTK_RESPONSE_CANCEL,
                            NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    
    GtkWidget *label = gtk_label_new("Nombre de répétitions :");

    GtkWidget *entry = gtk_entry_new();
    gchar buffer[12];
    g_snprintf(buffer, 12, "%d", process_repetitions);

    gtk_entry_set_text(GTK_ENTRY(entry), buffer);
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), entry, label, GTK_POS_RIGHT, 1, 1);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        process_repetitions = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
        if (process_repetitions <= 0) {
            process_repetitions = 1;
        }
    }
    gtk_widget_destroy(dialog);
}

/*
 * Gestion des images
 */


/* load_image
 *
 * Charge une image. Retourne le pixbuf associé ou NULL en cas d'erreur
 * 
 */
GdkPixbuf *load_source_image(const gchar *filename)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, &error);

    if (error != NULL) {
        message_dialog("Erreur", GTK_MESSAGE_ERROR, "%s", error->message);
        return NULL;
    } else {
        /* Ajouter le canal alpha s'il n'est pas présent dans l'image */
        GdkPixbuf *pixbuf_with_alpha = gdk_pixbuf_add_alpha(pixbuf, FALSE, 0, 0, 0);
        g_object_unref(pixbuf);
        return pixbuf_with_alpha;
    }
}


/* setup_image
 *
 * Crée les bitmaps de travail en fonction de l'image source
 * 
 */
void setup_images(GdkPixbuf *source_image)
{
    if (bitmaps[0].pixbuf != NULL) {
        g_object_unref(bitmaps[IMG_SRC].pixbuf);
    }

    bitmaps[IMG_SRC].pixbuf = source_image;

    GdkColorspace cs = gdk_pixbuf_get_colorspace(source_image);
    int bps = gdk_pixbuf_get_bits_per_sample(source_image);
    int w = gdk_pixbuf_get_width(source_image);
    int h = gdk_pixbuf_get_height(source_image);

    int i = 1;
    while (bitmaps[i].name != NULL) {
        if (bitmaps[i].pixbuf != NULL) {
            g_object_unref(bitmaps[i].pixbuf);
        }
        bitmaps[i++].pixbuf = gdk_pixbuf_new(cs, TRUE, bps, w, h);
    }
}


/* clear_images
 *
 * Efface les images de travail
 * 
 */
void clear_images()
{
    int i = 1;
    while (bitmaps[i].name != NULL) {
        if (bitmaps[i].pixbuf != NULL) {
            gdk_pixbuf_fill(bitmaps[i].pixbuf, 0xff);
        }
        i++;
    }
}


/* show_image
 *
 * Affiche une image de travail
 * 
 */
void show_image(int i)
{
    static int visible_image = 0;

    if (i >= 0) {
        visible_image = i;
    }

    if (bitmaps[visible_image].pixbuf != NULL) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), bitmaps[visible_image].pixbuf);

        /* Set the title bar */
        gchar buffer[128];
        g_snprintf(buffer, 128, "%s - %s", application_name, bitmaps[visible_image].name);
        gtk_window_set_title(GTK_WINDOW(main_window), buffer);
    }
}

/* run_processing_task
 *
 * Réalise le traitement dont le nom est passé en paramètre.
 * 
 */
void run_processing_task(const gchar *target) 
{
    if (bitmaps[IMG_SRC].pixbuf == NULL) {
        message_dialog("Avertissement", GTK_MESSAGE_WARNING, "Pas d'image chargée");
        return;
    }

    /* Récupérer les infos de l'image source */
    int w = gdk_pixbuf_get_width(bitmaps[IMG_SRC].pixbuf);
    int h = gdk_pixbuf_get_height(bitmaps[IMG_SRC].pixbuf);

    /* Effacer les images */
    clear_images();

    /* Récupérer les buffers d'image de chaque bitmap */
    uint8_t *pixels[N_BITMAPS];
    for (int i = 0; i < N_BITMAPS; ++i) {
        pixels[i] = gdk_pixbuf_get_pixels(bitmaps[i].pixbuf);
    }

    /* Trouver la fonction de traitement à appeler en fonction de la cible */
    int task_nr = 0;
    do {
        if (strcmp(process_tasks[task_nr].target, target) == 0)
            break;
    } while(process_tasks[++task_nr].target != NULL);

    /* Réaliser le traitement */
    clock_t start = clock();
    for (int i = 0; i < process_repetitions; ++i) {
        if (process_tasks[task_nr].process_fun != NULL) {
            process_tasks[task_nr].process_fun(w, h, pixels[IMG_SRC], pixels[IMG_TMP1], pixels[IMG_TMP2], pixels[IMG_DEST]);            
        }
    }
    clock_t end = clock();

    /* Afficher le temps écoulé */
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    message_dialog("Répétitions", GTK_MESSAGE_INFO, 
                   "Temps total (%d répétitions): %f secondes\n\n"
                   "Temps par itération : %f millisecondes",
                   process_repetitions, elapsed, 1000 * (elapsed / process_repetitions));
}


/*
 *
 * Fonctions de l'interface graphique principale
 * 
 */

/* Définition de l'UI */
static const gchar ui_info[] =
  "<interface>"
  "  <menu id='menubar'>"
  "    <submenu>"
  "      <attribute name='label'>_Fichier</attribute>"
  "      <section>"
  "        <item>"
  "          <attribute name='label'>_Ouvrir l'image source...</attribute>"
  "          <attribute name='action'>app.open</attribute>"
  "          <attribute name='accel'>&lt;Primary&gt;o</attribute>"
  "        </item>"
  "      </section>"
  "      <section>"
  "        <item>"
  "          <attribute name='label'>_Quitter</attribute>"
  "          <attribute name='action'>app.quit</attribute>"
  "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
  "        </item>"
  "      </section>"
  "    </submenu>"
  "    <submenu>"
  "      <attribute name='label'>Affichage</attribute>"
  "      <section>"
  "        <item>"
  "          <attribute name='label'>Image _source</attribute>"
  "          <attribute name='action'>app.view</attribute>" 
  "          <attribute name='target'>0</attribute>"
  "        </item>"
  "        <item>"
  "          <attribute name='label'>Image intermédiaire _1</attribute>"
  "          <attribute name='action'>app.view</attribute>" 
  "          <attribute name='target'>1</attribute>"
  "        </item>"
  "        <item>"
  "          <attribute name='label'>Image _intermédiaire _2</attribute>"
  "          <attribute name='action'>app.view</attribute>" 
  "          <attribute name='target'>2</attribute>"
  "        </item>"  
  "        <item>"
  "          <attribute name='label'>Image _finale</attribute>"
  "          <attribute name='action'>app.view</attribute>" 
  "          <attribute name='target'>3</attribute>"
  "        </item>"  
  "      </section>"
  "    </submenu>"    
  "    <submenu>"
  "      <attribute name='label'>_Traitement</attribute>"
  "      <section>"
  "        <item>"
  "          <attribute name='label'>Lancer l'implémentation _assembleur</attribute>"
  "          <attribute name='action'>app.process</attribute>" 
  "          <attribute name='target'>asm</attribute>"
  "        </item>"
  "        <item>"
  "          <attribute name='label'>Lancer l'implémentation _C</attribute>"
  "          <attribute name='action'>app.process</attribute>" 
  "          <attribute name='target'>c</attribute>"
  "        </item>"
  "        <item>"
  "          <attribute name='label'>Lancer l'implémentation _SIMD</attribute>"
  "          <attribute name='action'>app.process</attribute>" 
  "          <attribute name='target'>simd</attribute>"
  "        </item>"
  "      </section>"
  "      <section>"
  "        <item>"
  "          <attribute name='label'>_Répétitions...</attribute>"
  "          <attribute name='action'>app.repetitions</attribute>"
  "        </item>"
  "      </section>"
  "    </submenu>"  
  "  </menu>"
  "</interface>";


/* startup
 *
 * Appelé à la création de l'application
 * 
 */
static void startup(GApplication *app)
{
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, ui_info, -1, NULL);

    GMenuModel *menubar = (GMenuModel *)gtk_builder_get_object(builder, "menubar");

    gtk_application_set_menubar(GTK_APPLICATION (app), menubar);

    g_object_unref(builder);
}


/* activate
 *
 * Appelée à l'activation de l'application
 * 
 */
static void activate(GApplication *app)
{
    /* Créer la fenêtre principale */
    main_window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_default_size (GTK_WINDOW(main_window), 400, 600);
    gtk_window_set_title(GTK_WINDOW(main_window), application_name);

    /* Créer une boîte pour les widgets */
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER (main_window), box);

      /* Créer l'image */
    image = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);

    gtk_widget_show_all (main_window);
}

/* activate_open
 *
 * Appelée sur l'action 'ouvrir'. Ouvre un fichier image.
 * 
 */
static void activate_open(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    gchar* filename = file_chooser();

    if (filename != NULL) {
        GdkPixbuf *img_src = load_source_image(filename);
        if (img_src != NULL) {
            setup_images(img_src);
            clear_images();
            show_image(IMG_SRC);
            gtk_window_resize(GTK_WINDOW(main_window), 1, 1);
        }
    }
}

/* activate_quit
 *
 * Appelée sur l'action 'quitter'. Termine l'application.
 * 
 */
static void activate_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    g_application_quit(G_APPLICATION(user_data));
}


/* activate_view
 *
 * Appelée sur l'action 'view'. Gère l'affichage des images.
 * 
 */
static void activate_view(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    const gchar* target = g_variant_get_string(parameter, NULL);

    show_image(atoi(target));
}

/* activate_process
 *
 * Appelée sur l'action 'view'. Déclenche le traitement demandé
 * 
 */
static void activate_process(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    const gchar* target = g_variant_get_string(parameter, NULL);

    run_processing_task(target);
}

/* activate_repetitions
 *
 * Appelée sur l'action 'répétitions'. Affiche la boîte de dialogue appropriée.
 * 
 */
static void activate_repetitions(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  repetitions_dialog();
}

/* Définition des actions de l'interface */
static GActionEntry app_entries[] = {
    { "open", activate_open, NULL, NULL, NULL },
    { "quit", activate_quit, NULL, NULL, NULL },
    { "view", activate_view, "s", NULL, NULL },
    { "process", activate_process, "s", NULL, NULL },
    { "repetitions", activate_repetitions, NULL, NULL, NULL }
};

/* main 
 *
 * Fonction principale appelée au démarrage du programme
 * 
 */
int main(int argc, char *argv[]) 
{
    GtkApplication *app = gtk_application_new("fr.utc.mi01.atelier-photo", 0);

    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    g_signal_connect(app, "startup", G_CALLBACK (startup), NULL);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return status;
}
