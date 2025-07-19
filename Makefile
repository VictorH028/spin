BIN_NAME = spin
CXX = clang++

OBJ_DIR = obj
SO_DIR = lib
MAN_DIR = man

CXXFLAGS = -Wall -O3 -fPIC -std=c++20 -Iinclude
LDFLAGS = 

PYTHON_INCLUDES = $(shell python3 -m pybind11 --includes)
PYTHON_LDFLAGS = $(shell python3-config --libs --embed) -shared
SO_NAME = spinners$(shell python3-config --extension-suffix)
PYTHON_SITE_PACKAGES = $(shell python3 -m site --user-site)

# Reglas principales
all: style  build-bin build-python

# Config   https://zed0.co.uk/clang-format-configurator/   
style:
	@clang-format -style=WebKit -dump-config > .clang-format
	@find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -style=file -i {} \;

# Regla para compilar solo el binario
build-bin: $(PREFIX)/bin/$(BIN_NAME)

$(OBJ_DIR)/spin.o: spin.cpp
	@echo "Compiling the binary $(BIN_NAME)..."
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Copying the man page..."
	@mkdir -p $(PREFIX)/share/man/man1/
	@cp $(MAN_DIR)/spin.1 $(PREFIX)/share/man/man1/
	@mkdir -p ${PREFIX}/local/include 	
	@cp  ./include/spinners.hpp ${PREFIX}/local/include  


$(PREFIX)/bin/$(BIN_NAME): $(OBJ_DIR)/spin.o src/spinners.cpp
	@mkdir -p $(PREFIX)/bin
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Regla para compilar solo el módulo Python
build-python: $(SO_DIR)/$(SO_NAME)

$(OBJ_DIR)/spin_py.o: spin_py.cpp
	@echo "Compilando la biblioteca Python..."
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(PYTHON_INCLUDES) -c $< -o $@

$(OBJ_DIR)/spinners.o: src/spinners.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(SO_DIR)/$(SO_NAME): $(OBJ_DIR)/spin_py.o $(OBJ_DIR)/spinners.o
	@mkdir -p $(SO_DIR)
	@$(CXX) -shared $(CXXFLAGS) $^ -o $@ $(PYTHON_LDFLAGS)
	@echo "Instalando el módulo Python en $(PYTHON_SITE_PACKAGES)..."
	@mkdir -p $(PYTHON_SITE_PACKAGES)
	@cp $@ $(PYTHON_SITE_PACKAGES)

# Limpieza de archivos generados
clean:
	@echo "Limpiando archivos generados..."
	@rm -rf $(OBJ_DIR) $(SO_DIR) $(MAN_PAGE)

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
	rm -f ${PREFIX}/local/include 
	@echo "Desinstalación completa."

# Instalación completa
install: all
	@echo "Instalación completada de $(BIN_NAME) y su módulo Python."

# Targets auxiliares
.PHONY: all build-bin build-python clean clean-all install 

