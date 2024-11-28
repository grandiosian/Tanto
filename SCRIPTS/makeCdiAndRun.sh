#!/bin/sh
chmod 744 makeCdiAndRun.sh

flycast_path="C:/Users/lione/Desktop/RESSOURCES/EMULATEURS/DREAMCAST/flycast-win64-2.2/flycast.exe"
repertoire_courant=$(pwd)
# Liste des fichiers avec l'extension .elf dans le dossier courant
files_with_extension=$(ls *.elf 2>/dev/null)



# Vérifie si des fichiers avec l'extension .elf ont été trouvés
if [ -n "$files_with_extension" ]; then
    echo "Fichiers avec l'extension .elf dans le dossier courant :"
    echo "$files_with_extension"
     # Boucle sur chaque fichier pour afficher le nom sans l'extension
    for file in $files_with_extension; do
        # Utilisation de basename pour extraire le nom du fichier sans le chemin
        filename=$(basename "$file")
        
        # Utilisation de sed pour supprimer l'extension .elf
        filename_without_extension=$(echo "$filename" | sed 's/\.elf$//')
        
        echo "$filename_without_extension"
    done
else
    echo "Aucun fichier avec l'extension .elf trouvé dans le dossier courant."
fi

# Nom du dossier à créer
folder_name="cd_root"

# Vérifie si le dossier existe déjà
if [ -d "$folder_name" ]; then
    echo "Le dossier \"$folder_name\" existe déjà."
else
    # Crée le dossier s'il n'existe pas
    mkdir "$folder_name"
    echo "Le dossier \"$folder_name\" a été créé."
fi


elf2bin "$filename_without_extension".elf 
scramble "$filename_without_extension".bin  cd_root/1ST_READ.bin

makeip -v IP.bin

makedisc "$filename_without_extension".cdi cd_root IP.BIN "$filename_without_extension"

echo "$repertoire_courant"
$flycast_path "$repertoire_courant"/"$filename_without_extension".cdi