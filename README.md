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
  -h, --help            show this help message and exit
  --version             show program's version number and exit
  -t TEXT, --text=TEXT  Text to show
  -i INT, --interval=INT
                        Interval in milliseconds
  --style=NAME          Built-in spinner style name
  --frames=FRAMES       Custom frames: "⠋⠙⠚" or "one,two,three"
  --position=POSITION   Spinner position: before or after
  --text-style=STYLE    Text style: normal,bold,dim,italic,underline,reverse
  --cmd=COMMAND         Command to execute
  -c COLOR, --color=COLOR
                        ANSI 256 color (0-255)
  -q, --quiet           Suppress command output

  Execution:
    Parallel command execution

    -j N, --jobs=N      Max parallel jobs (0 = auto-detect from CPU/process limit)
    --wake-threshold=N  Commands in queue that take the Android wake lock (<1 disables)
    --no-wake-lock      Never take the Android wake lock
    --pin-workers       Pin each worker to its own CPU core
    --log=PATH          Log file path (default: $HOME/.spin.log)

  Information:
    Show available options

    --show-colors       Show list of 256 colors
    --show-style        Show built-in spinner styles
    --show-text-styles  Show available text styles
    --show-positions    Show spinner positions
    --show-system       Show detected CPU topology and execution limits

  Result:
    Display status after completion

    --result            Display message after execution finishes
    -e TEXT, --error=TEXT
                        Custom error message
    -s TEXT, --success=TEXT
                        Custom success message
```

**It also has an integrated manual.**
`man spin`

> [!NOTE]
> **Predefined Symbol**
> > circleHalves: ◐◓◑◒

## Example 

**Show a spinner with custom text:**

```bash
spin -t "Loading..." --cmd "sleep 3"
```

```bash 
echo "Hola mundo" | spin -t 
```
The option `--cmd` has parallel execution

```bash
spin -c 230 --cmd "sleep 10, sleep 15, sleep 20"
```

```shell
⊨ ps af                                                         ~
  PID TTY      STAT   TIME COMMAND
20312 pts/0    S<sl   0:00 /data/data/com.termux/files/usr/bin/fis
20410 pts/0    T<l    0:00  \_ spin -cmd sleep 10, sleep 20
20414 pts/0    T<     0:00  |   \_ sleep 10
20415 pts/0    T<     0:00  |   \_ sleep 20
```

### Usage **for**
```sh
_pkgs=("pkg1" "pkg2" "pkg3")
spin -cmd '$(for package in "${_pkgs[@]}"; do
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
s.set_color("105")
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
    spinner.setColor(105);
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
