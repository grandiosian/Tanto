#!/bin/sh
chmod 744 RunOnEmulator.sh

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


echo "$repertoire_courant"
$flycast_path "$repertoire_courant"/"$filename_without_extension".cdi