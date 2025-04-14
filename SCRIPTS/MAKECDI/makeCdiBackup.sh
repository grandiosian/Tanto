#!/bin/sh
chmod 744 makeCdi.sh

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

#makeip -v IP.bin

# Création de l'IP.BIN avec les options correctes
makeip -a "JUE" \
       -b "1ST_READ.BIN" \
       -c "YOUR_COMPANY" \
       -d "20240208" \
       -e "V1.000" \
       -g "$filename_without_extension" \
       -i "GD-ROM1/1" \
       -n "T-12345" \
       -p "1500F10" \
       IP.BIN

# Création de la piste audio vide
dd if=/dev/zero of=silence.raw bs=44100 count=176400

# Création de l'ISO
mkisofs -C 0,11702 -V "$filename_without_extension" -G IP.BIN -J -r -l -o "$filename_without_extension.iso" cd_root/

# Création du CDI
cdi4dc "$filename_without_extension.iso" "$filename_without_extension.cdi" -d silence.raw

# Nettoyage
rm "$filename_without_extension.iso"
rm silence.raw

echo "make CDI terminé !"
echo "$repertoire_courant"