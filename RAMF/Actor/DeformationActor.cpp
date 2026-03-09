#include "DeformationActor.h"
#include <string.h>
#include "Fop.h"
#include "ActiveMatterInterface.h"

using namespace actor;

void DeformationActor::initialize(const std::string &cfgContent, const Config &config)
{
    std::string str = config.Read<std::string>("shape factor");
    optionalLda = Fop::loadvec1d<double>(str);
}

void DeformationActor::takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive)
{
    auto itf = std::dynamic_pointer_cast<ChangeShapeAble>(am);
    // std::vector<double> newvswim(am->amatternum, 0.0);
    if (inaive)
    {
        for (unsigned i = 0; i < am->amatternum; i++)
        {
            itf->setShape(1.0, i);
        }
    }
    else
    {
        for (unsigned i = 0; i < am->amatternum; i++)
        {
            int act = (int)round(action[i][0]);
            double Lda = optionalLda[act];
            double ar;
            ar = Lda == 1 ? 1e8 : sqrt((1 + Lda) / (1 - Lda));
            itf->setShape(ar, i);
        }
    }
}

void DeformationContActor::initialize(const std::string &cfgContent, const Config &config)
{
    SetActionRescalingFactor(config);
    std::string str = config.Read<std::string>("shape factor range", "");
    if (str == "")
    {
        LdaMin = -1.0;
        LdaMax = 1.0;
    }
    else
    {
        std::vector<double> range = Fop::loadvec1d<double>(str);
        if (range.size() != 2)
        {
            throw(std::runtime_error("DeformationContActor:: Shape factor range must have a size of 2!"));
        }
        LdaMin = range[0];
        LdaMax = range[1];
    }
}

void DeformationContActor::takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive)
{
    auto itf = std::dynamic_pointer_cast<ChangeShapeAble>(am);
    // std::vector<double> newvswim(am->amatternum, 0.0);
    if (inaive)
    {
        for (unsigned i = 0; i < am->amatternum; i++)
        {
            itf->setShape(1.0, i);
        }
    }
    else
    {
        for (unsigned i = 0; i < am->amatternum; i++)
        {

            double Lda = action[i][0] * actionScale[0] + actionOffset[0];
            Lda = std::clamp(Lda, LdaMin, LdaMax);
            double ar;
            if (abs(Lda - 1.0) < __DBL_EPSILON__)
            {
                ar = Lda > 0 ? 1e8 : -1e8; // avoid numerical instability
            }
            else
            {
                ar = sqrt((1 + Lda) / (1 - Lda));
            }
            itf->setShape(ar, i);
        }
    }
}