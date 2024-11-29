# TANTO

*Read this in other languages: [Français](README.fr.md)*

TANTO is a development framework for the Sega Dreamcast console, built on KallistiOS (KOS) - a free, open-source development system.

## Project Structure

```
/
├── TANTO/          # Engine source files
├── BUILD/          # Compiled files
│   └── DEMO_XX/    # Compiled files from SAMPLES/DEMO_XX
├── SAMPLES/        # Example projects
│   └── DEMO_XX/    # Demo projects
└── SCRIPTS/        # Utility scripts
    ├── makeCdiAndRun.sh    # Build and run script
    └── run.bat            # Windows runner
```

## Prerequisites

- KallistiOS (KOS) v2.0+
- kos-cc compiler
- Additional libraries for sound and video (upcoming requirement)

## Installation & Setup

### Windows
- Recommended: Install DreamSDK from https://github.com/dreamsdk
- Alternative: Use WSL

### Linux/macOS
- Follow installation guide at https://github.com/KallistiOS/KallistiOS

Configure emulator path in `SCRIPTS/makeCdiAndRun.sh`:
```bash
flycast_path="path/to/your/dreamcast/emulator"
```

## Usage

1. Run `make` at project root
2. Navigate to `/BUILD/DEMO_XX/`
3. Run:
   - Windows with DreamSDK: `run.bat`
   - Linux/macOS/WSL: `makeCdiAndRun.sh`

Quick build: Run `make` in any project folder for automatic compilation and emulator launch.

## Troubleshooting

- "KOS not found": Check KOS environment variables
- Compilation error: Verify kos-cc version
- Emulator won't start: Check path in makeCdiAndRun.sh

## Documentation

Wiki and tutorials coming soon!

## License

Apache License 2.0