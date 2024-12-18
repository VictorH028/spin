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

# Regla para todos los targets 
all: bin python

# Compilar solo el binario
bin: $(PREFIX)/bin/$(BIN_NAME)

# Compilar el binario
$(PREFIX)/bin/$(BIN_NAME): $(OBJECTS_BIN)
	clang++ $(CXXFLAGS) $^ -o $@
	cp $(MAN_PAGE) $(PREFIX)/share/man/man1/

# Compilar solo el módulo Python
python: $(SO_FILE)

# Compilar la biblioteca .so para Python
$(SO_FILE): $(OBJECTS_PY)
	mkdir -p $(SO_DIR)
	clang++ -O3 -Wall -shared -fPIC $(shell python3 -m pybind11 --includes) $^ -o $@ \
		-I /data/data/com.termux/files/usr/lib/python3.12/site-packages/pybind11/include \
		-I /data/data/com.termux/files/usr/include/python3.12/ -L/data/data/com.termux/files/usr/lib -lpython3.12 -ldl -lpthread -lm
	mkdir -p $(PYTHON_SITE_PACKAGES)
	cp $(SO_FILE) $(PYTHON_SITE_PACKAGES)

# Compilar objetos
$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(OBJ_DIR)
	clang++ $(CXXFLAGS) -c $< -o $@ -I /data/data/com.termux/files/usr/lib/python3.12/site-packages/pybind11/include \
		-I /data/data/com.termux/files/usr/include/python3.12/ -L/data/data/com.termux/files/usr/lib

# Página de manual predefinida
$(MAN_PAGE):
	mkdir -p $(MAN_DIR)
	help2man -N --no-info --name="spix" -o $(MAN_PAGE) $(PREFIX)/bin/$(BIN_NAME)

# Limpieza de archivos generados
clean:
	rm -rf $(OBJ_DIR) $(SO_DIR) $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1

# Desinstalación
uninstall:
	rm -f $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1
	rm -f $(PYTHON_SITE_PACKAGES)/$(SO_NAME)
	@echo "Desinstalado $(BIN_NAME) y el módulo Python"

# Targets auxiliares
.PHONY: all bin python clean install install-python uninstall

