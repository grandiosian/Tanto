#!/bin/bash
chmod 744 RunOnEmulator.sh

# Trouver le fichier .cdi
cdi_file=$(ls *.cdi 2>/dev/null | head -1)
if [ -z "$cdi_file" ]; then
    echo "Aucun fichier .cdi trouvé."
    exit 1
fi

echo "Fichier trouvé: $cdi_file"

# Extraire le nom du fichier sans chemin et l'extension
filename_only=$(basename "$cdi_file")

# Créer un fichier temporaire pour recevoir les sorties
log_file="flycast_output.log"
touch "$log_file"
echo "Les sorties seront enregistrées dans: $log_file"

# Lancer l'émulateur en utilisant le fichier local sans conversion de chemin
echo "Lancement de l'émulateur..."

# Deux options pour exécuter:

# Option 1: Exécution directe sans redirection (pour déboguer)
"/mnt/c/Users/lione/Desktop/RESSOURCES/EMULATEURS/DREAMCAST/flycast-win64-2.2/flycast.exe" "$filename_only" &

# Capturer la sortie console en même temps
flycast_pid=$!
sleep 2  # Donner le temps à l'émulateur de démarrer

# Option 2: Si vous voulez vraiment capturer la sortie, décommentez ceci:
# "/mnt/c/Users/lione/Desktop/RESSOURCES/EMULATEURS/DREAMCAST/flycast-win64-2.2/flycast.exe" "$filename_only" > "$log_file" 2>&1 &
# flycast_pid=$!

echo "L'émulateur est en cours d'exécution (PID: $flycast_pid)"
echo "Pour voir les sorties de l'émulateur, consultez les logs de Flycast directement."
echo "Appuyez sur Ctrl+C pour terminer le script quand vous avez fini avec l'émulateur."

# Si Flycast a son propre fichier de log, vous pouvez le surveiller:
flycast_own_log="/mnt/c/Users/lione/AppData/Roaming/flycast/flycast.log"
if [ -f "$flycast_own_log" ]; then
    echo "Surveillance du fichier de log de Flycast..."
    tail -f "$flycast_own_log"
fi

# Attendre que l'utilisateur termine manuellement
wait $flycast_pid