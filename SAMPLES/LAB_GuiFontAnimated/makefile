
# Forcer l'utilisation de KOS-CC
CC = /opt/toolchains/dc/kos/utils/build_wrappers/kos-cc

# Dossiers du projet
SRC_DIR = ../../TANTO

TANTO_ENGINE_DIR  = ../../TANTO
SCRIPTS_DIR       = ../../SCRIPTS/MAKECDI
BUILD_DIR         = ../../BUILD/$(notdir $(CURDIR))
OBJ_DIR          = OBJ

# Paramètres de compilation KOS
CFLAGS += $(KOS_CFLAGS) -I../.. -I$(TANTO_ENGINE_DIR)
LDFLAGS = -L$(KOS_BASE)/lib -L$(KOS_BASE)/../kos-ports/lib -Wl,-rpath=$(KOS_BASE)/lib -lkmg

# Cibles de compilation
ELF = $(BUILD_DIR)/$(notdir $(CURDIR)).elf
CDI = $(BUILD_DIR)/$(notdir $(CURDIR)).cdi

TANTO_SRCS = $(wildcard $(TANTO_ENGINE_DIR)/*.c)
TANTO_OBJS = $(patsubst $(TANTO_ENGINE_DIR)/%.c,$(OBJ_DIR)/%.o,$(TANTO_SRCS))

OBJS = $(OBJ_DIR)/main.o $(TANTO_OBJS)

# Au début du fichier, après la définition des variables
$(info TANTO_SRCS = $(TANTO_SRCS))
$(info TANTO_OBJS = $(TANTO_OBJS))
$(info OBJS = $(OBJS))

.PHONY: all clean force $(OBJS)



# Règle principale
all: force $(CDI) copyScripts 

elf: force $(ELF) copyScripts


copyScripts:
	cp -r $(SCRIPTS_DIR)/* $(BUILD_DIR);

run: 
	@echo "Running on emulator..." 
	cd $(BUILD_DIR) && ./RunOnEmulator.sh

# Force la recompilation
force:
	@echo "Checking source files..."
	@if [ ! -f main.c ]; then \
		echo "Error: main.c not found"; \
		exit 1; \
	fi
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/cd_root
	mkdir -p $(OBJ_DIR)
	@if [ -d cd_root ] && [ "$$(ls -A cd_root)" ]; then \
		cp -r cd_root/* $(BUILD_DIR)/cd_root/; \
	fi

# Génération du CDI
$(CDI): $(ELF)
	@echo "Creating CDI..."
	cd $(BUILD_DIR) && $(SCRIPTS_DIR)/RunOnEmulator.sh ./$(notdir $(ELF))

# Génération du ELF
$(ELF): $(OBJ_DIR)/main.o $(TANTO_OBJS)
	@echo "Compiling TANTO files..."
	$(foreach src,$(TANTO_SRCS),$(CC) $(CFLAGS) -c $(src) -o $(OBJ_DIR)/$(notdir $(src:.c=.o));)
	@echo "Linking..."
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	cd $(BUILD_DIR) && $(SCRIPTS_DIR)/makeCdi.sh ./$(notdir $(ELF))


$(OBJ_DIR)/main.o: main.c
	@echo "Compiling main.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJS)
	rm -rf $(BUILD_DIR)
	rm -rf $(OBJ_DIR)

# Debug
debug:
	@echo "Current directory: $$(pwd)"
	@echo "Files in current directory:"
	@ls -la
	@echo "BUILD_DIR: $(BUILD_DIR)"
	@echo "OBJS: $(OBJS)"