# Define el prefijo de instalación (puedes cambiar esto)
PREFIX ?= /usr/local

# Define el nombre del binario
BIN_NAME = spix

# Define las flags del compilador
CFLAGS = -Wall  -std=c++17

# Define el directorio de los archivos objeto
OBJ_DIR = obj

# Define los archivos objeto
OBJECTS = $(OBJ_DIR)/spix.o

# Define el directorio de la página de manual
MAN_DIR = man

# Define la página de manual
MAN_PAGE = $(MAN_DIR)/spix.1

# Regla para todos los targets (el target por defecto es 'all')
all: $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1

# Regla para compilar el binario
$(PREFIX)/bin/$(BIN_NAME): $(OBJECTS)
	@mkdir -p $(@D)  # Crea el directorio si no existe
	clang++ $(CFLAGS) $(OBJECTS) -o $@

# Regla para compilar los archivos objeto
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D) # Crea el directorio si no existe
	clang++ $(CFLAGS) -c $< -o $@

# Regla para instalar la página de manual
$(PREFIX)/share/man/man1/$(BIN_NAME).1: $(MAN_PAGE)
	@mkdir -p $(@D)  # Crea el directorio si no existe
	cp $< $@

# Regla para limpiar los archivos generados
clean:
	rm -rf $(OBJ_DIR) $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1

# Regla para instalar (combina la instalación del binario y la página de manual)
install: all
	@echo "Instalando $(BIN_NAME) en $(PREFIX)/bin"
	@echo "Instalando la página de manual en $(PREFIX)/share/man/man1"

# Regla para desinstalar
uninstall:
	rm -f $(PREFIX)/bin/$(BIN_NAME) $(PREFIX)/share/man/man1/$(BIN_NAME).1
	@echo "Desinstalado $(BIN_NAME)"

# Marca estos targets como "phony"
.PHONY: all clean install uninstall

