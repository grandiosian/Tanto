#!/bin/sh
chmod 744 makeCdi.sh

repertoire_courant=$(pwd)
files_with_extension=$(ls *.elf 2>/dev/null)

if [ -n "$files_with_extension" ]; then
    echo "Fichiers avec l'extension .elf trouvés:"
    echo "$files_with_extension"
    for file in $files_with_extension; do
        filename=$(basename "$file")
        filename_without_extension=$(echo "$filename" | sed 's/\.elf$//')
        echo "$filename_without_extension"
    done
else
    echo "Aucun fichier .elf trouvé"
    exit 1
fi

folder_name="cd_root"

# Génération du binaire
echo "Génération du binaire..."
elf2bin "$filename_without_extension.elf"
scramble "$filename_without_extension.bin" "$folder_name/1ST_READ.BIN"

# Création de l'IP.BIN
echo "Création de l'IP.BIN..."
makeip -a "JUE" \
       -b "1ST_READ.BIN" \
       -c "SEGA ENTERPRISES" \
       -d "20240208" \
       -e "V1.000" \
       -g "DEMO_01" \
       -i "GD-ROM1/1" \
       -n "T-12345" \
       -p "1500F10" \
       IP.BIN

# Vérification de la structure avant création
echo "Structure du cd_root:"
ls -lR cd_root/

echo "Vérification des permissions:"
find cd_root/ -type f -exec ls -l {} \;

# Force les permissions correctes
find cd_root/ -type f -exec chmod 644 {} \;
find cd_root/ -type d -exec chmod 755 {} \;

# Création de l'ISO avec plus d'options et de verbosité
echo "Création de l'ISO..."
mkisofs -v -C 0,11702 \
        -V "$filename_without_extension" \
        -G IP.BIN \
        -J \
        -r \
        -l \
        -full-iso9660-filenames \
        -o "$filename_without_extension.iso" \
        cd_root/

# Vérifications détaillées
echo "Vérification détaillée de l'ISO:"
isoinfo -d -i "$filename_without_extension.iso"
isoinfo -f -i "$filename_without_extension.iso"

# Test d'extraction
mkdir -p test_extract
isoinfo -i "$filename_without_extension.iso" -x /1ST_READ.BIN > test_extract/1ST_READ.BIN
ls -l test_extract/1ST_READ.BIN
echo $filename_without_extension
# Création du CDI
echo "Création du CDI..."

# Vérifier si la commande cdi4dc existe
if command -v cdi4dc >/dev/null 2>&1; then
    echo "cdi4dc trouvé, utilisation de cdi4dc..."
    # Utiliser cdi4dc avec les arguments appropriés
    "../../SCRIPTS/cdi4dc.exe" "$filename_without_extension.iso" "$filename_without_extension.cdi"
else
    echo "cdi4dc non trouvé, utilisation de mkdcdisc..."
   
fi


 # Utiliser mkdcdisc avec les arguments appropriés
#mkdcdisc -e "$filename_without_extension".elf -o "$filename_without_extension".cdi --no-compress


echo "Build terminé dans: $repertoire_courant"