#ifndef SHAPENAVIGATIONAGENTS_H
#define SHAPENAVIGATIONAGENTS_H

#include "Agent.h"
namespace agent::shapeNav
{

    // theoretical strategy for shape navigation proposed by Kristian
    // Lambda=sign(\ve p\cdot\ve e_r)*sign(S_{np})
    // requirement: 2D, signals: n_r, n_t, s_np
    class TheoreticalStrategy : public Agent
    {
    public:
        TheoreticalStrategy() {};
        virtual void decideAction(const std::vector<std::vector<double>> &state,
                                  std::vector<std::vector<double>> &action,
                                  bool iexplore);
        virtual void train(MemoryBase *memory) {};
        virtual void initialize(const std::string &cfgContent, const Config &config) {};
        virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
        virtual void model_saver(const std::string &path, const std::string &suffix) {};
        virtual void model_loader(const std::string &path, const std::string &suffix) {};
        virtual void recParam(const char *path) {}
        virtual void dumpTrainInfo(int episode, const std::string &path) {};
        virtual void prepareTrainInfo(const std::string &path) {};
    };

    class PolynomialStrategy : public Agent
    {
    public:
        PolynomialStrategy() {};
        virtual void decideAction(const std::vector<std::vector<double>> &state,
                                  std::vector<std::vector<double>> &action,
                                  bool iexplore);
        virtual void train(MemoryBase *memory) {};
        virtual void initialize(const std::string &cfgContent, const Config &config);
        virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
        virtual void model_saver(const std::string &path, const std::string &suffix) {};
        virtual void model_loader(const std::string &path, const std::string &suffix) {};
        virtual void recParam(const char *path) {}
        virtual void dumpTrainInfo(int episode, const std::string &path) {};
        virtual void prepareTrainInfo(const std::string &path) {};

    protected:
        std::vector<double> c; // coefficients of the polynomial
    };

    class OrientationStableStrategy : public Agent
    {
    public:
        OrientationStableStrategy() {};
        virtual void decideAction(const std::vector<std::vector<double>> &state,
                                  std::vector<std::vector<double>> &action,
                                  bool iexplore);
        virtual void train(MemoryBase *memory) {};
        virtual void initialize(const std::string &cfgContent, const Config &config) {};
        virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
        virtual void model_saver(const std::string &path, const std::string &suffix) {};
        virtual void model_loader(const std::string &path, const std::string &suffix) {};
        virtual void recParam(const char *path) {}
        virtual void dumpTrainInfo(int episode, const std::string &path) {};
        virtual void prepareTrainInfo(const std::string &path) {};
    };

    class TanhFittedStrategy : public Agent
    {
    public:
        TanhFittedStrategy() {};
        virtual void decideAction(const std::vector<std::vector<double>> &state,
                                  std::vector<std::vector<double>> &action,
                                  bool iexplore);
        virtual void train(MemoryBase *memory) {};
        virtual void initialize(const std::string &cfgContent, const Config &config);
        virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
        virtual void model_saver(const std::string &path, const std::string &suffix) {};
        virtual void model_loader(const std::string &path, const std::string &suffix) {};
        virtual void recParam(const char *path) {}
        virtual void dumpTrainInfo(int episode, const std::string &path) {};
        virtual void prepareTrainInfo(const std::string &path) {};

    private:
        std::vector<double> sigMean; // use to normalize the signal
        std::vector<double> sigStd;
        std::vector<double> coeffs; // coefficients for the tanh function
        int mode;                   // 0: special fitting only for Ku=Inf; 1: general fitting (sum of coeffs*sig_i)
    };

}
#endif // SHAPENAVIGATIONAGENTS_H