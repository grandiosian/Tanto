# TANTO

TANTO est un framework de développement pour la console Sega Dreamcast, construit sur KallistiOS (KOS) - un système de développement libre et open-source.

## Structure du Projet

```
/
├── TANTO/          # Fichiers sources du moteur
├── BUILD/          # Fichiers compilés
│   └── DEMO_XX/    # Fichiers compilés de SAMPLES/DEMO_XX
├── SAMPLES/        # Exemples de projets
│   └── DEMO_XX/    # Projets de démo
└── SCRIPTS/        # Scripts utilitaires
    ├── makeCdiAndRun.sh    # Script de compilation et lancement
    └── run.bat            # Lanceur Windows
```

## Prérequis

- KallistiOS (KOS) v2.0+
- Compilateur kos-cc
- Bibliothèques additionnelles pour son et vidéo (requis ultérieurement)

## Installation & Configuration

### Windows
- Recommandé : Installer DreamSDK depuis https://github.com/dreamsdk
- Alternative : Utiliser WSL

### Linux/macOS
- Suivre le guide d'installation sur https://github.com/KallistiOS/KallistiOS

Configurer le chemin de l'émulateur dans `SCRIPTS/makeCdiAndRun.sh`:
```bash
flycast_path="chemin/vers/votre/emulateur/dreamcast"
```

## Utilisation

1. Exécuter `make` à la racine
2. Naviguer vers `/BUILD/DEMO_XX/`
3. Lancer :
   - Windows avec DreamSDK : `run.bat`
   - Linux/macOS/WSL : `makeCdiAndRun.sh`

Compilation rapide : Exécuter `make` dans n'importe quel dossier de projet pour une compilation et un lancement automatique.

## Résolution des problèmes

- "KOS not found" : Vérifier les variables d'environnement KOS
- Erreur de compilation : Vérifier la version de kos-cc
- L'émulateur ne démarre pas : Vérifier le chemin dans makeCdiAndRun.sh

## Documentation

Wiki et tutoriels prochainement disponibles !

## Licence

Apache License 2.0