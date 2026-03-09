#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
//#include <pybind11/stl_bind.h>
#include <pybind11/eigen.h>
#include <Eigen/Dense>
#include "Environment.h"
#include <stdexcept>

#include "Exceptions.h"


namespace py = pybind11;


// wrap Environment with some pybind11 settings
class PyEnvironment : public Environment {
public:
    // create simulation env.
    PyEnvironment(const std::string& configpath) :Environment(configpath) {};

    // prepare enviroment data before start of simulation
    void Prepare(bool isTrain, bool isLoad) {
        py::scoped_ostream_redirect stream;
        Environment::Prepare(isTrain, isLoad);
    }
    // reset at the begining of every episode 
    void Reset(bool isDump) {
        py::scoped_ostream_redirect stream;
        Environment::Reset(isDump);
    };
    // dumping simulation result
    void Dump(const char* path, int step) {
        py::scoped_ostream_redirect stream;
        Environment::Dump(path, step);
    }
    // taking action
    void Act(py::EigenDRef<const Eigen::MatrixXd> action, bool inaive) {
        py::scoped_ostream_redirect stream;
        for (int i = 0; i < amatternum; i++) {
            for (int j = 0; j < actor->dim(); j++) _action[i][j] = action(i, j);
        }
        Environment::Act(_action, inaive);
    }
    // update env
    void Update() {
        py::scoped_ostream_redirect stream;
        Environment::Update();
    }
    // receive state from env
    void ObserveState(py::EigenDRef<Eigen::MatrixXd> state) {
        py::scoped_ostream_redirect stream;
        if (state.rows() != amatternum) {
            throwException<std::runtime_error>("In Environment::ObserveState: The rows of state is not identical to amatternum!");
        }
        if (state.cols() != sensor->dim()) {
            throwException<std::runtime_error>("In Environment::ObserveState: The cols of state is not identical to state dimension!");
        }
        Environment::ObserveState(_state);
        for (int i = 0; i < amatternum; i++) {
            for (int j = 0; j < sensor->dim(); j++) state(i, j) = _state[i][j];
        }
    }
    // receive reward from env
    void ObserveReward(py::EigenDRef<Eigen::VectorXd> reward) {
        py::scoped_ostream_redirect stream;
        if (_reward.size() != reward.size()) {
            throwException<std::runtime_error>("In Environment::ObserveReward: The size of vector is not identical to amatternum!");
        }
        Environment::ObserveReward(_reward);
        for (int i = 0; i < reward.size(); i++) reward(i) = _reward[i];
    }

    // receive transition from env
    void ObserveTransition(py::EigenDRef<Eigen::MatrixXd> state,
        py::EigenDRef<Eigen::VectorXd> reward,
        py::EigenDRef<Eigen::VectorXi> terminateState,
        py::EigenDRef<Eigen::VectorXi> validMask) {
        py::scoped_ostream_redirect stream;
        if (state.rows() != amatternum)
            throwException<std::runtime_error>("In Environment::ObserveState: The rows of state is not identical to amatternum!");
        if (state.cols() != sensor->dim()) 
            throwException<std::runtime_error>("In Environment::ObserveState: The cols of state is not identical to state dimension!");
        if (reward.rows() != amatternum)
            throwException<std::runtime_error>("In Environment::ObserveState: The rows of reward is not identical to amatternum!");
        if (terminateState.rows() != amatternum)
            throwException<std::runtime_error>("In Environment::ObserveState: The rows of terminateState is not identical to amatternum!");
        if (validMask.rows() != amatternum)
            throwException<std::runtime_error>("In Environment::ObserveState: The rows of validMask is not identical to amatternum!");

        // always use a vector to receive every element of the matrix
        // because the memory of matraix is no guaranteed to be memory contiguous

        Environment::ObserveTransition(_state, _reward, _terminateState, _validMask);
        // std::vector -> Eigen::Matrix
        for (int i = 0; i < amatternum; i++) {
            for (int j = 0; j < sensor->dim(); j++) state(i, j) = _state[i][j];
            reward(i) = _reward[i];
            terminateState(i) = _terminateState[i];
            validMask(i) = _validMask[i];
        }
    }

    //summarize an episode, receive mean (total) reward
    double EpisodeSummarize(bool isTrain) {
        py::scoped_ostream_redirect stream;
        double meanRw = 0;
        Environment::EpisodeSummarize(meanRw, isTrain);
        return meanRw;
    }

    int DimAction() {
        return this->actor->dim();
    }
    int DimState() {
        return this->sensor->dim();
    }

};

//using namespace pybind11::literals;
PYBIND11_MODULE(amfpy, m) {
    m.doc() = "amfpy is a library for simulate smart active matter in fluid.";

    // binding PyEnvironment to python interface
    py::class_<PyEnvironment,std::shared_ptr<PyEnvironment>>(m, "Environment")
        // wrap a lambda function as the init function in python
        .def(py::init([](const std::string& path) {
        // redirect srd::out stream to python stdout stream. 
        py::scoped_ostream_redirect stream(
            std::cout,                               // std::ostream&
            py::module_::import("sys").attr("stdout") // Python output
        );
        // initial constructor
        return new PyEnvironment(path);
            }
        ))
        .def("Prepare", &PyEnvironment::Prepare,py::arg("isTrain"),py::arg("isLoad"))
        .def("Reset", &PyEnvironment::Reset, py::arg("isDump"))
        .def("Dump", &PyEnvironment::Dump, py::arg("dir"), py::arg("stepidx"))
        .def("Act", &PyEnvironment::Act, py::arg("action").noconvert(),py::arg("inaive"))
        .def("Update", &PyEnvironment::Update)
        .def("ObserveReward", &PyEnvironment::ObserveReward, py::arg("reward").noconvert())
        .def("ObserveState", &PyEnvironment::ObserveState, py::arg("state").noconvert())
        .def("ObserveTransition", &PyEnvironment::ObserveTransition,
            py::arg("state").noconvert(), py::arg("reward").noconvert(),
            py::arg("terminateState").noconvert(), py::arg("validMask").noconvert())
        .def("EpisodeSummarize", &PyEnvironment::EpisodeSummarize, 
           py::arg("isTrain"))
        .def("dim_action", &PyEnvironment::DimAction)
        .def("dim_state", &PyEnvironment::DimState);

};

