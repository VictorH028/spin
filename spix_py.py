import spinners
import time
# Crear un objeto Spinner
s = spinners.Spinner()

# Configurar texto y símbolos
s.set_text("Cargando...")
s.set_symbols("dots")

# Iniciar y detener el spinner
s.start()
time.sleep(5)
s.stop()

