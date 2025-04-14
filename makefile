# Dossiers du projet
TANTO_ENGINE_DIR = TANTO
SAMPLES_DIR      = SAMPLES
SCRIPTS_DIR      = SCRIPTS
BUILD_DIR        = BUILD

# Cible par défaut
.PHONY: all
all: build_samples

# Compilation de tous les échantillons (SAMPLES)
.PHONY: build_samples
build_samples:
	@for dir in $$(ls -d $(SAMPLES_DIR)/*); do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "Compilation de $$dir"; \
			$(MAKE) -C "$$dir" clean; \
			$(MAKE) -C "$$dir" elf; \
			$(MAKE) -C "$$dir" generate; \
		fi \
	done

# Nettoyage
.PHONY: clean
clean:
	@for dir in $$(ls -d $(SAMPLES_DIR)/*); do \
		if [ -f "$$dir/Makefile" ]; then \
			$(MAKE) -C "$$dir" clean; \
		fi \
	done
	-rm -rf $(BUILD_DIR)