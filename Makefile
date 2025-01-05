# Define el prefijo de instalación (puedes cambiar esto)
PREFIX ?= /usr/local

# Define el nombre del binario
BIN_NAME = spix

# Define las flags del compilador
CXXFLAGS = -Wall -std=c++17 -O3 -fPIC

# Directorio de archivos objeto
OBJ_DIR = obj

# Directorio para el compilado del .so
SO_DIR = lib

# Archivos fuente
CPP_SOURCES_BIN = spix.cpp
CPP_SOURCES_PY = spix_py.cpp

# Archivos objeto
OBJECTS_BIN = $(OBJ_DIR)/spix.o
OBJECTS_PY = $(OBJ_DIR)/spix_py.o

# Compilado de la biblioteca .so para Python
SO_NAME = spinners$(shell python3-config --extension-suffix)
SO_FILE = $(SO_DIR)/$(SO_NAME)

# Directorio de instalación del módulo Python
PYTHON_SITE_PACKAGES = $(shell python3 -m site --user-site)

# Define la página de manual
MAN_DIR = man
MAN_PAGE = $(MAN_DIR)/spix.1

# Reglas principales
all: build-bin build-python

# Regla para compilar solo el binario
build-bin: $(PREFIX)/bin/$(BIN_NAME)

$(PREFIX)/bin/$(BIN_NAME): $(OBJECTS_BIN)
	@echo "Compiling the binary $(BIN_NAME)..."
	mkdir -p $(PREFIX)/bin
	clang++ $(CXXFLAGS) $^ -o $@
	@echo "Copying the man page..."
	mkdir -p $(PREFIX)/share/man/man1/
	cp $(MAN_PAGE) $(PREFIX)/share/man/man1/

# Regla para compilar solo el módulo Python
build-python: $(SO_FILE)
#
# $(SO_FILE): $(OBJECTS_PY)
# 	@echo "Compilando la biblioteca Python..."
# 	mkdir -p $(SO_DIR)
# 	clang++ -shared $(CXXFLAGS) $(shell python3 -m pybind11 --includes) $^ -o $@ \
# 		$(shell python3-config --cflags --ldflags) -I/data/data/com.termux/files/usr/include/python3.12 \
# 		-I/data/data/com.termux/files/usr/lib/python3.12/site-packages/pybind11/include
# 	@echo "Instalando el módulo Python en $(PYTHON_SITE_PACKAGES)..."
# 	mkdir -p $(PYTHON_SITE_PACKAGES)
# 	cp $(SO_FILE) $(PYTHON_SITE_PACKAGES)
#
# Compilar objetos
$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling the object file $<..."
	mkdir -p $(OBJ_DIR)
	clang++ $(CXXFLAGS) -c $< -o $@ -I  \
		-I/data/data/com.termux/files/usr/include/python3.12

# Página de manual predefinida
$(MAN_PAGE):
	@echo "Generating the man page..."
	mkdir -p $(MAN_DIR)
	help2man -N --no-info --name="spix - Herramienta personalizada" -o $(MAN_PAGE) $(PREFIX)/bin/$(BIN_NAME)

# Limpieza de archivos generados
clean:
	@echo "Limpiando archivos generados..."
	rm -rf $(OBJ_DIR) $(SO_DIR) $(MAN_PAGE)

# Limpieza completa (incluye instalación)
clean-all: clean
	@echo "Limpiando binarios y bibliotecas instaladas..."
	rm -f $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1
	rm -f $(PYTHON_SITE_PACKAGES)/$(SO_NAME)

# Desinstalación
uninstall:
	@echo "Desinstalando $(BIN_NAME) y el módulo Python..."
	rm -f $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1
	rm -f $(PYTHON_SITE_PACKAGES)/$(SO_NAME)
	@echo "Desinstalación completa."

# Instalación completa
install: all
	@echo "Instalación completada de $(BIN_NAME) y su módulo Python."

# Targets auxiliares
.PHONY: all build-bin build-python clean clean-all install uninstall

