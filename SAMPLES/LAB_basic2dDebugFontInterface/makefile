# Configuration de base
CC = /opt/toolchains/dc/kos/utils/build_wrappers/kos-cc

# Dossiers du projet
TANTO_ENGINE_DIR  = ../../TANTO
SCRIPTS_DIR       = ../../SCRIPTS/MAKECDI
BUILD_DIR         = ../../BUILD/$(notdir $(CURDIR))
OBJ_DIR          = OBJ

# Définition des dossiers source
EXTRA_SRC_DIRS = ../../TANTO

# Le binaire final
TARGET = $(notdir $(CURDIR)).elf

# Les fichiers source et objets
MAIN_OBJ = $(OBJ_DIR)/main.o
ROMDISK_IMG = $(OBJ_DIR)/romdisk.img
ROMDISK_OBJ = $(OBJ_DIR)/romdisk.o

# Récupération automatique des fichiers .c de tous les dossiers
EXTRA_SRCS = $(foreach dir,$(EXTRA_SRC_DIRS),$(wildcard $(dir)/*.c))
EXTRA_OBJS = $(notdir $(EXTRA_SRCS:.c=.o))
EXTRA_OBJS := $(addprefix $(OBJ_DIR)/,$(EXTRA_OBJS))

# Récupérer tous les fichiers .o dans $(OBJ_DIR)
ALL_OBJS = $(wildcard $(OBJ_DIR)/*.o)

# Outils 
KOS_GENROMFS = $(KOS_BASE)/utils/genromfs/genromfs
BIN2O = $(KOS_BASE)/utils/bin2o/bin2o

# Flags 
CFLAGS = $(KOS_CFLAGS) -I../.. -I$(TANTO_ENGINE_DIR) -fdiagnostics-color=always -MMD -MP
LDFLAGS = -L$(KOS_BASE)/lib -L$(KOS_BASE)/../kos-ports/lib -Wl,-rpath=$(KOS_BASE)/lib -lkmg -lkosutils

# Inclusion des fichiers de dépendance
-include $(EXTRA_OBJS:.o=.d)

# Règles de compilation pas à pas
all: step1 step2 step3 step4 step5 step6 step7

generate: step1 step2 step3 step4 step5 step6 makeCdi

run:
	@echo "=== Run build on emulator ===="
	cd $(BUILD_DIR) && ./RunOnEmulator.sh

step1:
	@echo "=== ÉTAPE 1: Création des dossiers et copie cd_root ==="
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/cd_root
	mkdir -p $(OBJ_DIR)
	@if [ -d cd_root ] && [ "$$(ls -A cd_root)" ]; then \
		cp -r cd_root/* $(BUILD_DIR)/cd_root/; \
	fi

step2:
	@echo "=== ÉTAPE 2: Création du romdisk ==="
	@if [ ! -d romdisk ]; then \
		echo "Erreur : Le dossier 'romdisk' n'existe pas. Veuillez créer un dossier romdisk à la racine"; \
		exit 1; \
	else \
		$(KOS_GENROMFS) -v -f $(ROMDISK_IMG) -d romdisk; \
		$(BIN2O) $(ROMDISK_IMG) romdisk $(ROMDISK_OBJ); \
	fi

step3: $(EXTRA_OBJS)
	@echo "=== ÉTAPE 3: Compilation des fichiers .c supplémentaires ==="

$(OBJ_DIR)/%.o: $(foreach dir,$(EXTRA_SRC_DIRS),$(dir)/%.c)
	@echo "Compilation de $<"
	$(CC) $(CFLAGS) -c $< -o $@

step4:
	@echo "=== ÉTAPE 4: Compilation des .c en .o ==="
	$(CC) $(CFLAGS) -c main.c -o $(MAIN_OBJ)

step5:
	@echo "=== ÉTAPE 5: Liaison finale ==="
	$(CC) -o $(BUILD_DIR)/$(TARGET) $(MAIN_OBJ) $(ROMDISK_OBJ) $(ALL_OBJS) $(LDFLAGS)

step6:
	@echo "=== ÉTAPE 6: Copie des scripts ==="
	dos2unix $(SCRIPTS_DIR)/makeCdi.sh
	dos2unix $(SCRIPTS_DIR)/makeCdiAndRun.sh
	dos2unix $(SCRIPTS_DIR)/RunOnEmulator.sh
	chmod +x $(SCRIPTS_DIR)/makeCdi.sh
	chmod +x $(SCRIPTS_DIR)/makeCdiAndRun.sh
	chmod +x $(SCRIPTS_DIR)/RunOnEmulator.sh
	cp -r $(SCRIPTS_DIR)/* $(BUILD_DIR)

step7:
	@echo "=== ÉTAPE 7: Création CDI et lancement émulateur ==="
	cd $(BUILD_DIR) && ./makeCdi.sh ./$(TARGET)
	cd $(BUILD_DIR) && ./RunOnEmulator.sh


makeCdi:
	@echo "=== Création CDI ==="
	cd $(BUILD_DIR) && ./makeCdi.sh ./$(TARGET)

clean:
	@echo "=== Nettoyage ==="
	rm -rf $(BUILD_DIR)
	rm -rf $(OBJ_DIR)
	rm -f $(OBJ_DIR)/*.d

.PHONY: all clean step1 step2 step3 step4 step5 step6 step7