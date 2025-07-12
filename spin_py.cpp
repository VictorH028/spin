#include "include/spinners.hpp" // Incluye la librería de C++
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(spinner, m)
{
    m.doc() = "Spinners module to create terminal spinners with color support";

    // Exporta la función `getSpinner`
    /*m.def("get_spinner", &spinners::getSpinner, "Get spinner symbols by name",*/
    /*py::arg("key"));*/

    // Define la clase Spinner
    py::class_<Spinner>(m, "Spinner")
        .def(py::init<>(), "Default constructor") // Constructor sin parámetros
        .def("set_color", &Spinner::setColor, "Set the spinner color",
            py::arg("color_name"))
        .def("set_interval", &Spinner::setInterval,
            "Set the spinner interval in milliseconds", py::arg("interval"))
        .def("set_text", &Spinner::setText,
            "Set the text displayed with the spinner", py::arg("text"))
        .def("set_symbols", &Spinner::setSymbols,
            "Set the spinner symbols", py::arg("symbols"))
        // En la definición de la clase Spinner, añade:
        /*.def("get_spinner", &spinners::Spinner::getSpinner, "Get spinner symbols by name",*/
        /*    py::arg("key"))*/
        .def("start", &Spinner::start, "Start the spinner animation")
        .def("stop", &Spinner::stop, "Stop the spinner animation");
}
