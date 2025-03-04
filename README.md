![alt text](./.img/loading.jpg)
---
## spin - CLI Spinner 

spin is a command line (CLI) tool written in C++ that displays an animated spinner while a background process is running.  It is useful for visually indicating that a task is in progress, especially if the task takes a considerable amount of time.

## Install

- *Manual*

```
git clone https://github.com/VictorH028/spin && cd  spin
```
```
make build-bin
```

- *this apt*

```
wget -P $PREFIX/etc/apt/sources.list.d  https://raw.githubusercontent.com/VictorH028/victorh028.github.io/refs/heads/main/key/demon-termux-packages.list
```

```
apt update
```

```
apt install spin
```


## Usage
    
Command line:
```
Usage: spin [options]

Options:
  -v, --version         show version  
  -h, --help            show this help message and exit
  -t TEXT, --text=TEXT  Text to show
  -i INT, --interval=INT
                        Interval in milliseconds
  -s STRING, --symbols=STRING
                        Symbols the spinner
  -p COMMAND, --process=COMMAND
                        Command to execute
  -l, --list_symbols    List of symbols 
  -c COLOR, --color=COLOR
                        Change text color
  --list_colors         Show list colors 
```

**It also has an integrated manual.**
`man spin`

> [!NOTE]
> **Predefined Symbol**
> > circleHalves: ◐◓◑◒

## Example 

**Show a spinner with custom text:**

```bash
spin -t "Loading..." -p "sleep 3"
```

```bash 
echo "Hola mundo" | spin -t 
```

### Usage **for**
```sh
_pkgs=("pkg1" "pkg2" "pkg3")
spin -p '$(for package in "${_pkgs[@]}"; do
    echo "pkg install -y $package;"
done)'
```

## Programming

### Use as **python** module:

- Requirement 

```sh 
pip install pybind11
```

- Compilation

```
make build-python
```
#### Example

```py
import spinners
import time

# Create a Spinner object
s = spinners.Spinner()

# Set.textnand)symbols 
s.set_color("blue")
s.set_text("Cargando...")
s.set_symbols("dots")

#Start and stop the spinner
s.start()
time.sleep(5)
s.stop()
```
### Use in **C++**   

```cpp
#include "spinners.hpp"

int main (int argc, char *argv[]) {
    spinners::Spinner spinner;
    spinner.setColor("red");
    spinner.setText("Loading..");
    spinner.setSymbols("dots");

    spinner.start();
    sleep(5);
    spinner.stop();
    return 0;
}
```

> [!NOTE]
> Project ideas obtained from [ora](https://github.com/sindresorhus/ora)

## Contribute

If you would like to contribute to this project, please fork the repository and submit a pull request.

## Reference 

- [ANSI Escape Sequences](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797)
- [optparce](https://github.com/myint/optparse)
