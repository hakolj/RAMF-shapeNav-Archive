#ifndef DEFORMATIONACTOR_H
#define DEFORMATIONACTOR_H

#include "Actor.h"
namespace actor
{
    // descritized deformation for inertialess swimmer
    class DeformationActor : public Actor
    {

    private:
        std::vector<double> optionalLda;

    public:
        DeformationActor() : optionalLda() {};
        void takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive);
        void initialize(const std::string &cfgContent, const Config &config);
        inline virtual int dim() const { return 1; }
        inline virtual std::vector<int> num() const { return std::vector<int>({optionalLda.size()}); }
    };

    // continuous deformation for inertialess swimmer
    class DeformationContActor : public Actor
    {

    private:
        double LdaMax;
        double LdaMin;

    public:
        DeformationContActor() {};
        void takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive);
        void initialize(const std::string &cfgContent, const Config &config);
        inline virtual int dim() const { return 1; }
        inline virtual std::vector<int> num() const { return {0}; }
    };
}

#endif