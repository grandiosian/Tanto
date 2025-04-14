#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dc/cdrom.h>

void draw_debug_text(const char* text, int line) {
   if(line < 20) {  // 20 lignes maximum par page
       int offset = 640 * (24 * (line + 1)) + 8 * 2;
       bfont_draw_str(vram_s + offset, 640, 1, text);
   }
}

const char* get_status_string(int status) {
   switch(status) {
       case CD_STATUS_READ_FAIL: return "READ_FAIL";
       case CD_STATUS_BUSY: return "BUSY";
       case CD_STATUS_PAUSED: return "PAUSED";
       case CD_STATUS_STANDBY: return "STANDBY";
       case CD_STATUS_PLAYING: return "PLAYING";
       case CD_STATUS_SEEKING: return "SEEKING";
       case CD_STATUS_SCANNING: return "SCANNING";
       case CD_STATUS_OPEN: return "OPEN";
       case CD_STATUS_NO_DISC: return "NO_DISC";
       case CD_STATUS_RETRY: return "RETRY";
       case CD_STATUS_ERROR: return "ERROR";
       case CD_STATUS_FATAL: return "FATAL";
       default: return "UNKNOWN";
   }
}

const char* get_disc_type_string(int type) {
   switch(type) {
       case CD_CDDA: return "CDDA/NO_DISC";
       case CD_CDROM: return "CD-ROM/CD-R";
       case CD_CDROM_XA: return "CD-ROM XA";
       case CD_CDI: return "CD-i";
       case CD_GDROM: return "GD-ROM";
       case CD_FAIL: return "FAIL";
       default: return "UNKNOWN";
   }
}

void draw_page_status(int *line) {
   char msg[256];
   int cd_status, disc_type;

   draw_debug_text("=== STATUS CD ===", (*line)++);
   
   // Test de 1ST_READ.BIN
   file_t iso_file = fs_open("/cd/1ST_READ.BIN", O_RDONLY);
   if(iso_file >= 0) {
       draw_debug_text("1ST_READ.BIN trouvé - CDI standard", (*line)++);
       fs_close(iso_file);
   } else {
       draw_debug_text("1ST_READ.BIN non trouvé", (*line)++);
   }

   // Vérification du statut
   int status_ret = cdrom_get_status(&cd_status, &disc_type);
   sprintf(msg, "Retour status: %d", status_ret);
   draw_debug_text(msg, (*line)++);
   sprintf(msg, "Statut CD: %s (%d)", get_status_string(cd_status), cd_status);
   draw_debug_text(msg, (*line)++);
   sprintf(msg, "Type: %s (0x%x)", get_disc_type_string(disc_type), disc_type);
   draw_debug_text(msg, (*line)++);

   // Lecture du TOC avec réinitialisation et double vérification
   CDROM_TOC toc;
   draw_debug_text("Lecture du TOC...", (*line)++);

   // Réinitialisation avant lecture TOC
   cdrom_reinit();
   timer_spin_sleep(1000);

   if (cdrom_read_toc(&toc, 0) == ERR_OK) {
       sprintf(msg, "Premier secteur: %d", (int)TOC_LBA(toc.first));
       draw_debug_text(msg, (*line)++);
       sprintf(msg, "Dernier secteur: %d", (int)TOC_LBA(toc.last));
       draw_debug_text(msg, (*line)++);
       
       // Si les secteurs sont identiques, tente une seconde lecture
       if(TOC_LBA(toc.first) == TOC_LBA(toc.last)) {
           draw_debug_text("Secteurs identiques, seconde lecture...", (*line)++);
           timer_spin_sleep(2000);
           
           // Réinitialise avec paramètres spécifiques
           cdrom_reinit_ex(CDROM_READ_DATA_AREA, 0, 2048);
           timer_spin_sleep(1000);
           
           cdrom_read_toc(&toc, 0);
           sprintf(msg, "Premier secteur (2): %d", (int)TOC_LBA(toc.first));
           draw_debug_text(msg, (*line)++);
           sprintf(msg, "Dernier secteur (2): %d", (int)TOC_LBA(toc.last));
           draw_debug_text(msg, (*line)++);
       }
   } else {
       draw_debug_text("Erreur lecture TOC", (*line)++);
   }
}

void draw_page_paths(int *line) {
   char msg[256];
   const char* paths[] = {
       "/cd",
       "/cd/",
       "/cd_root",
       "/cd_root/",
       "/",
       NULL
   };

   draw_debug_text("=== TEST CHEMINS ===", (*line)++);
   
   for(int i = 0; paths[i] != NULL && *line < 18; i++) {
       sprintf(msg, "Test: %s", paths[i]);
       draw_debug_text(msg, (*line)++);
       
       file_t dir = fs_open(paths[i], O_RDONLY | O_DIR);
       if(dir >= 0) {
           sprintf(msg, "  Succès! (handle: %d)", dir);
           draw_debug_text(msg, (*line)++);
           fs_close(dir);
       } else {
           sprintf(msg, "  Échec (erreur: %d)", dir);
           draw_debug_text(msg, (*line)++);
       }
   }
}

void draw_page_files(int *line) {
   char msg[256];
   draw_debug_text("=== FICHIERS DANS /cd ===", (*line)++);
   
   file_t dir = fs_open("/cd", O_RDONLY | O_DIR);
   if(dir >= 0) {
       dirent_t *ent;
       while ((ent = fs_readdir(dir)) != NULL && *line < 18) {
           if (strcmp(ent->name, ".") == 0 || strcmp(ent->name, "..") == 0) 
               continue;
           
           char full_path[512];
           sprintf(full_path, "/cd/%s", ent->name);
           file_t test_file = fs_open(full_path, O_RDONLY);
           
           if (test_file >= 0) {
               sprintf(msg, "%s (taille: %d)", ent->name, fs_total(test_file));
               fs_close(test_file);
           } else {
               sprintf(msg, "%s (erreur ouverture)", ent->name);
           }
           draw_debug_text(msg, (*line)++);
       }
       fs_close(dir);
   } else {
       sprintf(msg, "Erreur ouverture /cd (code: %d)", dir);
       draw_debug_text(msg, (*line)++);
   }
}

int main(int argc, char **argv) {
   vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
   char msg[256];
   int current_page = 0;

   // Initialisation du CD
   cdrom_init();
   timer_spin_sleep(1000);

   while(1) {
       // Effacer l'écran
       memset(vram_s, 0, 640 * 480 * 2);
       int current_line = 0;

       // En-tête de page
       sprintf(msg, "Page %d/3 - Debug CD", current_page + 1);
       draw_debug_text(msg, current_line++);
       current_line++;  // Ligne vide

       // Afficher le contenu selon la page
       switch(current_page) {
           case 0:
               draw_page_status(&current_line);
               break;
           case 1:
               draw_page_paths(&current_line);
               break;
           case 2:
               draw_page_files(&current_line);
               break;
       }

       // Instructions de navigation
       draw_debug_text("START: Quitter, A: Page suivante, B: Page précédente", 19);

       // Attente input
       while(1) {
           MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
               if(st->buttons & CONT_START)
                   return 0;
               if(st->buttons & CONT_A) {
                   current_page = (current_page + 1) % 3;
                   timer_spin_sleep(200);
                   goto next_page;
               }
               if(st->buttons & CONT_B) {
                   current_page = (current_page + 2) % 3;
                   timer_spin_sleep(200);
                   goto next_page;
               }
           MAPLE_FOREACH_END()
       }
next_page:
       continue;
   }

   return 0;
}