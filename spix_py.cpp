#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "include/spinners.hpp"  //Incluye la de C++

namespace py = pybind11;

PYBIND11_MODULE(spinners, m) {
    m.def("get_spinner", &spinners::getSpinner, "Get spinner by name");

    py::class_<spinners::Spinner>(m, "Spinner")
        .def(py::init<>())  // Constructor por defecto
        .def(py::init<int, std::string, std::string>(), "Constructor con parámetros")
        .def("set_interval", &spinners::Spinner::setInterval)
        .def("set_text", &spinners::Spinner::setText)
        .def("set_symbols", &spinners::Spinner::setSymbols)
        .def("start", &spinners::Spinner::start)
        .def("stop", &spinners::Spinner::stop);
}

